# Area 52 Development repair promotion

The Cloud release combines White Walker equipment/grants/first Ice Elemental, Mystic Altar access and charge safety, warrior stance/Stealth restoration and explicit stance selection, Defias weapon-form eligibility, Sunfire replacement inheritance/New Moon and Waning Moon tooltip support, Druid of the Eclipse verification repairs, and Legacy of the Arcanist.

The two pending world migrations are ordered in release.json. No character/account migration or runtime configuration replacement is part of this release. The existing cache, catalog, certification and Retreat purchase systems are retained. Temporary Development diagnostic logging is not included in the Cloud binary.

Run verification through tools/verify_all.py. The native build/unit and the focused Arcanist, Defias weapon-form, altar charge/policy, stance/Stealth, two-hand marker and White Walker grant/starter harnesses passed on this candidate. The unchanged Druid mechanics additionally retain their matching prior build/lifecycle/proc qualification. Native adapters do not certify live gameplay.

Harness prerequisites: C++20 compiler, approved Area 52 DBCs mounted at /data/dbc; the altar compile probe reads the qualified build compile_commands.json at /botbuild. SQL harnesses use a disconnected MySQL fixture. The range-display Lua harness uses Lua 5.1 through Lupa and approved client staging at /client-staging. Client/archive and host-specific activation fixtures remain outside source history with the private package evidence; they are not silently presented as portable tests.

Client release artifacts belong in the existing client feed's release assets, not this repository. The 50-record tooltip update and GameTooltip range bridge are distributed through that feed. The portable Featured catalog worker adopts the verified new archive/manifest baseline while retaining curated inputs, queued notifications and retry state.

Verified is engineering status. Certified remains tester-owned. See release.json for exact source/artifact and playerbots provenance.
