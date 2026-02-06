# Gray next batch: promote 3 more RE instances

## Summary
Continued staged conversion and promoted 3 additional RE instance scripts from `.txt` to `.lua` after syntax hardening.

## Promoted
- `npc/re/instances/DevilTower`
- `npc/re/instances/OldGlastHeim`
- `npc/re/instances/TempleOfDemonGod`

## Key syntax fixes
- Resolved multiple chained assignments (`a = b = 0` / `a = b = ""`) into explicit per-variable assignments.
- Fixed malformed converted `if` block in `TempleOfDemonGod.lua` around target-type check.
- Fixed additional repeated reset blocks in `DevilTower.lua`.

## Validation
- `luac -p` passed for all three promoted files.
- `./map-server --npc-script-only --run-once` => `checked=825 failed=0`.
- Remaining RE `.txt` entries with `.lua` counterparts: `9`.
- DSL safety check: no `npc/**/*.txt` changes.
