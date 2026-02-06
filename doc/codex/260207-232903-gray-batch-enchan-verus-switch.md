# 260207-232903 gray batch enchan verus switch

## Batch scope
Gray migration batch: switch one RE merchant script from DSL to Lua.

## Changes
- Updated `/Users/cholf5/dev/rathena/npc/re/scripts_athena.conf`:
  - `npc/re/merchants/enchan_verus.txt` -> `npc/re/merchants/enchan_verus.lua`

## Validation
- Syntax check passed:
  - `lua -e "loadfile('npc/re/merchants/enchan_verus.lua')"`

## Environment note
- In Codex sandbox, `./map-server --run-once` currently cannot validate script runtime because map-server DB connect fails with:
  - `Can't connect to server on '127.0.0.1' (1)`
- This appears to be sandbox network restriction, not script syntax.

## Next
- Continue gray conversion with one file per batch.
- Prefer entries that pass `loadfile` before switching in conf.
