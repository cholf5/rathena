# Lua runtime batch 3: mapwarp and powernpc fixes

## Changes
- Updated `/Users/cholf5/dev/rathena/src/map/lua_engine.cpp`:
  - Added builtin alias `Initnpctimer` -> `initnpctimer` implementation.
  - Added noop builtin `mapwarp` to prevent startup runtime failures in battleground/arena scripts.
- Fixed conversion output issues in `/Users/cholf5/dev/rathena/npc/other/arena/arena_lvl80.lua`:
  - Replaced invalid string arithmetic with Lua concat:
    - `"Individual; Level 80 to " + MAX_LEVEL`
    - to
    - `"Individual; Level 80 to " .. MAX_LEVEL`
- Fixed conversion output issues in `/Users/cholf5/dev/rathena/npc/other/powernpc.lua`:
  - Replaced invalid concat using `+` with `..` for menu text assembly.
  - Corrected `setarray` first argument from `_ENV[...]` indexed value to string variable target syntax:
    - e.g. `setarray(".gnpMobsId[0]", ...)`, `setarray(".gnpMobsName$[0]", ...)`.
  - This ensures `.gnpMobsId`/`.gnpMobsName$` are created as array variables instead of indexing numeric defaults.

## Verification
- Rebuilt successfully with `make -j4`.
- `map-server` was not executed locally; validation depends on refreshed `errors.txt`.
