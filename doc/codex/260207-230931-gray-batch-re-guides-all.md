# Gray Batch: RE guides (all entries in scripts_athena.conf guides block)

## Scope
Converted and switched all configured `npc/re/guides/*.txt` entries to `.lua`.

Updated config file:
- `npc/re/scripts_athena.conf`

Coverage:
- `navigation`
- `guides_*` city files
- `guides_woe_te`

## Conversion Validation
- Report: `.codex_tmp/re_guides_batch1/report.md`
- Result: `total_files: 32`, `unsupported_items: 0`
- Lua syntax (`loadfile`) passed for all generated guide files

## Build Validation
- `make -j4` passed

## Notes
- `./map-server` was not executed by Codex in this step.
