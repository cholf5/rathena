# 260208-211924-enable-core-lua-builtin-effects

## What changed
- Replaced Lua compatibility no-op stubs with real runtime behavior for these builtins:
  - `getitem`
  - `delitem`
  - `warp`
  - `announce`
  - `mapannounce`
- Kept all other compatibility exports as no-op for now.

## Engine changes
- Updated `/Users/cholf5/dev/rathena/src/map/lua_engine.cpp`:
  - Added concrete implementations:
    - `lua_builtin_getitem`
    - `lua_builtin_delitem`
    - `lua_builtin_warp`
    - `lua_builtin_announce`
    - `lua_builtin_mapannounce`
  - Added helper functions:
    - `lua_get_item_by_arg`
    - `lua_get_target_sd`
    - `lua_builtin_announce_sub`
  - Registered above builtins in `lua_register_builtins`.
  - Removed these names from `noop_builtins[]`: `mapannounce`, `warp`, `getitem`, `delitem`, `announce`.
  - Added missing includes required by new runtime paths: `intif.hpp`, `itemdb.hpp`, `pet.hpp`, `log.hpp`.

## Documentation changes
- Updated `/Users/cholf5/dev/rathena/npc/lua/annotations.lua`:
  - Moved `getitem/delitem/warp/announce/mapannounce` out of "Compatibility no-op exports".
  - Added full EmmyLua docs for parameter meanings and behavior notes.

## Validation
- Build passed:
  - `make -C src/map -j4 map-server`
- Annotation syntax check passed:
  - `luac -p npc/lua/annotations.lua`

## Notes
- `delitem` implementation currently performs inventory deletion by item id/name and prefers non-equipped stacks first; it does not yet implement the full `delitem2/3/4` exact-match attribute matrix.
