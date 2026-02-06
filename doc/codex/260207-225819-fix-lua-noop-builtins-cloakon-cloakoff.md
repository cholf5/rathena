# Fix: Lua runtime no-op builtins for cloakonnpc/cloakoffnpc

## Context
Latest `errors.txt` reported runtime failures:
- `attempt to call a number value (global 'cloakonnpc')`

Affected in:
- `npc/re/jobs/novice/academy.lua`

## Root Cause
`cloakonnpc` was not registered in Lua builtin table; unresolved symbol fell back to numeric default variable value and failed when called.

## Change
Updated:
- `src/map/lua_engine.cpp`

Added to Lua noop builtin list:
- `cloakonnpc`
- `cloakoffnpc` (proactively, paired API)

## Validation
- `make -j4` passed.
- `./map-server` not run by Codex (per workflow).
