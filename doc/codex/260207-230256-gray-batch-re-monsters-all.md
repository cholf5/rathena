# Gray Batch: RE monsters (all entries in scripts_monsters.conf)

## Scope
Converted `npc/re/mobs/**` from `.txt` to `.lua` and switched all corresponding entries in:
- `npc/re/scripts_monsters.conf`

This includes:
- top-level files (e.g. `championmobs`, `int_land`, `towns`, etc.)
- `dungeons/*`
- `fields/*`
- commented entries aligned to `.lua` suffix (`academy`, `tra_fild`)

## Conversion Validation
- Report: `.codex_tmp/re_mobs_batch1/report.md`
- Result: `total_files: 102`, `unsupported_items: 0`
- Lua syntax: `loadfile` checks passed for generated files

## Build Validation
- `make -j4` passed

## Notes
- `./map-server` was not executed by Codex in this step.
