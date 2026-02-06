# Expand doc/script_lua.md into reference manual

## Summary
Expanded `doc/script_lua.md` from a short guide into a broader Lua scripting reference manual aligned with the role of `script_commands.txt` (for Lua runtime scope).

## What changed
- Added explicit scope statement: this manual is about rAthena Lua scripting API, not Lua language syntax.
- Added loading/unloading structure and config chain.
- Added top-level object definitions with examples:
  - warp, monster, mapflag, shop, duplicate, script, function.
- Added runtime execution model (`ctx:env()`, yield/resume context).
- Added full scoped variable API section:
  - generic, scoped functions, alias tables, behavior notes.
- Added command reference sections grouped by category, including:
  - implemented commands,
  - compatibility stubs/noops.
- Added best-practice rules, minimal template, validation workflow, migration policy.

## Notes
- Kept `doc/script_commands.txt` untouched.
- This update focuses on Lua runtime semantics and engine API surface.
