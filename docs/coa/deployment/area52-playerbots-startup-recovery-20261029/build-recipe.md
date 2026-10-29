# Reproduction requirements

Apply server.patch only to the reviewed deployed core baseline listed in verification-summary.json. Check original hashes, normalize the listed existing files to LF, apply, and verify resulting hashes. Apply module.patch to the pinned Brytenwally module commit, preserving the added files' line endings. The offline patch-reproduction check validates exact results.

Configure the native build with `-DCMAKE_INSTALL_PREFIX=/workspace/env/freepick`, matching this realm's runtime layout. ConfigMgr loads module configuration from the compiled `/workspace/env/freepick/etc/modules/` directory. Passing the correct worldserver.conf with `-c` does not override that directory. Confirm compiler commands and the runtime mount before qualifying a candidate.

Use the existing repository tools/verify_all.py build/unit selections and the startup-configuration regression. Keep gameplay environments off. The offline build uses two low-priority CPU jobs and the existing compiler image; no production service is used as a build/test environment.
