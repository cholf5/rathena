# Fix: Lua runtime no-op builtins for atoi/getmonsterinfo

## Context
After switching RE cities/monsters to Lua, `errors.txt` reported:
- `attempt to call a number value (global 'atoi')`
- `attempt to call a number value (global 'getmonsterinfo')`

Affected scripts included:
- `npc/re/cities/malaya.lua`
- `npc/re/mobs/dungeons/lhz_dun.lua`

## Root Cause
Missing Lua builtin registration caused unresolved symbols to fall back to numeric default values and fail when called.

## Change
Updated:
- `src/map/lua_engine.cpp`

Added to Lua noop builtin list:
- `atoi`
- `getmonsterinfo`

## Validation
- `make -j4` passed.
- `./map-server` not run by Codex (per workflow).
