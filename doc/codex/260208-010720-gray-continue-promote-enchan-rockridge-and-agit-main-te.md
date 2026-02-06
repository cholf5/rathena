# Gray Continue: Promote enchan_rockridge and agit_main_te

## Changes
- Fixed and promoted:
  - /Users/cholf5/dev/rathena/npc/re/merchants/enchan_rockridge.lua
    - repaired invalid chained assignments in reset branch
    - switched config from .txt to .lua in /Users/cholf5/dev/rathena/npc/re/scripts_athena.conf
  - /Users/cholf5/dev/rathena/npc/re/guild3/agit_main_te.lua
    - fixed malformed converter artifacts around function/label lines
    - switched config from .txt to .lua in /Users/cholf5/dev/rathena/npc/re/scripts_guild.conf

## Validation
- luac syntax checks passed for both files.
- ./map-server --npc-script-only --run-once passed with checked=792 failed=0.

## Remaining
- txt entries with existing lua siblings (not yet switched): 42
- among them, luac parse failures: 41
