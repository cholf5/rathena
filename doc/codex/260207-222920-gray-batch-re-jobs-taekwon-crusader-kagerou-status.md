# Gray batch: RE jobs (taekwon, crusader, kagerou_oboro)

## Scope
- `/Users/cholf5/dev/rathena/npc/re/jobs/1-1e/taekwon.lua`
- `/Users/cholf5/dev/rathena/npc/re/jobs/2-2/crusader.lua`
- `/Users/cholf5/dev/rathena/npc/re/jobs/2e/kagerou_oboro.lua`
- `/Users/cholf5/dev/rathena/npc/re/scripts_jobs.conf`

## What changed
- Re-generated all three Lua files from their `.txt` sources using `script2lua`.
- Updated RE jobs config to switch:
  - `npc/re/jobs/1-1e/taekwon.txt` -> `npc/re/jobs/1-1e/taekwon.lua`
  - `npc/re/jobs/2-2/crusader.txt` -> `npc/re/jobs/2-2/crusader.lua`
- Kept `npc/re/jobs/2e/kagerou_oboro.txt` active (did not switch config entry yet).

## Validation
- Lua syntax checks passed for all three regenerated files (`loadfile(...)`).
- Conversion report status:
  - `taekwon`: `unsupported_items: 0`
  - `crusader`: `unsupported_items: 0`
  - `kagerou_oboro`: `unsupported_items: 2`
    - line 758: `unclosed block automatically terminated`
    - line 971: `unexpected closing brace`
- Build check passed with `make -j4`.

## Decision
- `taekwon` and `crusader` were promoted to `.lua` entry immediately (clean conversion).
- `kagerou_oboro` remains on `.txt` entry temporarily due to structural unsupported markers despite syntax validity.
