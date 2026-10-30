# Companion package reproduction

Start with the deployed area52-playerbots-startup-recovery-20261029 core baseline; apply server.patch. Its exact before/after hashes and full qualified core overlays are in verification-summary.json. Preserve the prior recovery/Arena1v1/realm changes.

For mod-playerbots, start at the module_commit in verification-summary.json and apply module.patch. All changed module hashes are recorded. The full patch includes the earlier qualified fixed-pool and recruitment changes; only differences from deployed module source are installed by this package.

Build incrementally with the existing WSL compiler image and tools/verify_all.py, CMAKE_INSTALL_PREFIX=/workspace/env/freepick, two low-priority jobs. Run unit and area52-companion-final harness selections. No gameplay server is required. The packaged stripped executable is checked against the compiled executable's allocated ELF sections.

Activation uses the stable runner's guarded source/binary backup and rollback. No roster recreation, SQL migrations, configuration changes or client distribution are included. Runtime-owned bot gear and appearances persist through a binary rollback; human characters are outside the assignment scope.
