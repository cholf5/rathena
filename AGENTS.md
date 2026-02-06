# AGENTS.md

## Project
- Repository: `rAthena`
- Primary focus (current): Lua gameplay scripting migration/runtime for NPC pipeline.

## Current Lua Architecture
- Formal Lua gameplay scripts live under: `lua/npc/...`
- Lua demo scripts live under: `lua/demo/...`
- Formal Lua load list: `lua/scripts_lua.conf`
- Formal imports are enabled in:
  - `npc/re/scripts_main.conf`
  - `npc/pre-re/scripts_main.conf`

## Engine Conventions
- Lua runtime entrypoints are implemented in `src/map/lua_engine.cpp` and related files.
- Chunk runtime environment is bound by engine at load time.
- Business Lua scripts should not need `local _ENV = ctx:env()` for normal runtime API calls.

## Script Authoring Rules
- Use structured schema only (`warps`, `monsters`, `mapflags`, `shops`, `duplicates`, `scripts`, `functions`).
- Do not use legacy tuple keys (`w1/w2/w3/w4`).
- Mirror source hierarchy when porting DSL files (for traceability): `lua/npc/<same-path>.lua`.
- Avoid transitional naming like `_gold` for production scripts.

## Validation Workflow
1. Syntax check changed Lua files:
   - `luac -p <changed-lua-files>`
2. Runtime load check:
   - `./map-server --run-once`
3. Fix by stack trace file/line and rerun.

## Documentation Sources
- `doc/script_lua.md`
- `doc/script_lua_schema.md`
- `lua/annotations.lua`

## Change Logging
- For meaningful changes, add a note in:
  - `doc/codex/yymmdd-hhmms-<what-you-did>.md`

## Skill
- Project-level skill for handoff/continuation:
  - `.codex/skills/rathena-lua-dev`
