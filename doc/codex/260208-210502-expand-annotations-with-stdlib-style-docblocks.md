# Expand `npc/lua/annotations.lua` with stdlib-style docblocks

## Goal

Make EmmyLua annotations more explanatory, closer to std library documentation style.

## Changes

Updated `/Users/cholf5/dev/rathena/npc/lua/annotations.lua`:

- Expanded key API docs with richer sections:
  - behavior details
  - edge notes
  - usage examples
- Focused expansion on high-frequency runtime APIs:
  - `get_var` / `set_var`
  - dialog flow (`mes`, `next`, `close`, `select`, `input`)
  - control flow (`sleep`, `callsub`, `callfunc`, `getarg`)
  - event trigger (`donpcevent`)
  - compatibility helper docs (`set`, `setd`, `getd`)
- Kept EmmyLua typing intact and syntax-valid.

## Validation

- `luac -p /Users/cholf5/dev/rathena/npc/lua/annotations.lua` passed.
