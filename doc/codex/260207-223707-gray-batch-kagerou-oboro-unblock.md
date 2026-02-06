# Gray batch: kagerou_oboro unblock and RE jobs config switch

## Scope
- `/Users/cholf5/dev/rathena/npc/re/jobs/2e/kagerou_oboro.lua`
- `/Users/cholf5/dev/rathena/npc/re/scripts_jobs.conf`

## What changed
- `kagerou_oboro.lua` was regenerated from `.txt` and then minimally corrected in the `Kuuga Gai#ko` script block to preserve intent around the `L_Kick` label path:
  - Replaced `_ENV["goto"]("L_Kick")` with `callsub("L_Kick"); return`.
  - In the fallback `else` branch (where original DSL jumps to `L_Kick`), added `callsub("L_Kick"); return`.
  - Removed duplicated `do return end` in `L_Kick` label function.
- Updated RE jobs config entry:
  - `npc/re/jobs/2e/kagerou_oboro.txt` -> `npc/re/jobs/2e/kagerou_oboro.lua`

## Validation
- Lua syntax check passed for `/Users/cholf5/dev/rathena/npc/re/jobs/2e/kagerou_oboro.lua`.
- Build check passed with `make -j4`.

## Notes
- This is a targeted unblock for a known converter hard-case region.
- Follow-up item remains: fold this pattern into `script2lua` so future regenerations do not require manual touch-up.
