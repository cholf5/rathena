# Switch Config Entries Back to DSL for Existing Content

## Decision

Adopted option 1:

- Keep existing gameplay content on DSL (`.txt`) in config.
- Keep Lua focused on new scripting runtime and limited test/demo scripts.

## What was changed

Batch updated config files under:

- `/Users/cholf5/dev/rathena/npc/*.conf`
- `/Users/cholf5/dev/rathena/npc/re/*.conf`
- `/Users/cholf5/dev/rathena/npc/pre-re/*.conf`

Rule:

- For each `npc: <path>.lua`, if `<path>.txt` exists, switch the entry to `.txt`.
- If no `.txt` exists, keep `.lua` unchanged.

Result:

- `changed_files=14`
- `changed_lines=833`

After migration, remaining `.lua` entries in these confs are only:

- `npc/scripts_test.conf`
  - `npc/test/lua_scope_api_demo.lua`
  - `npc/test/lua_scope_api_array_demo.lua`

## Validation

- `./map-server --npc-script-only --run-once` exits `0`.
