# Gray Batch: RE Jobs 3-2 (7 files)

## Scope
Converted and switched the following RE job scripts from `.txt` to `.lua`:

- `npc/re/jobs/3-2/genetic.txt` -> `npc/re/jobs/3-2/genetic.lua`
- `npc/re/jobs/3-2/minstrel.txt` -> `npc/re/jobs/3-2/minstrel.lua`
- `npc/re/jobs/3-2/royal_guard.txt` -> `npc/re/jobs/3-2/royal_guard.lua`
- `npc/re/jobs/3-2/shadow_chaser.txt` -> `npc/re/jobs/3-2/shadow_chaser.lua`
- `npc/re/jobs/3-2/sorcerer.txt` -> `npc/re/jobs/3-2/sorcerer.lua`
- `npc/re/jobs/3-2/sura.txt` -> `npc/re/jobs/3-2/sura.lua`
- `npc/re/jobs/3-2/wanderer.txt` -> `npc/re/jobs/3-2/wanderer.lua`

Updated config entries in:
- `npc/re/scripts_jobs.conf`

## Conversion Validation
Batch conversion report:
- `.codex_tmp/re_jobs_batch6/report.md`

Result:
- `unsupported_items: 0`
- all 7 files converted successfully

Lua syntax validation:
- `lua -e 'assert(loadfile(...))'` passed for all generated files.

Build validation:
- `make -j4` passed.

## Notes
- `guillotine_cross` remains on `.txt` in this batch and is not changed here.
- As requested, `./map-server` was not executed by Codex in this step.
