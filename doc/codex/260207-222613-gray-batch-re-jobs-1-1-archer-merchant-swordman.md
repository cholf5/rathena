# Gray batch: RE jobs 1-1 (archer, merchant, swordman)

## Scope
- `/Users/cholf5/dev/rathena/npc/re/jobs/1-1/archer.lua`
- `/Users/cholf5/dev/rathena/npc/re/jobs/1-1/merchant.lua`
- `/Users/cholf5/dev/rathena/npc/re/jobs/1-1/swordman.lua`
- `/Users/cholf5/dev/rathena/npc/re/scripts_jobs.conf`

## What changed
- Re-generated the three Lua files from their DSL sources using `script2lua`:
  - `archer.txt -> archer.lua`
  - `merchant.txt -> merchant.lua`
  - `swordman.txt -> swordman.lua`
- Updated RE jobs script entry config to load these three `.lua` files instead of `.txt`.

## Validation
- Conversion reports for all three files: `unsupported_items: 0`.
- Lua syntax checks passed for all three regenerated files via `loadfile(...)`.
- Build check passed with `make -j4`.

## Notes
- This is a small, isolated RE batch and is intended to be validated by user-run `map-server --run-once` in the normal workflow.
