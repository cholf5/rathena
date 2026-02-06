# Fix script_lua.md: explicit NPC support wording

## Issue
The top-level definition section wording made it look like Lua might not support NPC definitions.

## Fix
Updated `doc/script_lua.md` to explicitly state:
- Lua does support NPC definitions.
- NPCs are defined via top-level `scripts[]` entries.
- Added mapping note: DSL `script` line -> Lua `scripts[]` entry.

## Impact
Clarifies authoring model and removes false impression that NPC is unsupported in Lua.
