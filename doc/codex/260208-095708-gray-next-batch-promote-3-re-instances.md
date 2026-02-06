# Gray next batch: promote 3 RE instance scripts

## Summary
Continued gray migration with syntax-fix-first workflow and promoted 3 RE instance entries from DSL to Lua.

## Files promoted (.txt -> .lua)
- `npc/re/instances/CentralLaboratory`
- `npc/re/instances/IsleOfBios`
- `npc/re/instances/VillaofDeception`

## Lua fixes applied in this batch
- `npc/re/instances/CentralLaboratory.lua`
  - fixed chained assignments in `OnInstanceInit`.
- `npc/re/instances/IsleOfBios.lua`
  - fixed chained string assignments in timer event.
- `npc/re/instances/VillaofDeception.lua`
  - fixed malformed `do({ ...` converted statement.

## Deferred (next batch)
- `npc/re/instances/DevilTower.lua` still has chained-assignment syntax issues.
- `npc/re/instances/OldGlastHeim.lua` still has chained-assignment syntax issues.

## Validation
- `./map-server --npc-script-only --run-once` => `checked=822 failed=0`
- Remaining RE `.txt` entries that have `.lua` pairs: `12`
- DSL safety: no `npc/**/*.txt` changes.
