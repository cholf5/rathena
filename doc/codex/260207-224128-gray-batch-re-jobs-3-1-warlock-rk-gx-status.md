# Gray batch: RE jobs 3-1 (warlock, rune_knight, guillotine_cross status)

## Scope
- `/Users/cholf5/dev/rathena/npc/re/jobs/3-1/warlock.lua`
- `/Users/cholf5/dev/rathena/npc/re/jobs/3-1/rune_knight.lua`
- `/Users/cholf5/dev/rathena/npc/re/jobs/3-1/guillotine_cross.lua`
- `/Users/cholf5/dev/rathena/npc/re/scripts_jobs.conf`

## What changed
- Re-generated all three Lua files from their `.txt` sources.
- Promoted config entries to `.lua` for:
  - `warlock`
  - `rune_knight`
- Kept `guillotine_cross` config entry on `.txt` temporarily.

## Validation
- Lua syntax check passed for all three generated files.
- Conversion report status:
  - `warlock`: `unsupported_items: 0`
  - `rune_knight`: `unsupported_items: 0`
  - `guillotine_cross`: `unsupported_items: 28` (multiple structural conversion issues)
- Build check passed with `make -j4`.

## Decision
- Only zero-unsupported files (`warlock`, `rune_knight`) were switched in config.
- `guillotine_cross` is deferred for dedicated converter hard-case handling before safe promotion.
