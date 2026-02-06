# Lua runtime: new scoped variable API (non-DSL style)

## Goal
Start decoupling new Lua scripts from DSL-style variable names like `.@name$`.

## Engine changes
File: `src/map/lua_engine.cpp`

Added generic API:
- `get_var(scope, name[, index])`
- `set_var(scope, name, value[, index])`

Added scope-specific APIs:
- `get_character_var` / `set_character_var`
- `get_character_temp_var` / `set_character_temp_var`
- `get_account_var` / `set_account_var`
- `get_account_global_var` / `set_account_global_var`
- `get_map_server_var` / `set_map_server_var`
- `get_npc_var` / `set_npc_var`
- `get_npc_temp_var` / `set_npc_temp_var`
- `get_instance_var` / `set_instance_var`

Supported scopes in generic API:
- `character`, `char`, `player`
- `character_temp`, `char_temp`, `player_temp`
- `account`
- `account_global`, `global_account`, `account2`
- `map_server`, `server`, `global`
- `npc`
- `npc_temp`, `local`
- `instance`

Behavior:
- Old DSL-style key access is still supported for backward compatibility.
- New APIs return `nil` when variable is absent (instead of forcing DSL default `0`/`""`).
- Indexed access (`index`) reads/writes table-style variable arrays.

## Validation
- Build: `make -C src/map map-server -j4` passed.
- Runtime parse check: `./map-server --npc-script-only --run-once` passed (`checked=830 failed=0`).
- Safety: no `npc/**/*.txt` DSL source changes.
