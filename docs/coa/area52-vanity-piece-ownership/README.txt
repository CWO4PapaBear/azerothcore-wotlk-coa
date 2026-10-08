Area 52 archetype vanity piece ownership repair

Opening a bundle previously stored bundle ownership and appearance unlocks, leaving individual piece ownership missing. The runtime patch stores all reward IDs in the same transaction and sends collection-added packets. Free Pick login backfills missing pieces from owned bundle IDs only; it does not infer ownership from arbitrary appearance matches. Repeating login is idempotent.

The client/server catalog adds 454 missing single-item entries so all 522 pieces across 56 bundles can be searched, filtered and delivered. Existing rows remain untouched. Entries copy same item-class/subclass category templates; chest/robe and weapon-slot equivalents are used where needed. Missing armor-material templates retain the slot flag with the correct armor-material bit. No purchase costs or learned spells are added.

Runtime patch targets the existing Area 52 reconstruction, not bare upstream. Upstream reviewed: 4580ec77be857a3194dabe54505edc15ca42f0c8. Existing upstream collection logic depends on individual VanityCollection rows. Proprietary DBC/MPQ assets remain outside Git; Stage-Client.py uses the owner baseline and existing bundle manifest.

Verification: build and unit stages passed through verify_all.py; focused harness reproduced the 454 missing mappings and checked complete repaired coverage and preservation of existing records/other archive members. C++ style passed. These checks do not claim in-game acceptance.

Status: source implemented; paired server binary/catalog and client archive staged. Not activated, installed or released. Activation needs both catalogs and the server binary. Existing collectors should relog after activation; no re-opening or new mail required.
