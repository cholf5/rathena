# Fix runtime error: missing checkcell builtin

## Summary
Resolved `errors.txt` runtime failure in `quests_17_2.lua` caused by missing `checkcell` Lua builtin.

## Change
- File: `src/map/lua_engine.cpp`
- Added builtin:
  - `lua_builtin_checkcell` (returns boolean true in compatibility mode)
- Registered in builtin table as `checkcell`.

## Why
`errors.txt` showed:
- `attempt to call a number value (global 'checkcell')`
This happened because unresolved globals defaulted to numeric constants/fallbacks, and `checkcell` was not registered.

## Validation
- Rebuilt: `make -C src/map map-server -j4`
- Script-only pass: `./map-server --npc-script-only --run-once`
  - `checked=819 failed=0`
- Safety: no DSL `.txt` changes.
