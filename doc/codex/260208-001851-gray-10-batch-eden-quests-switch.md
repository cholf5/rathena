# 260208-001851 gray 10-batch eden quests switch

## Scope
Performed another 10 gray migration batches in one run.

## Switched entries
Updated `/Users/cholf5/dev/rathena/npc/re/scripts_athena.conf`:

1. `npc/re/quests/eden/26-40.txt` -> `npc/re/quests/eden/26-40.lua`
2. `npc/re/quests/eden/56-70.txt` -> `npc/re/quests/eden/56-70.lua`
3. `npc/re/quests/eden/eden_100_111.txt` -> `npc/re/quests/eden/eden_100_111.lua`
4. `npc/re/quests/eden/eden_111_120.txt` -> `npc/re/quests/eden/eden_111_120.lua`
5. `npc/re/quests/eden/eden_121_130.txt` -> `npc/re/quests/eden/eden_121_130.lua`
6. `npc/re/quests/eden/eden_common.txt` -> `npc/re/quests/eden/eden_common.lua`
7. `npc/re/quests/eden/eden_quests.txt` -> `npc/re/quests/eden/eden_quests.lua`
8. `npc/re/quests/eden/eden_tutorial.txt` -> `npc/re/quests/eden/eden_tutorial.lua`
9. `npc/re/quests/ninja_quests.txt` -> `npc/re/quests/ninja_quests.lua`
10. `npc/re/quests/magic_books.txt` -> `npc/re/quests/magic_books.lua`

## Validation
- Command:
  - `./map-server --npc-script-only --run-once`
- Result:
  - exit code `0`
  - summary: `checked=744 failed=0`
  - `/Users/cholf5/dev/rathena/errors.txt` line count: `0`
