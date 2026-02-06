# Gray Continue: Promote 3 more Lua files

## Promoted
- /Users/cholf5/dev/rathena/npc/re/merchants/enchan_rockridge.lua
- /Users/cholf5/dev/rathena/npc/re/guild3/agit_main_te.lua
- /Users/cholf5/dev/rathena/npc/re/instances/SealedOs.lua

## Fixes applied
- enchan_rockridge.lua
  - fixed invalid chained assignments around card reset.
- agit_main_te.lua
  - fixed malformed converted lines around pseudo function/label artifacts.
- SealedOs.lua
  - fixed invalid chained assignment in OnInstanceInit state init.

## Config updates
- /Users/cholf5/dev/rathena/npc/re/scripts_athena.conf
- /Users/cholf5/dev/rathena/npc/re/scripts_guild.conf

## Validation
- luac checks passed for promoted files.
- ./map-server --npc-script-only --run-once:
  - checked=793 failed=0

## Remaining snapshot
- txt entries with existing lua siblings: 41
- among them, luac failures: 41
