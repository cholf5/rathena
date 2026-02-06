# 260208-001744 gray 10-batch quests eden merchant guild switch

## Scope
Performed another 10 gray migration batches in one run.

## Switched entries
### `/Users/cholf5/dev/rathena/npc/re/scripts_athena.conf`
1. `npc/re/merchants/novice_vending_machine.txt` -> `npc/re/merchants/novice_vending_machine.lua`
2. `npc/re/quests/eden/11-25.txt` -> `npc/re/quests/eden/11-25.lua`
3. `npc/re/quests/eden/86-90.txt` -> `npc/re/quests/eden/86-90.lua`
4. `npc/re/quests/eden/91-99.txt` -> `npc/re/quests/eden/91-99.lua`
5. `npc/re/quests/seals/brisingamen_seal.txt` -> `npc/re/quests/seals/brisingamen_seal.lua`
6. `npc/re/quests/quests_14_3.txt` -> `npc/re/quests/quests_14_3.lua`
7. `npc/re/quests/quests_morocc.txt` -> `npc/re/quests/quests_morocc.lua`
8. `npc/re/quests/woe_te/te_mission_alde.txt` -> `npc/re/quests/woe_te/te_mission_alde.lua`
9. `npc/re/quests/woe_te/te_mission_prt.txt` -> `npc/re/quests/woe_te/te_mission_prt.lua`

### `/Users/cholf5/dev/rathena/npc/re/scripts_guild.conf`
10. `npc/re/guild/invest_npc.txt` -> `npc/re/guild/invest_npc.lua`

## Validation
- Command:
  - `./map-server --npc-script-only --run-once`
- Result:
  - exit code `0`
  - summary: `checked=734 failed=0`
  - `/Users/cholf5/dev/rathena/errors.txt` line count: `0`
