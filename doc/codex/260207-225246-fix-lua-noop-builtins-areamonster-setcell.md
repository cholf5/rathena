# Fix: Lua runtime no-op builtins for areamonster/setcell

## Context
After switching more RE job scripts to Lua, `map-server --run-once` produced runtime errors:

- `attempt to call a number value (global 'areamonster')`
- `attempt to call a number value (global 'setcell')`

These were reported from:
- `npc/re/jobs/2e/kagerou_oboro.lua`
- `npc/re/jobs/3-1/archbishop.lua`

## Root Cause
`lua_env_index()` checks builtins first, then variable/default resolution.
`areamonster` and `setcell` were not included in the Lua builtin table, so they fell through to variable/default lookup and became numeric values, then failed when called like functions.

## Change
Updated noop builtin registration list in:
- `src/map/lua_engine.cpp`

Added:
- `areamonster`
- `setcell`

## Validation
- `make -j4` passed.
- `./map-server` was not run by Codex per workflow; user should re-run and refresh `errors.txt`.
