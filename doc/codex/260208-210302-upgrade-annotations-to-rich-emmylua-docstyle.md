# Upgrade `npc/lua/annotations.lua` to Rich EmmyLua Doc Style

## Why

Initial version was signature-centric and too terse for editor guidance.

## What changed

Rewrote `/Users/cholf5/dev/rathena/npc/lua/annotations.lua` with doc-style annotations:

- Added high-detail per-function docs (purpose, behavior notes, params, returns).
- Grouped API by domain:
  - scoped variable APIs
  - legacy variable helpers
  - dialog/interaction
  - flow/call utilities
  - utility/status/map helpers
  - compatibility no-op exports
- Kept `LuaNpcContext`, `LuaNpcEnv`, `LuaScopeName`, and scope table typing.
- Documented special case: runtime exports a builtin named `end`, but this cannot be declared as a Lua identifier.

## Validation

- `luac -p /Users/cholf5/dev/rathena/npc/lua/annotations.lua` passed.
