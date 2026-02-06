# Fix: RE other runtime issues after Lua switch

## Context
After switching `npc/re/other` to Lua, `errors.txt` showed:

- `attempt to call a number value (global 'sc_start2')` in `npc/re/other/clans.lua`
- `attempt to index a number value (global '.sens')` in `npc/re/other/TrainingZone123.lua`

Also, previous run showed mixed-mode DSL parse failures because still-DSL scripts depend on `Global_Functions`.

## Changes
1. Keep DSL global helper for mixed mode:
- `npc/re/scripts_athena.conf`
  - `npc/re/other/Global_Functions.txt` (kept as DSL)

2. Add missing Lua builtin noop:
- `src/map/lua_engine.cpp`
  - added `sc_start2`

3. Initialize script arrays in converted TrainingZone script:
- `npc/re/other/TrainingZone123.lua`
  - in `OnInit`, ensure `.sens` and `.is_moving` are tables via `setarray` before indexed access

## Validation
- `make -j4` passed
- `./map-server` not executed by Codex (per workflow)
