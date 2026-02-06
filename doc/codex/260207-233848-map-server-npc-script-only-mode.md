# 260207-233848 map-server npc-script-only mode

## Goal
Add a `map-server` runtime option to load/check NPC scripts without requiring DB connections.

## Changes
Updated `/Users/cholf5/dev/rathena/src/map/map.cpp`:

1. Added new map-only CLI option:
- `--npc-script-only`

2. Added parser for map-only options:
- new `map_get_options(argc, argv)`
- strips `--npc-script-only` from argv so shared `cli_get_options()` does not reject it as unknown.

3. Added help text in `display_helpscreen()`:
- `--npc-script-only` description.

4. Added no-DB NPC script load path in `MapServer::initialize()`:
- after `map_reloadnpc(false)`, if `map_npc_script_only` is set:
  - reads battle/script/inter/log config
  - initializes map index + map cache loading
  - initializes script-related subsystems needed for NPC load (`path`, `atcommand`, `battle`, `script`, `itemdb`, `skill`, `mob`, `npc`)
  - runs `npc_event_do_oninit()`
  - exits process with success (`std::exit(EXIT_SUCCESS)`) before SQL init

## Validation
- Build passed: `make -j4`.
- Runtime smoke check command accepted and entered new mode:
  - `./map-server --npc-script-only --run-once`
  - observed log includes `NPC script-only mode enabled...` and map loading path.

## Notes
- This mode intentionally bypasses SQL initialization (`map_sql_init()` / `log_sql_init()`).
- It is intended for NPC script load/regression checks in constrained environments.
