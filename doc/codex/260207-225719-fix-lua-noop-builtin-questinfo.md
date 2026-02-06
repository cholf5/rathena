# Fix: Lua runtime no-op builtin for questinfo

## Context
After converting `novice/academy` and `doram/spirit_handler` to Lua, `errors.txt` showed many runtime errors:

- `attempt to call a number value (global 'questinfo')`

Affected scripts included:
- `npc/re/jobs/novice/academy.lua`
- `npc/re/jobs/doram/spirit_handler.lua`

## Root Cause
`questinfo` was not present in Lua builtin registration, so name lookup fell through to variable/default resolution and returned numeric value, causing call failure.

## Change
Updated:
- `src/map/lua_engine.cpp`

Added `questinfo` to Lua noop builtins table.

## Validation
- `make -j4` passed.
- `./map-server` not run by Codex (per workflow).
