---
name: rathena-lua-dev
description: Continue rAthena Lua engine and gameplay script development in the current workspace. Use when tasks involve migrating or authoring NPC gameplay in Lua, updating lua_engine/npc_lua integration, maintaining lua/scripts_lua.conf load chain, fixing Lua runtime errors from map-server, writing or improving lua/annotations.lua EmmyLua docs, or preserving DSL-to-Lua behavioral parity with structured Lua schemas.
---

# rAthena Lua Dev Skill

Follow this workflow for all Lua-related work in rAthena.

## 1) Project Grounding

1. Work in the current workspace root (repository root).
2. Treat DSL behavior parity as primary for migrated scripts.
3. Keep formal Lua gameplay scripts under `lua/npc/...` (mirror source hierarchy).
4. Keep demos under `lua/demo/...`.
5. Keep load entries in `lua/scripts_lua.conf`.

## 2) Current Runtime Conventions

1. Runtime now binds Lua chunk environment automatically in `lua_engine`.
2. Do not require manual `local _ENV = ctx:env()` in normal business Lua scripts.
3. Use structured top-level schema only (`warps`, `monsters`, `mapflags`, `shops`, `duplicates`, `scripts`, `functions`).
4. Do not use legacy tuple keys (`w1/w2/w3/w4`).

## 3) File and Naming Rules

1. Use production-style names in `lua/npc/...`.
2. Do not use transition suffixes like `_gold`.
3. If splitting one source DSL file into multiple Lua files, use descriptive suffixes (example: `monster_race_single.lua`).

## 4) Load Chain Rules

1. Formal Lua script list is `lua/scripts_lua.conf`.
2. Ensure it is imported from:
- `npc/re/scripts_main.conf`
- `npc/pre-re/scripts_main.conf`
3. Do not keep formal Lua loads inside `npc/scripts_test.conf`.

## 5) Migration/Authoring Workflow

1. Pick a small batch (3-5 scripts) of complex targets.
2. Implement manual Lua rewrites with strict behavior parity.
3. Keep source traceability by matching folder hierarchy under `lua/npc/...`.
4. Add helper functions only when they improve clarity without changing behavior.
5. Avoid mutating original DSL `.txt` gameplay logic files for compatibility workarounds.

## 6) Validation Workflow

Run in order:

1. Lua syntax:
```bash
luac -p <changed-lua-files>
```

2. Server load check:
```bash
./map-server --run-once
```

3. If runtime errors occur, use stack trace file/line and patch exactly the failing path, then rerun.

## 7) Documentation and Handoff

1. Keep authoritative Lua docs in:
- `doc/script_lua.md`
- `doc/script_lua_schema.md`
- `lua/annotations.lua`

2. For every meaningful modification, add changelog note in:
- `doc/codex/yymmdd-hhmms-<what-you-did>.md`

3. Keep notes concise: summary, changed files, validation commands/results.

## 8) Quick Checklist Before Finish

1. No stale `_gold`/test-only naming in formal Lua paths.
2. `lua/scripts_lua.conf` references existing files only.
3. `luac -p` passes for touched scripts.
4. `./map-server --run-once` exits successfully.
5. Added `doc/codex` change note.

## References

Read these only when needed:
- `references/current-state.md`
