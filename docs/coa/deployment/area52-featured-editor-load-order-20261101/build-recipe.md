# Featured editor load-order correction

The activated editor accessed BuildViewSectionMixin.CategoryIcons at file load. The installed TOC loads BuildEditor/BuildEditorTemplates.xml (including EditableBuildView.lua) before BuildCreatorTemplates.xml (which defines BuildViewSectionMixin). This reproduces the owner's nil-global error. The earlier UI adapter incorrectly supplied that global early; its pass did not establish real addon loading.

Move both new icon registrations into F.Ensure, before section construction and after shared definitions load. Do not pre-create the mixin: the subsequent original definition would overwrite it. The registrations also run before the existing-view early return. No other client member changes, server changes, SQL, addon or DLL.

ui-r3/report.json passes the failing-baseline/corrected-late-definition regression plus the existing editor, picker, preview, serialization, validation and archive checks. The actual section icon definitions are loaded after the candidate. Offline widget adapters do not establish live graphical certification. Preserve the original activated package and all prior evidence; this is a separate correction.

Package preparation includes current publication templates and a single changed launcher component. No installation, restart or publication is authorized. Final package integrity and stable CheckOnly must pass before readiness is claimed.
