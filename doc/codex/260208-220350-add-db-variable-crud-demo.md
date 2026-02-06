# 260208-220350-add-db-variable-crud-demo

## What changed
Added a dedicated Lua demo NPC for script-variable CRUD operations across DB-related scopes.

## Files changed
- Added `/Users/cholf5/dev/rathena/npc/lua/demo_db_var_crud.lua`
  - New NPC: `Lua DB CRUD Demo#lua` (Prontera 162,173)
  - Demonstrates:
    - Create: `set_character_var`, `set_account_var`, `set_account_global_var`, `set_map_server_var`, `set_npc_var`
    - Read: corresponding `get_*` APIs and generic `get_var`
    - Update: increment/append updates and generic `set_var`
    - Delete: `set_*_var(..., nil)` deletion semantics
    - Indexed CRUD: account-scope array-like variable via index argument
- Updated `/Users/cholf5/dev/rathena/npc/lua/scripts_demo.conf`
  - Added `npc: npc/lua/demo_db_var_crud.lua`
- Updated `/Users/cholf5/dev/rathena/npc/lua/README.md`
  - Added entry describing db variable CRUD demo.

## Validation
- `luac -p npc/lua/demo_db_var_crud.lua` passed.
- `./map-server --npc-script-only --run-once` passed with `EXIT:0`.
