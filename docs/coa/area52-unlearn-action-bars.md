# Free Pick unlearned spell buttons

Resetting selections removed learned spells but left their action buttons behind. The isolated native reproduction confirmed Cold Snap (11958) was unlearned while button 0 still referenced it.

Selection reconciliation now snapshots learned spell buttons and clears only buttons whose spell is no longer learned after linked-spell and Mystic Enchant reconciliation. A player spell-forgotten hook also queues generic removals for reconciliation at the next player update or save, covering removals outside selection resets. State is per player. Known spells and non-spell buttons are retained; internal synchronous remove/regrant operations are evaluated using their final state. Removed buttons use the existing database persistence and action-bar update packet.

Upstream reviewed: jealous-sound/azerothcore-wotlk-coa main 413e03e986ad5d9a3e1d95f88af7effe39972b4f. Wildcard has an explicit list-based RemoveFromActionBars helper; it does not establish coverage for every spell-forgotten path. Wildcard behavior is unchanged by this Free Pick candidate. A broader upstream contribution still requires a focused Wildcard reproduction and verification.

The effective Area 52 client reset wrapper sends native reset requests and does not clear action bars itself. This repair is server-only and requires no client patch.

Publication compatibility: this checkout uses AscensionFreepick/SyncSpells; the active reconstruction uses AscensionFreePick/Apply. Both consume the identical helper header, but only the active reconstruction binary has been compiled in this run. This branch is not an upstream-ready PR.

Verification: the old native binary failed the removed-button assertion. The repaired runtime build, existing unit selection, and freepick_advancement harness passed. Corrected real-clock native regression passed all 19 steps, including reset cleanup, retention of an unrelated learned spell and cleanup after generic unlearning. VERIFY ALL: PASSED for the focused build/unit/harness and native gameplay runs. Reports: verification-reset-action-bars-build/report.json and verification-reset-action-bars-after/report.json under the local login-persistence verification output. No acceleration or batch sensitivity was reported. Reconnect, cross-spec and temporary remove/regrant runtime coverage remain unverified; no combat certification is claimed. Not activated.
