# Auto-bind Lua runtime environment and remove manual handler `_ENV` requirement

## Summary
Implemented engine-side environment binding so Lua handlers can call runtime exports without writing `local _ENV = ctx:env()` in business code.

## Engine changes
- Updated `/Users/cholf5/dev/rathena/src/map/lua_engine.cpp`:
  - Added `lua_push_runtime_env_table(lua_State*)` helper to build runtime env table with `__index`/`__newindex` hooks.
  - Reused this helper for runtime context env creation in `callfunc` and coroutine launch paths.
  - In `lua_engine_load_file()`, bound loaded chunk `_ENV` upvalue to runtime env table before executing the chunk.
    - This makes top-level helpers and handler globals resolve against runtime exports by default.

## Script/document alignment
- Removed manual `local _ENV = ctx:env()` lines from:
  - `/Users/cholf5/dev/rathena/lua/npc/airports/airships.lua`
  - `/Users/cholf5/dev/rathena/lua/npc/other/monster_race_single.lua`
  - `/Users/cholf5/dev/rathena/lua/npc/quests/quests_alberta_turtle_sailor.lua`
  - `/Users/cholf5/dev/rathena/lua/demo/*.lua`
- Updated `/Users/cholf5/dev/rathena/doc/script_lua.md` to document engine-side automatic env binding and removed stale code examples requiring manual `_ENV` lines.

## Build/test
- Rebuilt map-server with updated source:
  - `make -C src/map map-server -j4`
- Syntax checks:
  - `luac -p lua/npc/airports/airships.lua lua/npc/other/monster_race_single.lua lua/npc/quests/quests_alberta_turtle_sailor.lua lua/demo/*.lua lua/annotations.lua`
- Runtime check:
  - `./map-server --run-once` => `EXIT:0`
