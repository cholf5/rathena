# Add EmmyLua Annotations for Lua Runtime Exports

## Request

Add EmmyLua annotations for all functions/variables exported from C++ runtime to Lua layer.

## Change

Created:
- `/Users/cholf5/dev/rathena/npc/lua/annotations.lua`

Content includes:
- Scope aliases and accessors (`LuaScopeName`, `LuaScopeAccessor`)
- Context/environment types (`LuaNpcContext`, `LuaNpcEnv`)
- Global scope tables: `var`, `character`, `character_temp`, `account`, `account_global`, `map_server`, `npc_var`, `npc_temp`, `instance`
- Typed annotations for all builtins registered in `lua_register_builtins()`
- Typed annotations for compatibility no-op builtins
- Note for builtin `end` (registered in runtime but not representable as Lua identifier)

## Validation

- `luac -p /Users/cholf5/dev/rathena/npc/lua/annotations.lua` passed.
