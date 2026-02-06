# Fix: mixed-mode regression + Lua missing builtins

## Context
After switching `npc/re/other` to Lua, `errors.txt` showed two issue classes:

1. DSL parse errors in still-DSL scripts (`garden_of_time`, `quests_17_2`, `woe_te/*`) around functions like `F_queststatus`, `WoeTETimeStart`.
2. Lua runtime errors in converted scripts due missing builtins:
   - `getnpcid`
   - `charat`

## Root Cause
- Mixed-engine mode is still active. Converting `Global_Functions` to Lua removed DSL-side function definitions needed by many remaining `.txt` scripts.
- `getnpcid` and `charat` were not registered in Lua builtins table and fell back to numeric default values.

## Changes
1. Reverted mixed-mode critical entry:
- `npc/re/scripts_athena.conf`
  - `npc/re/other/Global_Functions.lua` -> `npc/re/other/Global_Functions.txt`

2. Added missing Lua noop builtins:
- `src/map/lua_engine.cpp`
  - added `getnpcid`
  - added `charat`

## Validation
- `make -j4` passed.
- `./map-server` not run by Codex (per workflow).
