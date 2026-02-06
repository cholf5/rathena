# Gray batch: RE jobs 3-1 (archbishop, mechanic, ranger)

## Scope
- `/Users/cholf5/dev/rathena/npc/re/jobs/3-1/archbishop.lua`
- `/Users/cholf5/dev/rathena/npc/re/jobs/3-1/mechanic.lua`
- `/Users/cholf5/dev/rathena/npc/re/jobs/3-1/ranger.lua`
- `/Users/cholf5/dev/rathena/npc/re/scripts_jobs.conf`

## What changed
- Re-generated three RE 3-1 job scripts from `.txt` via `script2lua`.
- Switched corresponding entries in `scripts_jobs.conf` from `.txt` to `.lua`.

## Validation
- Conversion reports:
  - `archbishop`: `unsupported_items: 0`
  - `mechanic`: `unsupported_items: 0`
  - `ranger`: `unsupported_items: 0`
- Lua syntax checks passed (`loadfile(...)`) for all three files.
- Build check passed with `make -j4`.

## Notes
- This batch follows the same low-risk promotion criteria: only zero-unsupported conversions were switched.
