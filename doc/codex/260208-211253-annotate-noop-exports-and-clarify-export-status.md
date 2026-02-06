# Annotate no-op exports and clarify Lua export status

## What user asked

- Expand annotations to long manual style.
- Clarify whether compatibility functions are exported.
- If not exported, export them.

## Findings

In `src/map/lua_engine.cpp`, compatibility functions under `noop_builtins[]` are already exported to Lua via `lua_register_builtin(...)` and currently mapped to `lua_builtin_noop`.

## Changes

Updated `/Users/cholf5/dev/rathena/npc/lua/annotations.lua`:

- Expanded core APIs with richer Behavior/Example sections.
- Reworked compatibility section to per-function detailed entries.
- Marked compatibility entries explicitly as `[No-op export]`.
- Added clear section note that these names are exported but currently stubbed.

## Validation

- `luac -p /Users/cholf5/dev/rathena/npc/lua/annotations.lua` passed.
