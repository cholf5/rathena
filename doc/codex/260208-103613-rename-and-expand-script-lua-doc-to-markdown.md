# Rename and expand Lua manual to Markdown

## Summary
Replaced text draft with a full Markdown Lua scripting manual aligned with `script_commands.txt` positioning.

## Changes
- Added: `doc/script_lua.md`
- Removed: `doc/script_lua.txt`

## Coverage added
- Script loading/unloading structure.
- Full top-level Lua object definitions:
  - warp / monster / mapflag / shop / duplicate / script / function
- NPC logic structure (`main/events/labels`, `callsub`, `callfunc`, events).
- Scoped variable API (generic + scoped + alias tables).
- Legacy compatibility policy.
- End-to-end minimal template and validation flow.

## Intent
Make `doc/script_lua.md` the primary Lua authoring manual for future feature development.
