# Freeze `script2lua` usage and add Lua gold manual rewrites

## Summary
Switched migration focus to manual high-complexity Lua rewrites (gold samples) and marked `script2lua` as exploratory-only.

## Changes
- Updated `/Users/cholf5/dev/rathena/src/tool/script2lua.cpp`
  - Added top-level status note: limited-maintenance / not primary production migration path.
  - Updated usage output to state converter is exploratory and requires manual rewrite for production.

- Added `/Users/cholf5/dev/rathena/lua/gold/gold_airship_domestic.lua`
  - Manual rewrite from `npc/airports/airships.txt`
  - Covers timer-driven route state, `donpcevent`, touch warp behavior, and duplicated warp portals.

- Added `/Users/cholf5/dev/rathena/lua/gold/gold_monster_race_single.lua`
  - Manual rewrite from `npc/other/monster_race.txt`
  - Covers multi-event NPC flow, timer orchestration, random stat seeding, item issue flow, monster spawn/kill, and winner announcements.

- Added `/Users/cholf5/dev/rathena/lua/gold/gold_turtle_sailor.lua`
  - Manual rewrite from `npc/quests/quests_alberta.txt`
  - Covers branching conversation, quest-flag checks, zeny deduction via scope API, RE/PRE warp branch.

- Added `/Users/cholf5/dev/rathena/lua/scripts_gold.conf`
  - Optional load list for the gold manual rewrites.

- Updated `/Users/cholf5/dev/rathena/lua/README.md`
  - Documented the new `gold/` scripts and `scripts_gold.conf` entrypoint.

## Validation
- `luac -p lua/gold/gold_airship_domestic.lua lua/gold/gold_monster_race_single.lua lua/gold/gold_turtle_sailor.lua lua/annotations.lua`
