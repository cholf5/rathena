# Gray Batch: RE warps (all entries in scripts_warps.conf)

## Scope
Converted `npc/re/warps/**` from `.txt` to `.lua` and switched all corresponding entries in:
- `npc/re/scripts_warps.conf`

Coverage includes:
- `cities/*`
- `dungeons/*`
- `fields/*`
- `other/*`
- `guildcastles`

Commented entry also aligned to `.lua`:
- `//npc: npc/re/warps/other/TrainingZone.lua`

## Conversion Validation
- Report: `.codex_tmp/re_warps_batch1/report.md`
- Result: `total_files: 58`, `unsupported_items: 0`
- Lua syntax: `loadfile` checks passed for all generated files

## Build Validation
- `make -j4` passed

## Notes
- `./map-server` was not executed by Codex in this step.
