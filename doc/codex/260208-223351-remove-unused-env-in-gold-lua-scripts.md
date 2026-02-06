# Remove unused `_ENV` locals in gold Lua scripts

## Summary
Removed redundant `local _ENV = ctx:env()` lines from gold sample scripts where `_ENV` was never referenced.

## Changes
- Updated `/Users/cholf5/dev/rathena/lua/npc/airports/airships_gold.lua`
- Updated `/Users/cholf5/dev/rathena/lua/npc/other/monster_race_single_gold.lua`
- Updated `/Users/cholf5/dev/rathena/lua/npc/quests/quests_alberta_turtle_sailor_gold.lua`

All removed lines were no-op locals and had no behavioral effect.

## Validation
- `rg -n "local _ENV = ctx:env\(\)" lua/npc/airports/airships_gold.lua lua/npc/other/monster_race_single_gold.lua lua/npc/quests/quests_alberta_turtle_sailor_gold.lua` => no matches
- `luac -p lua/npc/airports/airships_gold.lua lua/npc/other/monster_race_single_gold.lua lua/npc/quests/quests_alberta_turtle_sailor_gold.lua`
