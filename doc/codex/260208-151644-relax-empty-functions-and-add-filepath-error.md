# Relax Empty `functions[]` Entries + Better Error Location

## Problem

After enforcing structured Lua schema, server startup emitted repeated errors:

- `functions[N] missing required fields (name/body|run/labels)`

Some legacy-converted Lua files contain empty placeholder entries (`{}`) inside `functions[]`.

## Change

Updated `/Users/cholf5/dev/rathena/src/map/npc_lua.cpp`:

- `append_typed_functions(...)` now accepts `filepath` for diagnostics.
- Empty placeholder entries in `functions[]` are skipped silently.
- Only partially-invalid entries now error, with file path + index:
  - `npc_parseluafile: '<file>' functions[N] missing required fields ...`

## Validation

- `make -C src/map map-server -j4` passed.
- `./map-server --npc-script-only --run-once` exits `0`.
