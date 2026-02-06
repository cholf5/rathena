# 260208-000636 gray 10-batch quests switch

## Scope
Performed 10 gray migration batches in one run (no per-batch pause), each as `.txt -> .lua` config switch.

## Switched entries
Updated `/Users/cholf5/dev/rathena/npc/re/scripts_athena.conf`:

1. `npc/re/quests/first_class/tu_archer.txt` -> `npc/re/quests/first_class/tu_archer.lua`
2. `npc/re/quests/seals/megingard_seal.txt` -> `npc/re/quests/seals/megingard_seal.lua`
3. `npc/re/quests/cooking_quest.txt` -> `npc/re/quests/cooking_quest.lua`
4. `npc/re/quests/monstertamers.txt` -> `npc/re/quests/monstertamers.lua`
5. `npc/re/quests/quest_payon.txt` -> `npc/re/quests/quest_payon.lua`
6. `npc/re/quests/quests_izlude.txt` -> `npc/re/quests/quests_izlude.lua`
7. `npc/re/quests/quests_nameless.txt` -> `npc/re/quests/quests_nameless.lua`
8. `npc/re/quests/quests_niflheim.txt` -> `npc/re/quests/quests_niflheim.lua`
9. `npc/re/quests/quests_veins.txt` -> `npc/re/quests/quests_veins.lua`
10. `npc/re/quests/the_sign_quest.txt` -> `npc/re/quests/the_sign_quest.lua`

## Validation
- Command:
  - `./map-server --npc-script-only --run-once`
- Result:
  - exit code `0`
  - summary: `checked=704 failed=0`
  - `/Users/cholf5/dev/rathena/errors.txt` line count: `0`

## Note
This run followed the same gray process but grouped into 10 batches before reporting.
