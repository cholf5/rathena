# 260208-000318 gray batch enchan mora switch

## Batch scope
Gray migration batch: switch one RE merchant script from DSL to Lua.

## Changes
- Updated `/Users/cholf5/dev/rathena/npc/re/scripts_athena.conf`:
  - `npc/re/merchants/enchan_mora.txt` -> `npc/re/merchants/enchan_mora.lua`

## Validation
- Command:
  - `./map-server --npc-script-only --run-once`
- Result:
  - exit code `0`
  - summary `checked=694 failed=0`
  - `/Users/cholf5/dev/rathena/errors.txt` is empty

## Remaining deferred merchant txt entries
- `npc/re/merchants/enchan_rockridge.txt`
- `npc/re/merchants/malangdo_costume.txt`
- `npc/re/merchants/novice_vending_machine.txt`
- `npc/re/merchants/pet_trader.txt`

## Notes
- `enchan_rockridge.lua`, `malangdo_costume.lua`, `pet_trader.lua` still have Lua syntax issues from converter output and should be fixed before conf switch.
