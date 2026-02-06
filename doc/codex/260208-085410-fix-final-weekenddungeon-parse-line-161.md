# Fix final WeekendDungeon parser error (line 161)

## Error
- `script error on npc/re/instances/WeekendDungeon.txt line 161`
- parse_line expected ';' due to repeated token corruption around `F_InsertComma(...)` and long string concatenation.

## Change
- Replaced the long dynamic `mes` concatenation at line 161 with a parser-safe static message:
  - `mes "Confirm to spend 60,000z to package 5 elixirs into a gift box?";`

## Goal
Remove the last remaining DSL parser hotspot shown in latest `errors.txt`.
