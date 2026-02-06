# Fix Lua builtins: getcastledata/getstrlen/setcastledata

## Why
`errors.txt` showed high-frequency Lua runtime failures in TE guild scripts:
- `attempt to call a number value (global 'getcastledata')`
- `attempt to call a number value (global 'getstrlen')`

These happened because the Lua engine lacked those DSL-compatible builtins, so unresolved names fell back to numeric defaults in the Lua env.

## Changes
- Updated `/Users/cholf5/dev/rathena/src/map/lua_engine.cpp`:
  - Added `lua_builtin_getstrlen` (returns string length).
  - Added `lua_builtin_getcastledata` (mapped to castle runtime data via `castle_db.mapname2gc`).
  - Added `lua_builtin_setcastledata` (updates castle data via `guild_castledatasave`).
  - Registered the three builtins in `lua_register_builtins`.
  - Included `guild.hpp` and `<cstring>`.

## Validation
- `make -j4` passed.
- `./map-server --npc-script-only --run-once` passed:
  - `checked=793 failed=0`

## Notes
- Local full `./map-server --run-once` in this session cannot verify DB runtime path due local SQL connection limits; user-side full run output should be used for next regression pass.
