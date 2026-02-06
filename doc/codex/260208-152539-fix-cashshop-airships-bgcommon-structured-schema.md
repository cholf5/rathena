# Fix Structured Lua Schema Errors in Key Converted Files

## Issues addressed

Reported loader errors:

- `npc/other/CashShop_Functions.lua` `functions[11]` missing `name/body|run/labels`
- `npc/airports/airships.lua` `functions[1]` missing `name/body|run/labels`
- `duplicates[7/8]` missing `source/name/view`
- `scripts[1]` missing `name/body|main/events/labels`
- `npc/battleground/bg_common.lua` `functions[1]` missing `name/body|run/labels`

## Changes

### Runtime diagnostics

Updated `/Users/cholf5/dev/rathena/src/map/npc_lua.cpp`:

- `scripts[]` validation errors now include file path.
- `duplicates[]` validation errors now include file path.

### File conversions to structured schema

Updated these files from tuple-style `w1/w2/w3/w4` headers to structured keys:

- `/Users/cholf5/dev/rathena/npc/other/CashShop_Functions.lua`
  - `functions[]`: `name` retained, `run/labels` unchanged.
- `/Users/cholf5/dev/rathena/npc/airports/airships.lua`
  - `duplicates[]`: `map/x/y/dir/source/name/view`
  - `scripts[]`: `map/x/y/dir/type/name/sprite/trigger_x/trigger_y`
  - `functions[]`: `name`
- `/Users/cholf5/dev/rathena/npc/battleground/bg_common.lua`
  - `warps[]`: `map/x/y/dir/name/xs/ys/to_map/to_x/to_y`
  - `duplicates[]`: `map/x/y/dir/source/name/view`
  - `scripts[]`: `map/x/y/dir/type/name/sprite/trigger_x/trigger_y`
  - `functions[]`: `name`

Script bodies and gameplay logic were not rewritten.

## Validation

- `make -C src/map map-server -j4` passed.
- `./map-server --npc-script-only --run-once` exited `0`.
- Full `--run-once` in this environment still fails DB connect (`ragnarok@127.0.0.1:3306`).
