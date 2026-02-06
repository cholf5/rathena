# Fix errors.txt: npcspeed runtime call + $@ array default crash

## Summary
- Fixed two Lua runtime root causes observed in `errors.txt`.
- Switched one remaining problematic RE instance entry to `.lua` to avoid known DSL parse errors.

## Changes
1. `src/map/lua_engine.cpp`
- In variable fallback logic, missing keys prefixed with `$@` now default to an empty Lua table (instead of numeric `0`).
- Added `npcspeed` to noop builtins registration, matching current compatibility behavior.

2. `npc/re/scripts_athena.conf`
- Switched:
  - `npc/re/instances/WeekendDungeon.txt`
  - to `npc/re/instances/WeekendDungeon.lua`

## Verification
- Rebuilt map server binary:
  - `make -C src/map map-server -j4`
- Script-only validation:
  - `./map-server --npc-script-only --run-once`
  - Result: `checked=817 failed=0`

## Safety
- Confirmed no DSL source edits remain:
  - `git diff --name-only -- 'npc/**/*.txt'` is empty.
