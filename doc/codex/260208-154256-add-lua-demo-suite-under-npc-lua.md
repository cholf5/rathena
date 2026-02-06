# Add Lua Demo Suite Under `npc/lua/`

## Goal

Provide example Lua gameplay scripts for new development using the structured schema.

## Added files

- `/Users/cholf5/dev/rathena/npc/lua/README.md`
- `/Users/cholf5/dev/rathena/npc/lua/scripts_demo.conf`
- `/Users/cholf5/dev/rathena/npc/lua/demo_all_objects.lua`
- `/Users/cholf5/dev/rathena/npc/lua/demo_npc_medium.lua`
- `/Users/cholf5/dev/rathena/npc/lua/demo_npc_advanced.lua`

## Coverage

- `demo_all_objects.lua` covers all top-level object types:
  - `warps`, `monsters`, `mapflags`, `shops`, `duplicates`, `scripts`, `functions`
- NPC demos include three levels:
  - simple dialog (`mes/next/close`)
  - medium branching/state (`if/else`, `select`, scoped var APIs)
  - advanced Lua style (closures, table sorting, deterministic shuffle, `pcall`, composed helper functions)

## Validation

- Syntax check passed with `luac -p` for all three demo files.
