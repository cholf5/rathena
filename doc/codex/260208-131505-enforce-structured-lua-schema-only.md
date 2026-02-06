# Enforce Structured Lua Schema Only

## Why

- New Lua gameplay scripts must be readable and idiomatic.
- Legacy tuple keys (`w1/w2/w3/w4`) are removed from the Lua authoring path.
- Old DSL remains available for legacy content.

## Code changes

- Updated `/Users/cholf5/dev/rathena/src/map/npc_lua.cpp` to remove all `w1/w2/w3/w4` parsing and fallback branches.
- `scripts[]`/`functions[]` runtime binding collection now reads `name` only.
- Structured fields are now mandatory for typed loaders:
  - `shops[]`: `items` (instead of `w4`)
  - `duplicates[]`: `view` (instead of `w4`)
  - `scripts[]`: structured placement/name/sprite/trigger fields
  - `functions[]`: `name` + body/runtime handlers
- Error messages updated to reference structured required fields.

## Documentation changes

- Rewrote `/Users/cholf5/dev/rathena/doc/script_lua.md` to structured schema examples only.
- Rewrote `/Users/cholf5/dev/rathena/doc/script_lua_schema.md` to define only structured object keys.
- Explicitly documented that `w1/w2/w3/w4` must not be used in new Lua scripts.

## Compatibility note

- This is an intentional breaking change for Lua schema.
- Existing DSL `.txt` scripts are unaffected.
