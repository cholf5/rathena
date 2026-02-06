# 260208-214625-annotations-remove-vararg-signatures

## What changed
Updated `/Users/cholf5/dev/rathena/npc/lua/annotations.lua` to replace all `function xxx(...) end` signatures with explicit parameters.

## Scope
- Replaced vararg signatures in dialog helpers (`mes/select/menu`), utility (`checkweight`), and all compatibility exports.
- Added concrete `@param` and return types where needed.

## Validation
- Confirmed no remaining `function ...(...)` signatures in the file.
- `luac -p npc/lua/annotations.lua` passed.
