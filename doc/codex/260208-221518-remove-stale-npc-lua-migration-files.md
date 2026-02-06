# Remove stale migrated Lua files under `npc/`

## Summary
Removed obsolete Lua files generated during the DSL-to-Lua migration from `npc/`, keeping only `npc/test/*.lua` test scripts.

## Changes
- Deleted all `*.lua` under `npc/` except files under `npc/test/`.
- Kept active Lua load points:
  - `npc/scripts_test.conf` -> `npc/test/lua_scope_api_demo.lua`, `npc/test/lua_scope_api_array_demo.lua`
  - `lua/scripts_demo.conf` -> `lua/demo/*.lua`

## Validation
- `rg --files npc -g '*.lua' | rg -v '^npc/test/' | wc -l` => `0`
- `rg -n "^npc:\\s+.*\\.lua$" npc/scripts*.conf lua/scripts*.conf`
