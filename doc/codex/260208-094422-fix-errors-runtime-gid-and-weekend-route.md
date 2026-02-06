# Fix errors.txt: runtime .gid crash + weekend route

## Summary
Addressed current high-frequency `errors.txt` issues without modifying any DSL `.txt` scripts.

## Changes
1. `src/map/lua_engine.cpp`
- Added `npcspeed` to noop builtins.
- Missing `$@*` variables now default to `{}` instead of numeric `0`.

2. `npc/re/scripts_athena.conf`
- Switched `npc/re/instances/WeekendDungeon.txt` -> `npc/re/instances/WeekendDungeon.lua`.

3. `npc/re/quests/quests_14_3_bis.lua`
- In NPC `#f_boss_c` `OnInit`, initialized `_ENV[".gid"] = {}` before indexed writes.

## Validation
- Rebuild: `make -C src/map map-server -j4` (done earlier in this cycle for runtime changes).
- `./map-server --npc-script-only --run-once` => `checked=817 failed=0`.
- `luac -p npc/re/quests/quests_14_3_bis.lua` => pass.
- DSL safety check: `git diff --name-only -- 'npc/**/*.txt'` => empty.

## Remaining
Current `errors.txt` still contains DSL parser failures from:
- `npc/re/instances/NightmarishJitterbug.txt`
- `npc/re/quests/quests_17_2.txt`
These require either valid Lua replacements (preferred) or parser-side handling, without editing DSL sources.
