# Gray batch: morroc_raceway cleanup and guildrelay blocker tracking

## Changes
- Rebuilt `script2lua` from current `/Users/cholf5/dev/rathena/src/tool/script2lua.cpp` updates.
- Re-generated `/Users/cholf5/dev/rathena/npc/custom/etc/morroc_raceway.lua` from DSL source and replaced the previous broken Lua file.
- Updated `/Users/cholf5/dev/rathena/npc/scripts_custom.conf` comment entry:
  - `//npc: npc/custom/etc/morroc_raceway.txt`
  - -> `//npc: npc/custom/etc/morroc_raceway.lua`

## Verification
- `make -j4` succeeded.
- Lua syntax check:
  - `/Users/cholf5/dev/rathena/npc/custom/etc/morroc_raceway.lua` => OK
  - `/Users/cholf5/dev/rathena/npc/quests/guildrelay.lua` => still fails syntax (`unexpected symbol near ','` around line 1753)

## Current blocker
- `guildrelay` remains the last `.txt` active entry in `/Users/cholf5/dev/rathena/npc/scripts_athena.conf`.
- Fresh `script2lua` conversion of `guildrelay.txt` still reports:
  - `unsupported: line=2420 reason=unclosed block automatically terminated`
- This indicates a specific control-flow conversion bug in `script2lua` for that quest block; must be fixed before safely switching `guildrelay` back to `.lua`.
