"""Verify the historical SQL inputs for the Area 52 clean replay."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path

BASE = Path(__file__).resolve().parent
CORE = "ab8463e2f8ca2fdcf73d213c9e14a64353434905"


def git_bytes(obj):
    return subprocess.check_output(["git", "-C", str(BASE), "cat-file", "blob", obj])


def read_input(row, private_dir):
    source = row["source"]
    if source == "private-deployment-input":
        if private_dir is None:
            raise ValueError("Supply the private migration directory for " + row["name"])
        return (private_dir / row["name"]).read_bytes()
    if source == "recovered":
        return (BASE / row["path"]).read_bytes()
    if source == "ancestor-blob":
        data = git_bytes(row["blob"])
    elif source == "core":
        data = git_bytes(CORE + ":" + row["path"])
    else:
        raise ValueError(source)
    if row["newline"] == "raw":
        return data
    data = data.replace(b"\r\n", b"\n")
    if row["newline"] == "lf":
        return data
    if row["newline"] == "crlf":
        return data.replace(b"\n", b"\r\n")
    raise ValueError(row["newline"])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--private-dir", type=Path)
    args = parser.parse_args()
    plan = json.loads((BASE / "ledger-replay-order.json").read_text())
    ledgers = json.loads((BASE / "migration-ledgers.json").read_text())
    total = 0
    for schema, rows in plan.items():
        expected = {row[0]: row[1].lower() for row in ledgers[schema]}
        for row in rows:
            data = read_input(row, args.private_dir)
            if (hashlib.sha256(data).hexdigest() != row["sha256"]
                    or hashlib.sha1(data).hexdigest() != expected[row["name"]]):
                raise ValueError(schema + "/" + row["name"] + " failed hash verification")
            total += 1
    print(f"Verified {total} replay inputs")


if __name__ == "__main__":
    main()
