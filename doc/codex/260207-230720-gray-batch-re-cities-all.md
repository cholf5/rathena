# Gray Batch: RE cities (all entries in scripts_athena.conf city block)

## Scope
Converted and switched all configured `npc/re/cities/*.txt` entries to `.lua`.

Updated config file:
- `npc/re/scripts_athena.conf`

Switched files:
- `alberta`, `comodo`, `dewata`, `dicastes`, `eclage`, `izlude`, `jawaii`, `lutie`, `malangdo`, `malaya`, `mora`, `prontera`, `yuno`.

## Conversion Validation
- Report: `.codex_tmp/re_cities_batch1/report.md`
- Result: `total_files: 13`, `unsupported_items: 0`
- Lua syntax (`loadfile`) passed for all generated city files

## Build Validation
- `make -j4` passed

## Notes
- `./map-server` was not executed by Codex in this step.
