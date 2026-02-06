# Relocate gold samples to mirrored `lua/npc/...` layout

## Summary
Moved manual gold rewrite scripts from `lua/gold/` to a source-mirrored structure under `lua/npc/...` for better traceability to original DSL paths.

## Changes
- Moved files:
  - `/Users/cholf5/dev/rathena/lua/gold/gold_airship_domestic.lua` -> `/Users/cholf5/dev/rathena/lua/npc/airports/airships_gold.lua`
  - `/Users/cholf5/dev/rathena/lua/gold/gold_monster_race_single.lua` -> `/Users/cholf5/dev/rathena/lua/npc/other/monster_race_single_gold.lua`
  - `/Users/cholf5/dev/rathena/lua/gold/gold_turtle_sailor.lua` -> `/Users/cholf5/dev/rathena/lua/npc/quests/quests_alberta_turtle_sailor_gold.lua`
- Removed now-empty `lua/gold/` directory.
- Updated `/Users/cholf5/dev/rathena/lua/scripts_gold.conf` to point at `lua/npc/...` paths.
- Updated `/Users/cholf5/dev/rathena/lua/README.md` gold sample path references.

## Validation
- `luac -p lua/npc/airports/airships_gold.lua lua/npc/other/monster_race_single_gold.lua lua/npc/quests/quests_alberta_turtle_sailor_gold.lua lua/annotations.lua`
