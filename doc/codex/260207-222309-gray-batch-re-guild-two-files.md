# Gray batch: RE guild two-file conversion pass

## Scope
- `npc/re/guild/invest_main.lua`
- `npc/re/guild/mission_main.lua`

## What was done
- Re-generated both Lua files from their DSL sources using `script2lua`:
  - `/Users/cholf5/dev/rathena/npc/re/guild/invest_main.txt` -> `/Users/cholf5/dev/rathena/npc/re/guild/invest_main.lua`
  - `/Users/cholf5/dev/rathena/npc/re/guild/mission_main.txt` -> `/Users/cholf5/dev/rathena/npc/re/guild/mission_main.lua`
- Replaced repo versions with generated outputs.

## Validation
- Conversion reports (both): `unsupported_items: 0`.
- Lua syntax checks passed:
  - `loadfile("/Users/cholf5/dev/rathena/npc/re/guild/invest_main.lua")`
  - `loadfile("/Users/cholf5/dev/rathena/npc/re/guild/mission_main.lua")`
- Full build check passed via `make -j4`.

## Notes
- `guildrelay` remains a separate blocker under `npc/quests/` and is still loaded via `.txt` in `scripts_athena.conf` as before.
