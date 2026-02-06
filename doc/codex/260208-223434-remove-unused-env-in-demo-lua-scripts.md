# Remove unused `_ENV` locals in demo Lua scripts

## Summary
Cleaned up redundant `local _ENV = ctx:env()` declarations in demo scripts where `_ENV` was never used.

## Changes
- Updated `/Users/cholf5/dev/rathena/lua/demo/demo_all_objects.lua`
- Updated `/Users/cholf5/dev/rathena/lua/demo/demo_db_var_crud.lua`
- Updated `/Users/cholf5/dev/rathena/lua/demo/demo_npc_advanced.lua`
- Updated `/Users/cholf5/dev/rathena/lua/demo/demo_npc_medium.lua`
- Updated `/Users/cholf5/dev/rathena/lua/demo/demo_runtime_exports.lua`

All edits are no-op cleanup for readability and do not change behavior.

## Validation
- `rg -n "local _ENV = ctx:env\(\)" lua/demo/*.lua` => no matches
- `luac -p lua/demo/*.lua`
