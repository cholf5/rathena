# Fix Airports Lua Schema Errors (Hugel / Izlude / Lighthalzen)

## Problem

Reported loader errors:

- `npc/airports/hugel.lua` `scripts[1]` missing required fields
- `npc/airports/izlude.lua` `scripts[1]` missing required fields
- `npc/airports/lighthalzen.lua` `scripts[1/2]` missing required fields
- `npc/airports/lighthalzen.lua` `duplicates[1/2]` missing required fields

## Change

Converted tuple-style headers (`w1/w2/w3/w4`) to structured fields in:

- `/Users/cholf5/dev/rathena/npc/airports/hugel.lua`
  - `scripts[]` -> `map/x/y/dir/type/name/sprite/trigger_x/trigger_y`
- `/Users/cholf5/dev/rathena/npc/airports/izlude.lua`
  - `scripts[]` -> structured fields (`map = "-"` supported)
- `/Users/cholf5/dev/rathena/npc/airports/lighthalzen.lua`
  - `scripts[]` -> structured fields
  - `duplicates[]` -> `map/x/y/dir/source/name/view`

Script bodies were kept unchanged.

## Validation

- `./map-server --npc-script-only --run-once` exits `0`.
