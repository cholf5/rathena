# Fix Lua builtins for repeated runtime errors (charat/flagemblem)

## Why
Latest `errors.txt` showed repeated runtime failures from Lua NPC scripts:
- `attempt to call a number value (global 'flagemblem')` in `npc/re/guild3/agit_main_te.lua`
- `attempt to concatenate a nil value` at `npc/re/quests/woe_te/te_mission_main.lua:53` (uses `charat(...)`)

## Changes
Updated `/Users/cholf5/dev/rathena/src/map/lua_engine.cpp`:
- Added real builtin `charat(str, idx)` returning one-character string or empty string.
- Registered `charat` as builtin (removed it from noop list so it is not overridden).
- Added `flagemblem` to noop builtin list to prevent unresolved-call crashes in TE guild Lua script.

## Validation
- `make -j4` passed.
- `./map-server --npc-script-only --run-once` passed:
  - `checked=793 failed=0`

## Note
`errors.txt` still contains older entries from pre-fix runs; need a fresh full run to confirm deltas.
