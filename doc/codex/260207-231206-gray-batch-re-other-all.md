# Gray Batch: RE other (all entries in scripts_athena.conf other block)

## Scope
Converted and switched all configured `npc/re/other/*.txt` entries to `.lua`.

Updated config file:
- `npc/re/scripts_athena.conf`

Switched entries include:
- `Global_Functions`, `global_npcs`, `achievements`, `adven_boards`, `bulletin_boards`,
  `clans`, `dimensional_gap`, `item_merge`, `mail`, `mercenary_rent`, `pvp`,
  `resetskill`, `stone_change`, `turbo_track`, `CashShop_Functions`, `kachua_key`, `TrainingZone123`.

## Conversion Validation
- Report: `.codex_tmp/re_other_batch1/report.md`
- Result: `total_files: 17`, `unsupported_items: 0`
- Lua syntax (`loadfile`) passed for all generated files

## Build Validation
- `make -j4` passed

## Notes
- `./map-server` was not executed by Codex in this step.
