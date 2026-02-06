# Relocate Lua scripts from `npc/lua` to root `lua/`

## Summary
Moved Lua runtime annotation and demo assets to a root-level `lua/` directory, with demos isolated under `lua/demo/`.

## Changes
- Updated `/Users/cholf5/dev/rathena/lua/scripts_demo.conf`
  - Repointed entries from `npc/lua/*.lua` to `lua/demo/*.lua`
- Updated `/Users/cholf5/dev/rathena/lua/README.md`
  - Updated demo file paths to `demo/*.lua`
  - Updated enablement path from `npc/lua/scripts_demo.conf` to `lua/scripts_demo.conf`
  - Updated runtime demo note path to `demo/demo_runtime_exports.lua`
- Removed empty directory `/Users/cholf5/dev/rathena/npc/lua`

## Validation
- `luac -p lua/annotations.lua lua/demo/demo_all_objects.lua lua/demo/demo_npc_medium.lua lua/demo/demo_npc_advanced.lua lua/demo/demo_runtime_exports.lua lua/demo/demo_db_var_crud.lua`
- `./map-server --npc-script-only --run-once` (exit code 0)
