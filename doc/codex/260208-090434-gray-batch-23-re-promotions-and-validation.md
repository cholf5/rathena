# Gray Batch: 23 RE Lua promotions + validation

## Summary
- Continued gray migration in grouped mode.
- Revalidated remaining RE `.txt` entries with existing `.lua` siblings by running `script2lua --overwrite` + `luac -p`.
- Promoted 23 RE entries from `.txt` to `.lua` in `npc/re/scripts_athena.conf`.

## Promoted Entries (txt -> lua)
- npc/re/instances/BakonawaLake
- npc/re/instances/FridayDungeon
- npc/re/instances/LostFarm
- npc/re/instances/MalangdoCulvert
- npc/re/instances/ThorGunsuBase
- npc/re/instances/HorrorToyFactory
- npc/re/instances/HiddenGarden
- npc/re/instances/Regenschirm
- npc/re/instances/RitualOfBlessing
- npc/re/instances/WaterGarden
- npc/re/instances/WernerLaboratoryCentralRoom
- npc/re/instances/Wolves
- npc/re/quests/newgears/2012_headgears
- npc/re/quests/quests_14_3_bis
- npc/re/quests/quests_15_1
- npc/re/quests/quests_16_2
- npc/re/quests/quests_illusion_dungeons
- npc/re/quests/quests_18
- npc/re/quests/quests_dungeons_200
- npc/re/quests/quests_rockridge
- npc/re/quests/woe_te/te_goditem_prt01
- npc/re/quests/woe_te/te_goditem_alde1
- npc/re/custom/lasagna/lasagna_npcs

## Validation
- `./map-server --npc-script-only --run-once`
  - passed
  - `Lua NPC validation done. checked=816 failed=0`
- `./map-server --run-once`
  - failed due DB connection in this sandbox run:
  - `Can't connect to server on '127.0.0.1' (1)`

## Notes
- During this run, `npc/re/scripts_athena.conf` was accidentally truncated by a bad one-liner and then restored from `HEAD` immediately using `git show HEAD:npc/re/scripts_athena.conf > npc/re/scripts_athena.conf`, after which all intended switches were re-applied safely.
