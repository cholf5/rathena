# Pause bulk migration; improve Lua script system variable model

## Context
Per request, bulk `.txt -> .lua` migration is paused. Focus shifted to script system quality:
- keep old scripts compatible,
- provide clean Lua-native scoped variable APIs for new scripts,
- validate with small Lua sample scripts.

## Runtime API additions
File: `src/map/lua_engine.cpp`

Already added in prior step:
- Generic: `get_var(scope, name[, index])`, `set_var(scope, name, value[, index])`
- Scoped funcs: `get/set_{character,character_temp,account,account_global,map_server,npc,npc_temp,instance}_var`

New in this step:
- Semantic alias tables in Lua environment, each exposing `.get/.set`:
  - `var`
  - `character`
  - `character_temp`
  - `account`
  - `account_global`
  - `map_server`
  - `npc_var`
  - `npc_temp`
  - `instance`

## Sample Lua scripts (new-style usage)
Added:
- `npc/test/lua_scope_api_demo.lua`
- `npc/test/lua_scope_api_array_demo.lua`

Loaded from:
- `npc/scripts_test.conf`

These examples avoid DSL-style variable names and use scoped APIs directly.

## Validation
- Build: `make -C src/map map-server -j4` passed.
- Syntax: `luac -p` passed for both new sample scripts.
- Script-only validation: `./map-server --npc-script-only --run-once` passed (`checked=830 failed=0`).

## Safety
- No DSL source (`npc/**/*.txt`) changes.
