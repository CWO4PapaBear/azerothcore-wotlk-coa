import argparse
import json
import re
from collections import defaultdict
from pathlib import Path

def update(source, evidence):
    data = json.loads(Path(evidence).read_text())
    assert data["instance"] == [{"map": 650, "script": "instance_trial_of_the_champion"}]
    assert data["vehicle"] == [{"entry": 35491, "accessory_entry": 35451}]
    assert {x["entry"] for x in data["objects"]} == {195709, 195710, 195374, 195375, 195323, 195324}
    assert all(x["type"] == 3 for x in data["objects"])
    creatures = {x["entry"]: x for x in data["creatures"]}
    assert creatures[35451]["difficulty_entry_1"] == 35490
    assert creatures[35451]["ScriptName"] == "boss_black_knight"
    tables = {}
    for name in ("object_loot", "creature_loot", "reference_loot"):
        tables[name] = defaultdict(list)
        for row in data[name]:
            tables[name][row["Entry"]].append(row)

    def expand(table, entry, trail=()):
        assert entry not in trail
        assert entry in tables[table]
        for row in tables[table][entry]:
            if row["QuestRequired"] or not row["LootMode"] & 1 or row["MaxCount"] <= 0:
                continue
            if row["Reference"]:
                yield from expand("reference_loot", row["Reference"], trail + (entry,))
            elif row["Item"]:
                yield row["Item"]

    routes = defaultdict(set)
    for obj in data["objects"]:
        diff = int(obj["entry"] in (195710, 195375, 195324))
        for item in expand("object_loot", obj["data1"]):
            routes[item].add(650 * 4 + diff)
    for entry, diff in ((35451, 0), (35490, 1)):
        for item in expand("creature_loot", creatures[entry]["lootid"]):
            routes[item].add(650 * 4 + diff)
    items = {x["entry"]: x for x in data["items"]}
    routes = {item: values for item, values in routes.items()
              if items[item]["RequiredLevel"] == 80 and items[item]["class"] in (2, 4)
              and items[item]["InventoryType"] and items[item]["Quality"] in (2, 3, 4)}
    assert {47213, 47214, 47215, 47216} <= routes.keys()

    source = Path(source) / "src/server/coa"
    path = source / "Area52CacheCatalogMetadata.inc"
    text = path.read_text()
    start = text.index("static std::map<uint32, CacheCatalogRecord> const CacheCatalog = {")
    end = text.index("\n};", start)
    records = {int(i): tuple(map(int, (cap, level, itemset))) for i, cap, level, itemset in
               re.findall(r"\{\s*(\d+),\s*\{\s*(\d+),\s*(\d+),\s*(\d+)\s*\}", text[start:end])}
    for item in routes:
        row = items[item]
        record = (row["RequiredLevel"], row["ItemLevel"], row["itemset"])
        assert item not in records or records[item] == record
        records[item] = record
    text = text[:text.index("\n", start) + 1] + "".join(
        f"    {{ {item}, {{ {v[0]}, {v[1]}, {v[2]} }} }},\n" for item, v in sorted(records.items())) + text[end:]
    path.write_text(text)

    path = source / "CallboardDungeonDrops.inc"
    text = path.read_text()
    start = text.index("CallboardDropMaps = {")
    end = text.index("\n};", start)
    records = {int(i): {int(v) for v in re.findall(r"\d+", values)} for i, values in
               re.findall(r"\{\s*(\d+),\s*\{([^}]+)", text[start:end])}
    for item, values in routes.items():
        records.setdefault(item, set()).update(values)
    text = text[:text.index("\n", start) + 1] + "".join(
        f"    {{ {item}, {{ " + ", ".join(map(str, sorted(values))) + " } },\n"
        for item, values in sorted(records.items())) + text[end:]
    path.write_text(text)

    path = source / "Area52CacheSourceMetadata.inc"
    text = path.read_text()
    start = text.index("CacheSourceNodes = {")
    end = text.index("\n};", start)
    nodes = [(int(kind), tuple(map(int, re.findall(r"\d+", values)))) for kind, values in
             re.findall(r"\{\s*(\d+),\s*\{([^}]*)", text[start:end])]
    index = {node: i for i, node in enumerate(nodes)}

    def intern(kind, values):
        node = (kind, tuple(sorted(set(values))))
        if node not in index:
            index[node] = len(nodes)
            nodes.append(node)
        return index[node]

    root_start = text.index("CacheItemSources = {")
    root_end = text.index("\n};", root_start)
    roots = {int(item): int(node) for item, node in re.findall(r"\{\s*(\d+),\s*(\d+)\s*\}",
                                                             text[root_start:root_end])}
    for item, values in routes.items():
        children = {intern(2, (value,)) for value in values}
        if item in roots:
            prior = roots[item]
            if nodes[prior][0] == 5:
                children.update(nodes[prior][1])
            else:
                children.add(prior)
        roots[item] = next(iter(children)) if len(children) == 1 else intern(5, children)
    replacement = "".join(f"    {{ {item}, {node} }},\n" for item, node in sorted(roots.items()))
    text = text[:text.index("\n", root_start) + 1] + replacement + text[root_end:]
    text = text[:text.index("\n", start) + 1] + "".join(
        f"    {{ {kind}, {{ " + ", ".join(map(str, values)) + " } },\n"
        for kind, values in nodes) + text[end:]
    path.write_text(text)
    return {"items": sorted(routes), "routes": {str(k): sorted(v) for k, v in routes.items()}}

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", required=True)
    parser.add_argument("--evidence", required=True)
    args = parser.parse_args()
    print(json.dumps(update(args.source_root, args.evidence), sort_keys=True))
