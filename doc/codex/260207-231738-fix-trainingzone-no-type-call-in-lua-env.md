# Fix: TrainingZone Lua init without `type()` in script env

## Context
`errors.txt` reported:
- `attempt to call a number value (global 'type')`

At:
- `npc/re/other/TrainingZone123.lua` line around `OnInit`

## Root Cause
In the Lua NPC env, unresolved globals can resolve to numeric defaults (DSL-compatible behavior). Calling `type(...)` in this env is therefore unsafe.

## Change
Updated:
- `npc/re/other/TrainingZone123.lua`

In `OnInit`, removed `type(...)` checks and switched to unconditional array initialization:
- `setarray(".sens[0]", 0, 0, 0, 0)`
- `setarray(".is_moving[0]", 0, 0, 0, 0)`

Then continue with existing `.@num` setup and timer init.

## Validation
- `make -j4` passed.
- `./map-server` not run by Codex (per workflow).
