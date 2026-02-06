# Promote Lua scripts to formal load path and remove `_gold` naming

## Summary
Promoted Lua migration scripts from test-only naming/entry to formal production loading, removed `_gold` file suffixes, and renamed `scripts_gold.conf` to `scripts_lua.conf`.

## Changes
- Renamed files:
  - `/Users/cholf5/dev/rathena/lua/npc/airports/airships_gold.lua` -> `/Users/cholf5/dev/rathena/lua/npc/airports/airships.lua`
  - `/Users/cholf5/dev/rathena/lua/npc/other/monster_race_single_gold.lua` -> `/Users/cholf5/dev/rathena/lua/npc/other/monster_race_single.lua`
  - `/Users/cholf5/dev/rathena/lua/npc/quests/quests_alberta_turtle_sailor_gold.lua` -> `/Users/cholf5/dev/rathena/lua/npc/quests/quests_alberta_turtle_sailor.lua`
  - `/Users/cholf5/dev/rathena/lua/scripts_gold.conf` -> `/Users/cholf5/dev/rathena/lua/scripts_lua.conf`

- Updated formal load chain:
  - `/Users/cholf5/dev/rathena/npc/re/scripts_main.conf`
    - Added `import: lua/scripts_lua.conf`
  - `/Users/cholf5/dev/rathena/npc/pre-re/scripts_main.conf`
    - Added `import: lua/scripts_lua.conf`

- Removed test-only include:
  - `/Users/cholf5/dev/rathena/npc/scripts_test.conf`
    - Removed `npc: lua/scripts_gold.conf`

- Updated Lua docs/config references:
  - `/Users/cholf5/dev/rathena/lua/scripts_lua.conf`
  - `/Users/cholf5/dev/rathena/lua/README.md`
  - `/Users/cholf5/dev/rathena/doc/script_lua.md`

- Runtime fix after formal load validation:
  - Restored `local _ENV = ctx:env()` in handlers that use runtime builtins.
  - Refactored module-level helper functions in formal Lua scripts to receive `env` explicitly (`env.map_server.*`, `env.character.*`, `env.get_var/set_var`) to avoid nil global resolution.

## Validation
- `luac -p lua/npc/airports/airships.lua lua/npc/other/monster_race_single.lua lua/npc/quests/quests_alberta_turtle_sailor.lua lua/demo/*.lua lua/annotations.lua`
- `./map-server --run-once` => `EXIT:0`
- `rg -n "_gold\\.lua|scripts_gold\\.conf" --glob '!doc/codex/**'` => no matches
