# Fix `npc/other/Global_Functions.lua` for Structured `functions[]`

## Problem

`npc/other/Global_Functions.lua` used legacy tuple keys (`w1/w2/w3`) in `functions[]` entries.
After enforcing structured Lua schema, loader rejected entries with:

- `functions[N] missing required fields (name/body|run/labels)`

## Change

Updated `/Users/cholf5/dev/rathena/npc/other/Global_Functions.lua`:

- Removed `w1 = "function"`, `w2 = "script"`, `w3 = "..."` from function entries.
- Replaced with structured `name = "..."`.
- Kept all `run`/`labels` bodies unchanged.

## Validation

- `./map-server --npc-script-only --run-once` exits `0`.
