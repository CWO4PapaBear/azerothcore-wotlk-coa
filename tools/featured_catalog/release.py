from pathlib import Path
import copy
import json
import re
from common import sha, save
from catalog_worker import pinned


def prepare(job, config):
    baseline = pinned(config["manifest"]["path"], config["manifest"]["sha256"])
    pointer_data = pinned(config["pointer"]["path"], config["pointer"]["sha256"])
    pointer = json.loads(pointer_data)
    manifest = json.loads(baseline)
    if pointer["manifest_sha256"] != sha(baseline) or manifest["schema"] != 4:
        raise ValueError("Release baseline mismatch")
    repository = config["repository"]
    if not re.fullmatch(r"[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+", repository):
        raise ValueError("Invalid release repository")
    channel = pointer["channel"]
    if not re.fullmatch(r"[a-z0-9-]+", channel):
        raise ValueError("Invalid channel")
    identity = sha((job.name + sha(baseline)).encode())[:32]
    tag = channel + "-catalog-" + identity
    prefix = "https://github.com/" + repository + "/releases/download/" + tag + "/"
    output = job / "release"
    output.mkdir(exist_ok=True)
    changed = []
    for filename, target, expected in (
        ("patch-E.MPQ", "Data/area-52/patch-E.MPQ", config["archive_sha256"]),
        ("patch-M.MPQ", "Data/patch-M.MPQ", config["base_archive_sha256"]),
    ):
        data = (job / filename).read_bytes()
        rows = [r for r in manifest["baseline"] if r["path"].lower() == target.lower()]
        components = [c for c in manifest["components"] if len(c["files"]) == 1
                      and c["files"][0]["path"].lower() == target.lower()]
        if len(rows) != 1 or len(components) != 1 or rows[0]["sha256"] != expected:
            raise ValueError("Archive release baseline mismatch")
        if sha(data) == expected:
            continue
        row = {"path": rows[0]["path"], "bytes": len(data), "sha256": sha(data)}
        component = "featured-catalog-" + sha(data)[:32]
        chunks = []
        for index, start in enumerate(range(0, len(data), 64 * 1024 * 1024)):
            block = data[start:start + 64 * 1024 * 1024]
            name = component + "-" + str(index).zfill(3) + ".bin"
            path = output / name
            if path.exists() and path.read_bytes() != block:
                raise ValueError("Immutable chunk drift")
            path.write_bytes(block)
            chunks.append({"asset": name, "url": prefix + name, "bytes": len(block), "sha256": sha(block)})
        manifest["components"].remove(components[0])
        manifest["components"].append({"id": component, "bytes": len(data), "files": [row], "chunks": chunks})
        manifest["baseline"] = [row if r["path"].lower() == target.lower() else r for r in manifest["baseline"]]
        changed.append(target)
    if not changed:
        return None
    manifest.update(version="catalog-" + identity, tag=tag, notes_url=prefix + "PATCH-NOTES.md")
    encoded = (json.dumps(manifest, indent=2) + "\n").encode()
    pointer.update(manifest_url=prefix + "manifest.json", manifest_sha256=sha(encoded),
                   message="Featured Build catalog updated.")
    notes = b"# Featured Build catalog\n\nUpdated saved Featured Build appearances and summaries.\n"
    assets = {"manifest.json": encoded, "PATCH-NOTES.md": notes}
    for name, data in assets.items():
        path = output / name
        if path.exists() and path.read_bytes() != data:
            raise ValueError("Immutable release drift")
        path.write_bytes(data)
    receipt = {"tag": tag, "pointer": pointer, "baseline_pointer": json.loads(pointer_data),
               "changed_paths": changed, "assets": {p.name: sha(p.read_bytes()) for p in output.iterdir()
                                                      if p.is_file() and p.name != "receipt.json"}}
    save(output / "receipt.json", receipt)
    return receipt
