# 260208-000903 gray 10-batch quests guild custom switch

## Scope
Performed another 10 gray migration batches in one run (no per-batch pause).

## Switched entries
### `/Users/cholf5/dev/rathena/npc/re/scripts_athena.conf`
1. `npc/re/quests/eden/41-55.txt` -> `npc/re/quests/eden/41-55.lua`
2. `npc/re/quests/eden/eden_service.txt` -> `npc/re/quests/eden/eden_service.lua`
3. `npc/re/quests/mrsmile.txt` -> `npc/re/quests/mrsmile.lua`
4. `npc/re/quests/pile_bunker.txt` -> `npc/re/quests/pile_bunker.lua`
5. `npc/re/quests/quests_13_1.txt` -> `npc/re/quests/quests_13_1.lua`
6. `npc/re/quests/quests_glastheim.txt` -> `npc/re/quests/quests_glastheim.lua`
7. `npc/re/quests/quests_lighthalzen.txt` -> `npc/re/quests/quests_lighthalzen.lua`
8. `npc/re/custom/lasagna/lasa_dun.txt` -> `npc/re/custom/lasagna/lasa_dun.lua`
9. `npc/re/custom/lasagna/lasa_fild.txt` -> `npc/re/custom/lasagna/lasa_fild.lua`

### `/Users/cholf5/dev/rathena/npc/re/scripts_guild.conf`
10. `npc/re/guild/mission_npc.txt` -> `npc/re/guild/mission_npc.lua`

## Validation
- Command:
  - `./map-server --npc-script-only --run-once`
- Result:
  - exit code `0`
  - summary: `checked=714 failed=0`
  - `/Users/cholf5/dev/rathena/errors.txt` line count: `0`

## Note
Skipped `Global_Functions` in this batch on purpose to avoid high-coupling risk during staged migration.
