# 260208-001348 fix setnpctimer missing builtin

## Issue
errors.txt showed repeated Lua runtime failures in npc/quests/quests_gonryun.lua:
- attempt to call a number value (global 'setnpctimer')

## Change
Updated /Users/cholf5/dev/rathena/src/map/lua_engine.cpp:
- added setnpctimer into noop_builtins[].

## Validation
- Build passed: make -j4.

## Next
- Re-run map-server and check refreshed errors.txt for the next missing builtin/runtime incompatibility.
