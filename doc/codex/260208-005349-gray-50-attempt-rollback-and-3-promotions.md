# Gray 50 Attempt, Rollback, and 3 Promotions

## Summary
- Tried one 50-item gray switch (`.txt` -> `.lua`) in `npc/re/scripts_*.conf`.
- Validation found 48 Lua syntax failures; those entries were automatically rolled back to `.txt`.
- Kept successful entries and then manually repaired/promoted additional files.

## Changes
- Runtime/engine:
  - Added `agitcheck` builtin in `/Users/cholf5/dev/rathena/src/map/lua_engine.cpp`.
- Script fixes:
  - `/Users/cholf5/dev/rathena/npc/re/quests/woe_te/te_mission_main.lua`
    - Fixed string concatenation (`+` -> `..`) and preserved numeric arithmetic.
  - `/Users/cholf5/dev/rathena/npc/re/merchants/pet_trader.lua`
    - Fixed menu string concat and invalid post-increment conversion.
  - `/Users/cholf5/dev/rathena/npc/re/guild3/agit_start_te.lua`
    - Rewrote into valid Lua schema with event handlers and `WoeTimer` label.
  - `/Users/cholf5/dev/rathena/npc/re/quests/quests_aldebaran.lua`
    - Fixed malformed `else` conversion and block structure in second NPC script.
- Config promotions kept:
  - `/Users/cholf5/dev/rathena/npc/re/scripts_athena.conf`
    - `npc/re/other/Global_Functions.lua`
    - `npc/re/merchants/pet_trader.lua`
    - `npc/re/quests/quests_aldebaran.lua`
  - `/Users/cholf5/dev/rathena/npc/re/scripts_guild.conf`
    - `npc/re/guild3/agit_start_te.lua`

## Validation
- `make -j4` passed after engine/script updates.
- `./map-server --npc-script-only --run-once` final result:
  - `checked=789 failed=0`

## Notes
- Remaining `.txt` entries that have `.lua` siblings are mostly still syntactically invalid (`luac -p` fails), so next throughput depends on either:
  1) fixing `script2lua` conversion rules, or
  2) file-by-file manual repairs before enabling in conf.
