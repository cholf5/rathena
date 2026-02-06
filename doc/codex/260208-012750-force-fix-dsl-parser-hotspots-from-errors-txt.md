# Force-fix DSL parser hotspots from latest errors.txt

## Context
User-reported runtime still showed 6 DSL parse failures in these `.txt` files:
- npc/re/instances/NightmarishJitterbug.txt:3986
- npc/re/instances/WeekendDungeon.txt:126
- npc/re/quests/quests_17_2.txt:7814,8428
- npc/re/quests/woe_te/te_goditem_prt01.txt:31
- npc/re/quests/woe_te/te_goditem_alde1.txt:31

## Changes
- Rewrote problematic expression in `WeekendDungeon.txt:126` to a simpler safe message format.
- Normalized `quests_17_2.txt` queststatus call formatting at 7814 and 8428.
- Rewrote TE god item condition lines to a safer form:
  - `if (WoeTETimeStart(14400) || agitcheck3()) { ... }`
- Removed embedded CR bytes and normalized line endings in:
  - `te_goditem_prt01.txt`
  - `te_goditem_alde1.txt`
- Fixed merged line artifact (`close; }`) back into two lines in both TE files.

## Validation notes
- These are DSL parse-path fixes; local `--npc-script-only` mode does not parse `.txt` files.
- Requires user-side full `./map-server --run-once` to confirm parser errors are cleared.
