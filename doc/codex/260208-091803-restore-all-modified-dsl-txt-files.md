# Restore DSL source files to avoid gameplay-risk changes

## Reason
User flagged a critical issue: DSL source files were modified to bypass parser errors. This is not acceptable for migration correctness.

## Action taken
Restored all modified DSL `.txt` files back to `HEAD` exactly:
- `npc/re/instances/NightmarishJitterbug.txt`
- `npc/re/instances/WeekendDungeon.txt`
- `npc/re/quests/quests_17_2.txt`
- `npc/re/quests/woe_te/te_goditem_alde1.txt`
- `npc/re/quests/woe_te/te_goditem_prt01.txt`

## Verification
- `git diff --name-only -- '*.txt'` => empty
- No remaining `.txt` file changes in working tree.

## Policy update for next batches
- Do not modify any DSL source (`npc/**/*.txt`) for migration convenience.
- Fixes must be in Lua runtime / `script2lua` / generated `.lua` / config routing only.
