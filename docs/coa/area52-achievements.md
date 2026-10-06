# Area 52 achievement eligibility

`CoA.FreePick.AchievementEligibility = 1` enables the policy for CoA-enabled, live Hero realms whose configured mode mask is zero or contains only the quality-budget bit (0x400). Other class models, realm types and mode masks retain their existing behavior. Configuration reload can toggle eligibility; catalog changes require a restart.

The policy runs before criteria evaluation and before final achievement completion. It follows category ancestry for Seasonal and alternate-mode categories and recognizes explicit mode labels. It does not match generic words such as Nightmare, Season or Resolute alone. Existing character achievements and realm-first completion records are not modified.

Equivalent realm-first records are grouped from the effective Achievement and Achievement_Criteria DBCs. A family must match normalized title, faction, map, parent, category, points, flags, icon, completion count, reference, and all non-localized criterion fields. Numeric ID is only a stable tie-break after equivalence is established; it is not used to infer a realm or game mode. The lowest ID is retained within such a family. Records with SQL criteria conditions, SQL rewards, or referenced achievement progress are conservatively excluded from alias grouping.

In the reviewed catalog, this retains 7399/7348/6334 and excludes equivalent aliases 11399/11348/10334. Nightmare 15334 and Seasonal 16904 are blocked by mode metadata. Normal level achievements, Hero realm-first, Warsong Gulch Ironman, Nightmare Vale and Resolute Cape remain eligible. The audit covered 22603 achievement records and 33598 criteria. One apparently similar Naxxramas realm-first family (1402/9730) is deliberately not collapsed because its SQL conditions and rewards differ; it requires separate content review.

The policy uses current effective data at startup rather than a frozen ID denylist. Startup logs report the excluded count. Failure to load either DBC logs an error and leaves this policy unavailable; treat that log as a failed deployment check. Test with `tools/verify_all.py --stages harness --harness area52_achievement_policy`. The harness also accepts AREA52_ACHIEVEMENT_FIXTURE and AREA52_ACHIEVEMENT_RESULT for an effective-data audit.

Verification used upstream revision 36a3d8b506f8dabaee3faac3cf117a9ee2d03f79 and the primary installed client/server data. Automated checks are not a substitute for awarding fresh exploration achievements in game after activation.
