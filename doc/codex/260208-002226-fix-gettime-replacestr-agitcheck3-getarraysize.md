# 260208-002226 fix gettime replacestr agitcheck3 getarraysize

## Issue
errors.txt reported missing Lua globals causing runtime failures:
- gettime
- replacestr
- agitcheck3
- getarraysize

## Changes
Updated /Users/cholf5/dev/rathena/src/map/lua_engine.cpp:
- Added lua builtin implementations and registration for:
  - getarraysize
  - gettime
  - replacestr
  - agitcheck3

Notes:
- getarraysize now computes array size from numeric keys (0-based expected by converted scripts).
- gettime now returns local time fields for common mode ids (year/month/day/dayofweek/dayofyear/hour/minute/second).
- replacestr now performs full-string replacement.
- agitcheck3 currently returns 0 (safe default for transition).

## Validation
- Build passed: make -j4
