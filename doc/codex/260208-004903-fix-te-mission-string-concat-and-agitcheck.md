# Fix TE mission string concat and agitcheck builtin

## Changes
- Added Lua builtin `agitcheck()` in `/Users/cholf5/dev/rathena/src/map/lua_engine.cpp` and registered it.
- Fixed converted Lua script string concatenation operators in `/Users/cholf5/dev/rathena/npc/re/quests/woe_te/te_mission_main.lua`:
  - Converted DSL-style string `+` to Lua `..` for event names, npc names, messages, and menu text.
  - Preserved numeric arithmetic (`+ 1`) where needed.

## Why
- Runtime error from full map-server run:
  - `attempt to call a number value (global 'agitcheck')`
  - `attempt to add a 'string' with a 'string'` in `te_mission_main.lua`

## Validation
- `make -j4` passed.
- `./map-server --npc-script-only --run-once` passed with:
  - `checked=784 failed=0`
