# Gray Batch: RE jobs remaining 5 files + script2lua constant-token fix

## Scope
Converted and switched the following RE job scripts from `.txt` to `.lua`:

- `npc/re/jobs/2e/rebellion.txt` -> `npc/re/jobs/2e/rebellion.lua`
- `npc/re/jobs/novice/academy.txt` -> `npc/re/jobs/novice/academy.lua`
- `npc/re/jobs/novice/supernovice_ex.txt` -> `npc/re/jobs/novice/supernovice_ex.lua`
- `npc/re/jobs/repair.txt` -> `npc/re/jobs/repair.lua`
- `npc/re/jobs/doram/spirit_handler.txt` -> `npc/re/jobs/doram/spirit_handler.lua`

Updated config entries in:
- `npc/re/scripts_jobs.conf`

## Converter Fix
Updated:
- `src/tool/script2lua.cpp`

Added expression conversion support for symbol-like constants that start with a digit and include underscore (example: `4_M_THIEF_RUMIN`).

Behavior:
- convert such tokens to `_ENV["..."]` during expression conversion,
- prevents malformed Lua syntax in generated code when constants are used as command arguments.

## Validation
- Per-file conversion reports under `.codex_tmp/re_jobs_batch7/*.report.md`
- Each report shows: `unsupported_items: 0`
- Lua syntax check passed for all 5 generated files (`loadfile`)
- `make -j4` passed

## Status
- RE jobs list now only leaves `npc/re/jobs/3-1/guillotine_cross.txt` as `.txt`.
- `./map-server` was not executed by Codex in this step.
