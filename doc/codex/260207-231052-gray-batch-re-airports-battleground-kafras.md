# Gray Batch: RE airports / battleground / kafras (small block)

## Scope
Converted and switched the following entries from `.txt` to `.lua` in `npc/re/scripts_athena.conf`:

- `npc/re/airports/izlude.lua`
- `npc/re/battleground/bg_common.lua`
- `npc/re/kafras/cool_event_corp.lua`
- `npc/re/kafras/kafras.lua`
- `npc/re/kafras/Kafra_Teleportation_Services.lua`
- `npc/re/kafras/Zonda_Teleportation_Services.lua`

## Conversion Validation
Reports:
- `.codex_tmp/re_misc_batch1/airports.report.md`
- `.codex_tmp/re_misc_batch1/battleground.report.md`
- `.codex_tmp/re_misc_batch1/kafras.report.md`

Result:
- all converted files `unsupported_items: 0`
- Lua syntax (`loadfile`) passed for all generated files

## Build Validation
- `make -j4` passed

## Notes
- `./map-server` was not executed by Codex in this step.
