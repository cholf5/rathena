# 260208-000115 npc-script-only stable no-db

## Problem
`./map-server --npc-script-only --run-once` still failed in practice (fatal signal / hang around map load path).

## Root cause
The first implementation still entered heavy map/NPC runtime initialization paths that are not safe for a DB-less validation run (direct or indirect SQL-dependent branches, plus long init chains).

## Changes
### 1) Added script-only mode query API
- `/Users/cholf5/dev/rathena/src/map/map.hpp`
  - added `bool map_is_npc_script_only(void);`
- `/Users/cholf5/dev/rathena/src/map/map.cpp`
  - implemented `map_is_npc_script_only()`.

### 2) Guarded SQL-dependent NPC init branches
- `/Users/cholf5/dev/rathena/src/map/npc.cpp` (`do_init_npc`)
  - in script-only mode, skip:
    - `npc_market_fromsql()`
    - `barter_db.load()`
    - `npc_market_checkall()`

### 3) Added lightweight Lua-only NPC source validator
- `/Users/cholf5/dev/rathena/src/map/npc.hpp`
  - added `int32 npc_validate_srcfiles_lua_only(void);`
- `/Users/cholf5/dev/rathena/src/map/npc.cpp`
  - implemented `npc_validate_srcfiles_lua_only()`:
    - iterates `npc_src_files`
    - validates only `.lua` entries via `lua_engine_load_file()`
    - skips DSL files intentionally
    - prints checked/failed summary and returns failed count

### 4) Reworked `--npc-script-only` behavior to stable no-DB validator path
- `/Users/cholf5/dev/rathena/src/map/map.cpp`
  - `--npc-script-only` now:
    - reads npc source list from configs (`map_reloadnpc(false)`)
    - initializes Lua engine
    - runs `npc_validate_srcfiles_lua_only()`
    - finalizes Lua engine
    - exits success/failure directly
  - no SQL init, no map load chain, no OnInit execution.

## Validation
- Build: `make -j4` passed.
- Runtime check:
  - command: `./map-server --npc-script-only --run-once`
  - result: exit code `0`
  - stdout summary: `checked=693 failed=0`
  - `errors.txt`: empty

## Current behavior contract
`--npc-script-only` = "Lua NPC file load/compile validation only" (from configured NPC source list), DB-free and deterministic.
