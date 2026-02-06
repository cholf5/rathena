# Lua runtime builtins and scope fix

## Changes
- Fixed compile issue in `/Users/cholf5/dev/rathena/src/map/lua_engine.cpp` by adding a forward declaration for `lua_current_execution()` before scoped variable keying logic.
- Added scoped key support for local temp vars (`.@*`) using execution context ref to reduce cross-context pollution in Lua variable storage.
- Added Lua builtins:
  - `getd(name)` -> dynamic variable read
  - `setd(name, value)` -> dynamic variable write
  - `setarray(name_or_name[idx], ...)` -> array/table write semantics
- Added noop-compatible stubs for missing commands that were causing call failures:
  - `waitingroom`, `hideonnpc`, `unloadnpc`, `stopnpctimer`
- Fixed `getbattleflag` binding from a numeric field to a callable builtin function returning `0`, avoiding `attempt to call a number value`.
- Improved variable registry to support table values using Lua registry refs and cleanup on overwrite.
- Fixed variable type detection in `lua_store_variable`: now uses exact Lua type (`LUA_TNUMBER` / `LUA_TSTRING` / `LUA_TBOOLEAN` / `LUA_TTABLE`) instead of `lua_isstring`-first logic, preventing numbers from being stored as strings and reducing `attempt to compare string with number`.

## Verification
- Rebuilt successfully with `make -j4` in `/Users/cholf5/dev/rathena`.
- Confirmed no compile error remains in `lua_engine.cpp`.

## Next
- User should run:
  - `./map-server --run-once 2> errors.txt`
- Then continue runtime triage based on remaining errors (expected next hotspot: numeric/string comparison in `npc/quests/okolnir.lua`).
