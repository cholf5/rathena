# Lua runtime builtin stubs batch 2

## Changes
- Expanded Lua builtin compatibility in `/Users/cholf5/dev/rathena/src/map/lua_engine.cpp` to reduce startup runtime errors from missing DSL commands.
- Added no-op stubs for:
  - `enablewaitingroomevent`
  - `disablewaitingroomevent`
  - `monster`
  - `killmonster`
  - `bg_unbook`
  - `removemapflag`
  - `areawarp`
  - `hideoffnpc`
- Added `getvariableofnpc` builtin with safe default semantics:
  - returns `""` for variable names ending with `$`
  - returns `0` otherwise
- This avoids `attempt to call a number value` and prevents immediate `nil` propagation for common string variable callsites.

## Verification
- Rebuilt successfully with `make -j4`.
- `map-server` was not executed locally (per workflow); user should provide refreshed `errors.txt`.
