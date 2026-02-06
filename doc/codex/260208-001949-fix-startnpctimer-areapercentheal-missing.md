# 260208-001949 fix startnpctimer areapercentheal missing

## Issue
Latest errors.txt reported missing Lua builtins during runtime:
- global startnpctimer
- global areapercentheal

## Change
Updated /Users/cholf5/dev/rathena/src/map/lua_engine.cpp:
- added startnpctimer to noop_builtins[]
- added areapercentheal to noop_builtins[]

## Validation
- Build passed: make -j4

## Next
- Re-run map-server and refresh errors.txt to continue compatibility cleanup.
