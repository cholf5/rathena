# 260207-232314 merchant noop builtins round2

## Background
New `errors.txt` after previous fix dropped to 12 lines and exposed two additional missing Lua globals during merchant init:
- `npcshopupdate`
- `movenpc`

## Changes
- Updated `/Users/cholf5/dev/rathena/src/map/lua_engine.cpp`.
- Added to `noop_builtins[]`:
  - `npcshopupdate`
  - `movenpc`

## Validation
- Build command: `make -j4`
- Result: success (`map-server` relinked).

## Next
- User reruns `./map-server --run-once 2> errors.txt`.
- Continue eliminating remaining missing builtins/functions from updated `errors.txt`.
