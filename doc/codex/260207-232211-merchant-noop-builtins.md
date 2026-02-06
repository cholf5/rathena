# 260207-232211 merchant noop builtins

## Background
After switching another RE merchants batch to `.lua`, `errors.txt` showed repeated Lua runtime failures:
- `attempt to call a number value (global 'setunitdata')`
- `attempt to call a number value (global 'setunittitle')`
- `attempt to call a number value (global 'npcshopdelitem')`

## Changes
- Updated `/Users/cholf5/dev/rathena/src/map/lua_engine.cpp`.
- Added the following names to `noop_builtins[]` in `lua_register_builtins()`:
  - `setunitdata`
  - `setunittitle`
  - `npcshopdelitem`

## Why
Current migration strategy allows unsupported commands to be tolerated during staged conversion. These commands are used during NPC init/setup paths in converted merchant scripts and were not yet registered in Lua, causing startup errors.

## Validation
- Build command: `make -j4`
- Result: success (`map-server` relinked successfully).

## Next
- User reruns `./map-server --run-once 2> errors.txt`.
- Continue with next missing builtin/function from refreshed `errors.txt`.
