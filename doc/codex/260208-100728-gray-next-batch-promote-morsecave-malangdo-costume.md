# Gray next batch: promote MorseCave + malangdo_costume

## Summary
Promoted 2 additional RE scripts from DSL `.txt` to Lua `.lua` after targeted syntax fixes.

## Promoted
- `npc/re/instances/MorseCave`
- `npc/re/merchants/malangdo_costume`

## Syntax fixes in this batch
- `npc/re/instances/MorseCave.lua`
  - fixed malformed color literal: `00EBFF` -> `0x00EBFF`
- `npc/re/merchants/malangdo_costume.lua`
  - fixed malformed converted statement: `do({ _ENV[".@menu$"] = "")` -> plain assignment.
- `npc/re/instances/FacewormsNest.lua`
  - partial cleanup of malformed `do({ ...` sequence (not yet fully luac-clean; deferred).

## Validation
- `luac -p` passed for `MorseCave.lua` and `malangdo_costume.lua`.
- `./map-server --npc-script-only --run-once` => `checked=827 failed=0`.
- Remaining RE `.txt` entries with `.lua` counterparts: `7`.
- DSL safety check: no `npc/**/*.txt` changes.
