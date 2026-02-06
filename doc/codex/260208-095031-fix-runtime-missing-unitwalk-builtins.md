# Fix runtime error: missing unitstopwalk/unitwalk builtins

## Summary
Resolved current `errors.txt` failure in `quests_17_2.lua` caused by missing movement builtins.

## Change
- File: `src/map/lua_engine.cpp`
- Added noop builtin registrations:
  - `unitstopwalk`
  - `unitwalk`

## Validation
- Rebuilt: `make -C src/map map-server -j4`
- Script-only check: `./map-server --npc-script-only --run-once`
  - `checked=819 failed=0`
- DSL safety: no `.txt` script changes.
