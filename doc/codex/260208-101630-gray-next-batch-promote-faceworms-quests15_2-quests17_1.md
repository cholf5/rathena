# Gray next batch: promote FacewormsNest + quests_15_2 + quests_17_1

## Summary
Continued staged migration and promoted 3 additional RE scripts to Lua.

## Promoted
- `npc/re/instances/FacewormsNest`
- `npc/re/quests/quests_15_2`
- `npc/re/quests/quests_17_1`

## Key fixes
- `FacewormsNest.lua`
  - cleaned malformed converted `do({ ...` + `do({ monster ...` sequences in spawn helpers.
- `quests_15_2.lua`
  - fixed malformed inline statement (`: if (...)`) into normal Lua `if ... then ... end`.
  - fixed malformed `do({` random-coordinate assignment.
- `quests_17_1.lua`
  - fixed malformed leader-menu select conversion.
  - fixed missing `then` in quest loop.
  - rewrote broken `while` loop block to proper structure with explicit increment and closing `end`.

## Validation
- `luac -p` passed for all 3 promoted files.
- `./map-server --npc-script-only --run-once` => `checked=830 failed=0`.
- Remaining RE `.txt` entries with `.lua` counterparts: `4`.
- DSL safety: no `npc/**/*.txt` changes.

## Remaining candidates
- `npc/re/quests/eden/eden_131_140.txt`
- `npc/re/quests/quests_dicastes.txt`
- `npc/re/quests/quests_eclage.txt`
- `npc/re/quests/quests_malangdo.txt`
