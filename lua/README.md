# Lua Demo Scripts

This folder contains Lua gameplay script demos using the structured schema from `doc/script_lua.md` and `doc/script_lua_schema.md`.

Files:
- `demo/demo_all_objects.lua`: covers every top-level object type (`warps`, `monsters`, `mapflags`, `shops`, `duplicates`, `scripts`, `functions`).
- `demo/demo_npc_medium.lua`: medium-complexity NPC flow (branching, state, helper functions).
- `demo/demo_npc_advanced.lua`: advanced Lua usage (tables, closures, pcall, sorting, functional-style helpers).
- `demo/demo_runtime_exports.lua`: compatibility-runtime API showcase (`monster/map/unit/shop/timer/effect` helpers).
- `demo/demo_db_var_crud.lua`: CRUD demo for DB-variable scopes (`character`, `account`, `account_global`, plus `map_server`/`npc` visibility).
- `scripts_demo.conf`: optional load list for these demos.
- `scripts_lua.conf`: formal load list for Lua-authored gameplay scripts.
- `npc/airports/airships.lua`: timer-driven domestic airship route conversion from `npc/airports/airships.txt`.
- `npc/other/monster_race_single.lua`: multi-event/timer/item/runner-flow conversion from `npc/other/monster_race.txt`.
- `npc/quests/quests_alberta_turtle_sailor.lua`: dialogue + zeny-fee + warp flow conversion from `npc/quests/quests_alberta.txt`.

The project does not auto-load this folder by default.
To enable demos temporarily, include or add lines from `lua/scripts_demo.conf` in your test config.
`lua/scripts_lua.conf` is imported from primary script config (`npc/re/scripts_main.conf` and `npc/pre-re/scripts_main.conf`).

`demo/demo_runtime_exports.lua` contains side-effectful commands (warp, setcell, unloadnpc, etc.).
Load and run it only on test servers.
