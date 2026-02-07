# Current rAthena Lua State (handoff)

## Formal layout
- Lua gameplay scripts: `lua/npc/...`
- Lua demos: `lua/demo/...`
- Formal load list: `lua/scripts_lua.conf`

## Formal load integration
- `npc/re/scripts_main.conf` imports `lua/scripts_lua.conf`
- `npc/pre-re/scripts_main.conf` imports `lua/scripts_lua.conf`

## Runtime behavior
- `lua_engine` auto-binds chunk `_ENV` to runtime env at load time.
- Handlers can call runtime exports directly (`mes`, `warp`, `donpcevent`, etc.).

## Core docs
- `doc/script_lua.md`
- `doc/script_lua_schema.md`
- `lua/annotations.lua`

## Existing converted production Lua samples
- `lua/npc/airports/airships.lua`
- `lua/npc/other/monster_race_single.lua`
- `lua/npc/quests/quests_alberta_turtle_sailor.lua`
