# Gray batch: RE jobs 1-1 (acolyte, mage, thief)

## Scope
- `/Users/cholf5/dev/rathena/npc/re/jobs/1-1/acolyte.lua`
- `/Users/cholf5/dev/rathena/npc/re/jobs/1-1/mage.lua`
- `/Users/cholf5/dev/rathena/npc/re/jobs/1-1/thief.lua`
- `/Users/cholf5/dev/rathena/npc/re/scripts_jobs.conf`

## What changed
- Re-generated the three Lua files from DSL sources using `script2lua`:
  - `acolyte.txt -> acolyte.lua`
  - `mage.txt -> mage.lua`
  - `thief.txt -> thief.lua`
- Updated RE jobs config entries to load the regenerated `.lua` files.

## Validation
- Conversion reports for all three files: `unsupported_items: 0`.
- Lua syntax checks passed for all three generated files using `loadfile(...)`.
- Build check passed with `make -j4`.

## Notes
- This continues the incremental RE gray migration pattern.
- Runtime validation remains user-run (`./map-server --run-once`) per workflow.
