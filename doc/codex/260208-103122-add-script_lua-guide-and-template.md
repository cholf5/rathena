# Add Lua script guide doc/script_lua.txt

## Summary
Created a dedicated Lua scripting guide as requested, without touching `doc/script_commands.txt`.

## Added
- `doc/script_lua.txt`

## Content highlights
- Lua-first scripting principles.
- Scoped variable API reference:
  - `get_var/set_var`
  - scoped getter/setter functions
  - scoped alias tables (`character`, `account`, `map_server`, etc.)
- Legacy-to-new mapping guidance.
- Minimal recommended Lua NPC template.
- Array variable example.
- Migration policy for old/new scripts.

## Why
- Keep legacy DSL reference doc stable.
- Provide a clean, dedicated authoring target for new Lua scripts.
