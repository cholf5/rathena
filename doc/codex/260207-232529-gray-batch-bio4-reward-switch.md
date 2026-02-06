# 260207-232529 gray batch bio4 reward switch

## Batch scope
Gray migration batch: switch exactly one RE merchant script from DSL to Lua.

## Changes
- Updated `/Users/cholf5/dev/rathena/npc/re/scripts_athena.conf`:
  - `npc/re/merchants/bio4_reward.txt` -> `npc/re/merchants/bio4_reward.lua`

## Pre-switch validation
- Syntax check passed:
  - `lua -e "loadfile('npc/re/merchants/bio4_reward.lua')"`

## Notes
- Kept other previously deferred merchant `.txt` entries unchanged for rollback safety.
- Runtime validation is delegated to user-run `./map-server --run-once 2> errors.txt` as requested.
