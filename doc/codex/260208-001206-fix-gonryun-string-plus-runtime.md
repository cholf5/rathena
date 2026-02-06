# 260208-001206 fix gonryun string plus runtime

## Issue
Runtime error when running map-server:
- npc/quests/quests_gonryun.lua:842: attempt to add a 'string' with a 'number'

Root cause: converter left DSL-style string concatenation using + in Lua code.

## Changes
Updated /Users/cholf5/dev/rathena/npc/quests/quests_gonryun.lua:
- Replaced string concatenations from + to .. in event dispatch and mes() text assembly.
- Fixed lines around:
  - trace1/trace2/trace3 donpcevent(...) dynamic name construction
  - player name display via strcharinfo(0)
  - sick message assembly block

## Validation
- loadfile('npc/quests/quests_gonryun.lua') => OK
- No remaining obvious string + concatenation patterns in this file (" + / + " search).
