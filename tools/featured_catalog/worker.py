from pathlib import Path
import argparse
import copy
import fcntl
import json
import re
import time
from common import save
from catalog_worker import cycle
from database import Database
from events import Delivery, Watcher, signal
from github import Publisher
from native_wire import decode_wire
from release import prepare


class Worker:
    def __init__(self, config, query=None):
        self.config = copy.deepcopy(config)
        if config["schema"] != 1 or config["mode"] not in ("stage", "github_pr", "github_direct"):
            raise ValueError("Unsupported worker configuration")
        if config["mode"] != "stage" and config["github"]["repository"] != config["release"]["repository"]:
            raise ValueError("Publication repositories differ")
        if config["mode"] != "stage":
            publication = config["github"]
            if not re.fullmatch(r"channels/[A-Za-z0-9_-]+\.json", publication["channel_path"]):
                raise ValueError("Invalid channel path")
            if not re.fullmatch(r"[A-Za-z0-9_-]+", publication.get("base_branch", "main")):
                raise ValueError("Invalid feed branch")
        self.root = Path(config["state_directory"])
        self.root.mkdir(parents=True, exist_ok=True)
        self.query = query or Database(config["database"])
        self.renderer = self.config["renderer"]
        self.renderer["outbox"] = str(self.root / "outbox")
        self.renderer["mpq_tools"] = str(Path(__file__).parent)
        for key in ("release", "publication"):
            self.renderer.pop(key, None)
        for key in ("archive", "rewards", "summary_catalogs"):
            if key not in self.renderer:
                raise ValueError("Missing renderer input: " + key)

    def tick(self):
        state_path = self.root / "publication.json"
        state = json.loads(state_path.read_text()) if state_path.exists() else {}
        if state.get("pending"):
            identity = state["pending"]
        else:
            identity = cycle(self.renderer, self.query, decode_wire)
        if state.get("completed") == identity:
            return "unchanged"
        job = self.root / "outbox" / identity
        release_config = copy.deepcopy(self.config["release"])
        adopted = self.root / "adopted.json"
        if adopted.exists():
            release_config.update(json.loads(adopted.read_text()))
        receipt_path = job / "release" / "receipt.json"
        receipt = json.loads(receipt_path.read_text()) if receipt_path.exists() else prepare(job, release_config)
        if receipt is None:
            return "unchanged"
        if self.config["mode"] == "stage":
            save(self.root / "staged.json", {"job": identity, "tag": receipt["tag"]})
            return "staged"
        state["pending"] = identity
        save(state_path, state)
        publication = dict(self.config["github"], mode=self.config["mode"])
        result = Publisher(publication, self.root / (identity + ".github.json")).run(job, receipt)
        if result != "published":
            raise RuntimeError("Publication not verified")
        from common import sha
        pointer = job / "release" / "published-pointer.json"
        save(pointer, receipt["pointer"])
        manifest = job / "release" / "manifest.json"
        save(adopted, {"manifest": {"path": str(manifest), "sha256": sha(manifest.read_bytes())},
                       "pointer": {"path": str(pointer), "sha256": sha(pointer.read_bytes())},
                       "archive_sha256": sha((job / "patch-E.MPQ").read_bytes()),
                       "base_archive_sha256": sha((job / "patch-M.MPQ").read_bytes())})
        save(state_path, {"completed": identity})
        return self.tick()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--config", required=True)
    parser.add_argument("--once", action="store_true")
    args = parser.parse_args()
    config = json.loads(Path(args.config).read_text())
    worker = Worker(config)
    with (worker.root / "worker.lock").open("a") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        if args.once:
            print(worker.tick(), flush=True)
            return
        delivery = Delivery(worker.root / "events.json", lambda: signal(config["signal_path"]), worker.tick)
        while True:
            watcher = None
            try:
                watcher = Watcher(config["signal_path"])
                delivery.notify()
                while True:
                    delivery.run_due()
                    if watcher.wait(delivery.delay()):
                        delivery.notify()
            except (OSError, RuntimeError):
                time.sleep(30)
            finally:
                if watcher:
                    watcher.close()


if __name__ == "__main__":
    main()
