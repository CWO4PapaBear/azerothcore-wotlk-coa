from pathlib import Path
import base64
import json
import subprocess
import tempfile
import time
from common import sha, save


class GitHub:
    def __init__(self, repository):
        self.repository = repository

    def api(self, path, payload=None, method="POST"):
        command = ["gh", "api", "repos/" + self.repository + "/" + path]
        if payload is not None:
            command += ["--method", method, "--input", "-"]
        for attempt in range(3 if payload is None else 1):
            result = subprocess.run(command, input=json.dumps(payload) if payload is not None else None,
                                    text=True, capture_output=True, timeout=120, check=False)
            if not result.returncode:
                return json.loads(result.stdout)
            if payload is None and attempt < 2:
                time.sleep(attempt + 1)
        raise RuntimeError("GitHub write uncertain" if payload is not None else "GitHub read failed")

    def file(self, path, ref):
        result = self.api("contents/" + path + "?ref=" + ref)
        if result.get("encoding") != "base64":
            raise ValueError("Unsupported repository file")
        return base64.b64decode(result["content"])

    def asset_hash(self, tag, name):
        with tempfile.TemporaryDirectory() as directory:
            result = subprocess.run(["gh", "release", "download", tag, "--repo", self.repository,
                                     "--pattern", name, "--dir", directory],
                                    capture_output=True, timeout=300, check=False)
            if result.returncode:
                raise RuntimeError("Release asset read failed")
            return sha((Path(directory) / name).read_bytes())

    def upload(self, tag, path):
        result = subprocess.run(["gh", "release", "upload", tag, str(path), "--repo", self.repository],
                                capture_output=True, timeout=600, check=False)
        if result.returncode:
            raise RuntimeError("Release upload uncertain")


class Publisher:
    def __init__(self, config, state_path, api=None):
        self.config = config
        self.path = Path(state_path)
        self.api = api or GitHub(config["repository"])
        self.state = json.loads(self.path.read_text()) if self.path.exists() else {}

    def persist(self):
        save(self.path, self.state)

    def once(self, key, reconcile, write):
        existing = reconcile()
        if existing:
            self.state[key] = "verified"
            self.persist()
            return existing
        if key in self.state:
            raise RuntimeError("Unresolved publication intent: " + key)
        self.state[key] = "intent"
        self.persist()
        try:
            write()
        except Exception:
            pass
        for attempt in range(3):
            existing = reconcile()
            if existing:
                self.state[key] = "verified"
                self.persist()
                return existing
            time.sleep(attempt + 1)
        raise RuntimeError("Unresolved publication intent: " + key)

    def run(self, job, receipt):
        api = self.api
        branch = self.config.get("base_branch", "main")
        path = self.config["channel_path"]
        actual = json.loads(api.file(path, branch))
        delivered = actual == receipt["pointer"]
        if not delivered and actual != receipt["baseline_pointer"]:
            raise RuntimeError("Live feed changed; reconcile baseline before publication")
        tag = receipt["tag"]
        def release():
            matches = [v for v in api.api("releases?per_page=100") if v["tag_name"] == tag]
            if len(matches) > 1:
                raise ValueError("Ambiguous release")
            return matches[0] if matches else None
        remote = self.once("release", release,
                           lambda: api.api("releases", {"tag_name": tag, "name": tag,
                                                        "body": "Featured catalog candidate", "prerelease": True}))
        for name, digest in receipt["assets"].items():
            local = job / "release" / name
            if sha(local.read_bytes()) != digest:
                raise ValueError("Candidate asset drift")
            def present():
                assets = api.api("releases/" + str(remote["id"]) + "/assets?per_page=100")
                if not any(a["name"] == name for a in assets):
                    return False
                if api.asset_hash(tag, name) != digest:
                    raise ValueError("Remote immutable asset mismatch")
                return True
            self.once("asset:" + name, present, lambda: api.upload(tag, local))
        if delivered:
            return "published"
        head = "catalog/" + tag
        encoded = (json.dumps(receipt["pointer"], indent=2) + "\n").encode()
        if "commit" not in self.state:
            parent = api.api("git/ref/heads/" + branch)["object"]["sha"]
            if json.loads(api.file(path, parent)) != receipt["baseline_pointer"]:
                raise RuntimeError("Feed moved before candidate commit")
            tree = api.api("git/commits/" + parent)["tree"]["sha"]
            blob = api.api("git/blobs", {"content": base64.b64encode(encoded).decode(), "encoding": "base64"})
            tree = api.api("git/trees", {"base_tree": tree, "tree": [
                {"path": path, "mode": "100644", "type": "blob", "sha": blob["sha"]}]})
            commit = api.api("git/commits", {"message": "Update Featured Build catalog " + tag,
                                            "tree": tree["sha"], "parents": [parent]})
            self.state.update(commit=commit["sha"], parent=parent)
            self.persist()
        commit = self.state["commit"]
        if self.config.get("mode") == "github_direct":
            def promoted():
                return json.loads(api.file(path, branch)) == receipt["pointer"]
            self.once("promotion", promoted, lambda: api.api("git/refs/heads/" + branch,
                      {"sha": commit, "force": False}, method="PATCH"))
            return "published"
        def find_branch():
            refs = api.api("git/matching-refs/heads/" + head)
            matching = [v for v in refs if v["ref"] == "refs/heads/" + head]
            if matching and matching[0]["object"]["sha"] != commit:
                raise ValueError("Candidate branch changed")
            return bool(matching)
        self.once("branch", find_branch, lambda: api.api("git/refs", {"ref": "refs/heads/" + head, "sha": commit}))
        if json.loads(api.file(path, head)) != receipt["pointer"]:
            raise ValueError("Candidate branch readback mismatch")
        def find_pr():
            values = api.api("pulls?state=all&per_page=100&head=" + self.config["repository"].split("/")[0]
                             + ":" + head + "&base=" + branch)
            if len(values) > 1:
                raise ValueError("Ambiguous catalog pull request")
            return values[0] if values else None
        pr = self.once("pr", find_pr, lambda: api.api("pulls", {
            "title": "Featured Build catalog update", "head": head, "base": branch,
            "body": "Generated from saved Featured revisions. Review the catalog assets before merging. "
                    "The live feed must still match the recorded baseline. Candidate: " + tag}))
        self.state["pull_request"] = pr["html_url"]
        self.persist()
        if pr["state"] == "closed" and not pr.get("merged_at"):
            raise RuntimeError("Catalog pull request closed without merge")
        raise PendingReview("Catalog awaits review or live feed readback")


class PendingReview(RuntimeError):
    pass
