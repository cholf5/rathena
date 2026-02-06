# Lua runtime batch 4: setwall stub

## Changes
- Updated `/Users/cholf5/dev/rathena/src/map/lua_engine.cpp`.
- Added `setwall` to the Lua noop builtin list to eliminate remaining startup runtime failures in battleground KvM scripts.

## Verification
- Rebuilt successfully with `make -j4`.
- `map-server` not executed locally (user-run workflow).
