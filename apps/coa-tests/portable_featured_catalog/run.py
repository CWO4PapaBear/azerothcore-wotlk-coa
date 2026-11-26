from pathlib import Path
import importlib.util
import json
import os
import subprocess
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools/featured_catalog"))
from common import save, sha
from events import Delivery, Watcher, signal
from github import Publisher
from database import Database
from catalog_worker import QUERY, PRICING_QUERY, records, dbc
from worker import Worker
from native_wire import decode_wire
from lib.mpq import MPQArchive


class Tests(unittest.TestCase):
    def test_events_and_atomic_save(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "signal"
            watch = Watcher(path)
            try:
                temporary = Path(directory) / "new"
                temporary.write_text("123-456\n")
                temporary.replace(path)
                self.assertTrue(watch.wait(1))
                self.assertEqual(signal(path), "123-456")
            finally:
                watch.close()

    def test_retry_and_save_during_delivery(self):
        with tempfile.TemporaryDirectory() as directory:
            now = [0]
            current = ["1-1"]
            calls = []
            def deliver():
                calls.append(1)
                if len(calls) == 1:
                    raise RuntimeError("unavailable")
                current[0] = "2-2"
                return "published"
            delivery = Delivery(Path(directory) / "state", lambda: current[0], deliver, lambda: now[0])
            now[0] = 5
            delivery.run_due()
            self.assertTrue(delivery.state["pending"])
            now[0] = 35
            delivery.run_due()
            self.assertTrue(delivery.state["pending"])
            now[0] = 40
            delivery.run_due()
            self.assertFalse(delivery.state["pending"])

    def test_uncertain_write_reconciles_without_duplicate(self):
        with tempfile.TemporaryDirectory() as directory:
            publisher = Publisher({}, Path(directory) / "state", api=object())
            found = []
            def write():
                found.append(True)
                raise RuntimeError("lost response")
            self.assertTrue(publisher.once("asset", lambda: bool(found), write))
            self.assertTrue(publisher.once("asset", lambda: bool(found), write))
            self.assertEqual(len(found), 1)

    def test_unresolved_write_is_not_replayed(self):
        with tempfile.TemporaryDirectory() as directory, patch("github.time.sleep"):
            path = Path(directory) / "state"
            publisher = Publisher({}, path, api=object())
            writes = []
            for instance in (publisher, Publisher({}, path, api=object())):
                if path.exists():
                    instance.state = json.loads(path.read_text())
                with self.assertRaises(RuntimeError):
                    instance.once("asset", lambda: False, lambda: writes.append(1))
            self.assertEqual(writes, [1])

    def test_database_scope_and_private_credentials(self):
        with tempfile.TemporaryDirectory() as directory:
            defaults = Path(directory) / "mysql.cnf"
            defaults.write_text("[client]\nuser=catalog\n")
            defaults.chmod(0o600)
            database = Database({"defaults_file": str(defaults), "characters": "chars", "world": "world"})
            with self.assertRaises(ValueError):
                database("DELETE FROM area52_build")
            result = subprocess.CompletedProcess([], 0, "rows", "")
            with patch("database.subprocess.run", return_value=result) as run:
                self.assertEqual(database(PRICING_QUERY), "rows")
                self.assertIn("world.coa_client", run.call_args.kwargs["input"])
            defaults.chmod(0o644)
            with self.assertRaises(ValueError):
                Database(database.config)

    def test_live_feed_drift_blocks_before_write(self):
        class API:
            def file(self, path, ref):
                return b'{"different":true}'
        with tempfile.TemporaryDirectory() as directory:
            publisher = Publisher({"channel_path": "channels/example.json"}, Path(directory) / "state", api=API())
            with self.assertRaisesRegex(RuntimeError, "Live feed changed"):
                publisher.run(Path(directory), {"pointer": {}, "baseline_pointer": {"old": True}})
            self.assertFalse(publisher.path.exists())

    def test_complete_pr_and_direct_delivery(self):
        class API:
            def __init__(self):
                self.pointer = {"baseline": True}
                self.candidate = {"candidate": True}
                self.releases = []
                self.assets = []
                self.refs = []
                self.prs = []
                self.writes = []
                self.hashes = {}

            def file(self, path, ref):
                return json.dumps(self.candidate if ref.startswith("catalog/") else self.pointer).encode()

            def upload(self, tag, path):
                self.writes.append("upload")
                self.assets.append({"name": path.name})
                self.hashes[path.name] = sha(path.read_bytes())
                raise RuntimeError("lost successful upload response")

            def asset_hash(self, tag, name):
                return self.hashes[name]

            def api(self, path, payload=None, method="POST"):
                if payload is not None:
                    self.writes.append(path)
                if path.startswith("releases?"):
                    return self.releases
                if path == "releases":
                    self.releases.append(dict(payload, id=1))
                    raise RuntimeError("lost release response")
                if path.startswith("releases/1/assets"):
                    return self.assets
                if path.startswith("git/ref/heads/"):
                    return {"object": {"sha": "parent"}}
                if path == "git/commits/parent":
                    return {"tree": {"sha": "tree"}}
                if path in ("git/blobs", "git/trees", "git/commits"):
                    return {"sha": "candidate"}
                if path.startswith("git/matching-refs/"):
                    return self.refs
                if path == "git/refs":
                    self.refs.append({"ref": payload["ref"], "object": {"sha": payload["sha"]}})
                    return self.refs[-1]
                if path.startswith("git/refs/heads/"):
                    assert method == "PATCH" and payload["force"] is False
                    self.pointer = self.candidate
                    return {}
                if path.startswith("pulls?"):
                    return self.prs
                if path == "pulls":
                    self.prs.append({"html_url": "https://example.invalid/pr/1", "state": "open"})
                    raise RuntimeError("lost successful PR response")
                raise AssertionError(path)

        for mode in ("github_pr", "github_direct"):
            with tempfile.TemporaryDirectory() as directory, patch("github.time.sleep"):
                job = Path(directory)
                (job / "release").mkdir()
                asset = job / "release/manifest.json"
                asset.write_bytes(b"manifest")
                receipt = {"tag": "example-catalog-1", "pointer": {"candidate": True},
                           "baseline_pointer": {"baseline": True}, "assets": {asset.name: sha(asset.read_bytes())}}
                api = API()
                config = {"mode": mode, "repository": "example/client", "channel_path": "channels/example.json"}
                publisher = Publisher(config, job / "state", api)
                if mode == "github_pr":
                    from github import PendingReview
                    for _ in range(2):
                        with self.assertRaises(PendingReview):
                            publisher.run(job, receipt)
                    self.assertEqual(api.writes.count("pulls"), 1)
                    self.assertEqual(api.pointer, receipt["baseline_pointer"])
                    api.prs[0]["state"] = "closed"
                    with self.assertRaisesRegex(RuntimeError, "closed without merge"):
                        publisher.run(job, receipt)
                    api.pointer = receipt["pointer"]
                self.assertEqual(publisher.run(job, receipt), "published")
                self.assertEqual(api.writes.count("upload"), 1)
                api.hashes[asset.name] = "wrong"
                with self.assertRaisesRegex(ValueError, "asset mismatch"):
                    publisher.run(job, receipt)

    def test_new_draft_reward_queries_and_native_parser(self):
        if not os.environ.get("FEATURED_CATALOG_FIXTURE"):
            self.skipTest("Licensed client fixture not supplied")
        import runpy
        runpy.run_path(str(Path(__file__).with_name("new_starts.py")), run_name="__main__")

    def test_container_without_network_or_windows_runtime(self):
        image = os.environ.get("FEATURED_CATALOG_IMAGE")
        fixture = os.environ.get("FEATURED_CATALOG_FIXTURE")
        if not image or not fixture:
            self.skipTest("Container qualification inputs not supplied")
        code = """import json,sys
from pathlib import Path
from worker import Worker
from catalog_worker import QUERY,PRICING_QUERY
root=Path(sys.argv[1]);original=json.loads((root/'runtime/config.json').read_text())
config={'schema':1,'mode':'stage','state_directory':'/tmp/catalog-test','renderer':original,'release':original['release']}
snapshots={QUERY:(root/'builds.tsv').read_text(),PRICING_QUERY:(root/'prices.tsv').read_text()}
assert Worker(config,snapshots.__getitem__).tick()=='staged'
print('Portable container rendered both client catalogs without network or Windows runtime')
"""
        subprocess.run(["docker", "run", "--rm", "--network", "none", "--read-only", "--tmpfs", "/tmp",
                        "--volume", fixture + ":" + fixture + ":ro", "--entrypoint", "python3", image,
                        "-c", code, fixture], check=True, timeout=120)

    def test_notification_permissions_as_worker_uid(self):
        if os.geteuid() != 0:
            self.skipTest("UID transition requires root in offline fixture")
        from notification_access import check, grant
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            root.chmod(0o700)
            (root / "area52-featured-catalog.signal").write_text("123-456\n")
            with self.assertRaises(RuntimeError):
                check(root)
            self.assertIn("uid=1000", grant(root, root / "receipt.json"))
            self.assertEqual((root.stat().st_mode & 0o007), 0)
            self.assertEqual((root / "area52-featured-catalog.signal").stat().st_uid, 0)

    def test_preview_weapon_visibility(self):
        import copy
        from catalog_worker import render, cycle
        from catalog_rewards import render as rewards
        from native_wire import encode_wire
        fixture = Path(os.environ["FEATURED_CATALOG_FIXTURE"])
        config = json.loads((fixture / "runtime/config.json").read_text())
        raw = (fixture / "builds.tsv").read_text()
        original = records(raw, decode_wire)
        load = lambda row: Path(row["path"]).read_bytes()
        base, items = load(config["details"]), load(config["items"])
        inc = json.loads(load(config["incarnations"]))["records"]
        rw = config["rewards"]
        reward_args = [load(rw[k]) for k in ("sets", "vanity", "appearances")]
        baseline_rewards = rewards(*reward_args, original, json.loads(load(rw["mounts"]))["kinds"], json.loads(load(rw["defaults"]))["bundles"]["Bundles"])
        baseline, _ = render(base, original, inc, items)
        for mode in range(4):
            assigned = copy.deepcopy(original)
            assigned[86]["weapons"] = [mode]
            result, _ = render(base, assigned, inc, items)
            selected = assigned[86]["items"]
            expected = selected[:12]
            if mode in (2,3): expected[9:11] = [0,0]
            if mode in (1,3): expected[11] = 0
            rows = dbc(result, 28)
            self.assertEqual(sum(r[1] == 86 for r in rows), 20)
            for before, after in zip(dbc(baseline,28),rows):
                if after[1] == 86:
                    self.assertEqual(after[16:28], expected)
                    self.assertEqual(after[:16],before[:16])
                else: self.assertEqual(before,after)
            self.assertEqual(assigned[86]["items"], original[86]["items"])
            self.assertEqual(rewards(*reward_args, assigned, json.loads(load(rw["mounts"]))["kinds"], json.loads(load(rw["defaults"]))["bundles"]["Bundles"]),baseline_rewards)
        lines=[]
        for line in raw.splitlines():
            aid, rev, wire = line.split('\t')
            if int(aid)==86:
                build=decode_wire(bytes.fromhex(wire))
                old=build['Description'].encode()+b'\0'
                new=build['Description'].encode()+b'\n\n###AREA52_FEATURED_WEAPONS\n1:2\0'
                self.assertEqual(bytes.fromhex(wire).count(old),1)
                wire=bytes.fromhex(wire).replace(old,new,1).hex()
            lines.append('\t'.join((aid,rev,wire)))
        candidate='\n'.join(lines)
        self.assertEqual(records(candidate,decode_wire)[86]['weapons'],[2])
        with tempfile.TemporaryDirectory() as directory:
            cfg=copy.deepcopy(config);cfg['outbox']=directory
            identity=cycle(cfg,{QUERY:candidate,PRICING_QUERY:(fixture/'prices.tsv').read_text()}.__getitem__,decode_wire)
            job=Path(directory)/identity
            with MPQArchive(job/'patch-E.MPQ') as archive:
                rows=dbc(archive.read_file('DBFilesClient\\CharacterCreationArchetypeDetails.dbc'),28)
                self.assertTrue(all(row[25:27]==[0,0] and row[27]==original[86]['items'][11] for row in rows if row[1]==86))
                for member in ('DBFilesClient\\ItemSet.dbc','Interface\\AddOns\\Ascension_AppearanceUI\\FeaturedSetItems.lua'):
                    self.assertIn(str(original[86]['items'][9]), archive.read_file(member).decode() if member.endswith('.lua') else str(dbc(archive.read_file(member),53)))
            receipt=json.loads((job/'patch-E.catalog.json').read_text())
            with MPQArchive(job/'patch-E.MPQ') as archive:
                for member,digest in receipt['preserved'].items():self.assertEqual(sha(archive.read_file(member)),digest)
        for value in ('1:4','1:4294967295','2:2','1:2,3','1:2\n###AREA52_FEATURED_WEAPONS\n1:1'):
            build={'Category':8,'Description':'###AREA52_ARCHETYPE_ID\n86\n###AREA52_FEATURED_WEAPONS\n'+value}
            with self.assertRaises(ValueError):records('86\t1\t00',lambda _:build)

    def test_captured_saved_catalog(self):
        fixture = os.environ.get("FEATURED_CATALOG_FIXTURE")
        if not fixture:
            self.skipTest("Licensed client fixture not supplied")
        fixture = Path(fixture)
        original = json.loads((fixture / "runtime/config.json").read_text())
        with tempfile.TemporaryDirectory() as directory:
            config = {"schema": 1, "mode": "stage", "state_directory": directory,
                      "renderer": original, "release": original["release"]}
            snapshots = {QUERY: (fixture / "builds.tsv").read_text(), PRICING_QUERY: (fixture / "prices.tsv").read_text()}
            worker = Worker(config, snapshots.__getitem__)
            self.assertEqual(worker.tick(), "staged")
            self.assertEqual(worker.tick(), "staged")
            job = Path(directory) / "outbox" / json.loads((Path(directory) / "staged.json").read_text())["job"]
            assigned = records(snapshots[QUERY], decode_wire)
            with MPQArchive(job / "patch-E.MPQ") as archive:
                details = dbc(archive.read_file("DBFilesClient\\CharacterCreationArchetypeDetails.dbc"), 28)
                sets = {row[0]: row for row in dbc(archive.read_file("DBFilesClient\\ItemSet.dbc"), 53)}
                vanity = archive.read_file("DBFilesClient\\VanityCollection.dbc")
                _, count, _, size, _ = struct.unpack_from("<4s4I", vanity)
                bundles = {row[1]: row for row in (struct.unpack_from("<76I", vanity, 20 + i * size)
                                                  for i in range(count))}
                wardrobe = archive.read_file("Interface\\AddOns\\Ascension_AppearanceUI\\FeaturedSetItems.lua").decode()
                for archetype, revision in ((27, 8), (86, 7)):
                    items = assigned[archetype]["items"]
                    self.assertEqual(assigned[archetype]["revision"], revision)
                    self.assertTrue(set(i for i in items if i) <= set(bundles[9901000 + archetype][18:42]))
                    variants = [row for row in details if row[1] == archetype]
                    self.assertEqual(len(variants), 20)
                    self.assertTrue(all(row[16:28] == items[:12] for row in variants))
                    self.assertEqual([i for i in sets[90000 + archetype][18:35] if i], [i for i in items[:12] if i])
                    for slot, item in zip((1,3,5,10,8,9,6,15,7,16,17,18), items[:12]):
                        if item:
                            self.assertIn("[" + str(slot) + "]=" + str(item), wardrobe)
            for name in ("patch-E", "patch-M"):
                receipt = json.loads((job / (name + ".catalog.json")).read_text())
                with MPQArchive(job / (name + ".MPQ")) as archive:
                    for member, digest in receipt["preserved"].items():
                        self.assertEqual(sha(archive.read_file(member)), digest)
            launcher = original["release"]["launcher"]
            sys.path[:0] = [launcher, str(Path(launcher) / "tools")]
            from launcher.fullclient import validate
            validate(json.loads((job / "release/manifest.json").read_text()), original["release"]["repository"])


if __name__ == "__main__":
    unittest.main()
