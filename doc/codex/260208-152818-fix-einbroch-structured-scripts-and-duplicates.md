# Fix `npc/airports/einbroch.lua` Structured Schema Errors

## Problem

Loader errors:

- `npc/airports/einbroch.lua` `scripts[3]` missing required fields
- `npc/airports/einbroch.lua` `duplicates[1]` missing required fields

## Change

Updated `/Users/cholf5/dev/rathena/npc/airports/einbroch.lua`:

- `duplicates[]` converted from tuple keys to structured fields:
  - `map/x/y/dir/source/name/view`
- `scripts[]` converted from tuple keys to structured fields:
  - `map/x/y/dir/type/name/sprite` (+ trigger fields when present)
- Runtime bodies (`main/events/labels`) unchanged.

## Validation

- `./map-server --npc-script-only --run-once` exits `0`.
