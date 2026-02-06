# rAthena Lua Script Manual

- Author: GPT-5.3-Codex
- Last Updated: 2026-02-08
- Status: Active reference for Lua NPC scripting

## Scope

This manual documents rAthena's Lua gameplay scripting interface. It does not teach Lua language basics.

This document covers:
- Script loading and file contract
- Top-level gameplay objects (`warps`, `monsters`, `mapflags`, `shops`, `duplicates`, `scripts`, `functions`)
- Runtime command/builtin surface used from Lua handlers
- Lua-first variable scope APIs
- Validation workflow

## 1. Script loading

Lua scripts are loaded through the same chain as NPC scripts.

Load entry:

```txt
npc: lua/npc/path/to/file.lua
```

Unload entry:

```txt
delnpc: lua/npc/path/to/file.lua
```

Notes:
- `.txt` (DSL) and `.lua` can coexist at server level.
- New gameplay should be authored in `.lua`.
- Formal Lua load list: `lua/scripts_lua.conf`.

## 2. Lua file contract

Each Lua script file must return one table:

```lua
return {
  warps = {},
  monsters = {},
  mapflags = {},
  shops = {},
  duplicates = {},
  scripts = {},
  functions = {},
}
```

Full schema reference:
- `doc/script_lua_schema.md`

## 3. Top-level gameplay object definitions

## 3.1 Warps

```lua
warps = {
  {
    map = "prontera", x = 156, y = 180, dir = 0,
    name = "prt_to_inn",
    xs = 2, ys = 2,
    to_map = "prt_in", to_x = 58, to_y = 86,
  },
}
```

Required fields: `map`, `name`, `to_map`.

## 3.2 Monsters

```lua
monsters = {
  {
    map = "prt_fild08", x = 0, y = 0, xs = 0, ys = 0,
    name = "Poring", mob_id = 1002, amount = 20,
  },
}
```

Required fields: `map`, `name`, `mob_id`, `amount`.

## 3.3 Mapflags

```lua
mapflags = {
  { map = "prontera", flag = "nobranch" },
  { map = "prontera", flag = "bexp", value = "50" },
  { map = "prontera", flag = "nobranch", off = true },
}
```

Required fields: `map`, `flag`.

## 3.4 Shops

```lua
shops = {
  {
    map = "prontera", x = 156, y = 180, dir = 0,
    type = "shop",
    name = "Tool Dealer",
    items = "501:2,502:10,503:100",
  },
}
```

Required fields: `name`, `items`.

## 3.5 Duplicates

```lua
duplicates = {
  {
    map = "prontera", x = 155, y = 181, dir = 4,
    source = "OriginalNpc",
    name = "DuplicateNpc",
    view = "1_M_MERCHANT",
  },
}
```

Required fields: `source`, `name`, `view`.

## 3.6 NPC scripts

Lua NPC definitions live in top-level `scripts = { ... }`.

```lua
scripts = {
  {
    map = "prontera", x = 150, y = 180, dir = 4,
    type = "script",
    name = "Lua Example NPC",
    sprite = 1,

    main = function(ctx)
      mes("[Lua Example NPC]")
      mes("Hello from Lua runtime")
      close()
    end,

    events = {
      OnInit = function(ctx)
      end,
    },

    labels = {
      S_Test = function(ctx)
        return
      end,
    },
  },
}
```

Required fields: `name`, plus one executable body source:
- `body` (DSL text body string), or
- runtime handlers (`main` / `events` / `labels`).

## 3.7 Global functions

```lua
functions = {
  {
    name = "F_MyFunction",
    run = function(ctx, ...)
      return
    end,
    labels = {
      L_Inner = function(ctx)
      end,
    },
  },
}
```

Called with `callfunc("F_MyFunction", ...)`.

## 4. Runtime execution model

Lua chunk environment is bound by the engine at load time to the runtime export table.  
That means handlers can call gameplay APIs directly (`mes`, `warp`, `donpcevent`, etc.) without explicitly writing `local _ENV = ctx:env()`.

Flow commands (`next`, `sleep`, `input`, `close2`) may yield and resume.

## 5. Lua-first variable APIs

## 5.1 Generic API

```lua
get_var(scope, name[, index]) -> value | nil
set_var(scope, name, value[, index])
```

Supported scope values:
- `character`, `char`, `player`
- `character_temp`, `char_temp`, `player_temp`
- `account`
- `account_global`, `global_account`, `account2`
- `map_server`, `server`, `global`
- `npc`
- `npc_temp`, `local`
- `instance`

## 5.2 Scope-specific APIs

- `get_character_var` / `set_character_var`
- `get_character_temp_var` / `set_character_temp_var`
- `get_account_var` / `set_account_var`
- `get_account_global_var` / `set_account_global_var`
- `get_map_server_var` / `set_map_server_var`
- `get_npc_var` / `set_npc_var`
- `get_npc_temp_var` / `set_npc_temp_var`
- `get_instance_var` / `set_instance_var`

## 5.3 Alias tables

- `var.get/set(scope, name[, index])`
- `character.get/set`
- `character_temp.get/set`
- `account.get/set`
- `account_global.get/set`
- `map_server.get/set`
- `npc_var.get/set`
- `npc_temp.get/set`
- `instance.get/set`

## 6. Runtime command reference (high frequency)

## 6.1 Dialog and interaction

- `mes(...)`
- `next()`
- `clear()`
- `close()` / `close2()` / `close3()`
- `select(...)` / `menu(...)`
- `input(var[,min,max])`

## 6.2 Flow and calls

- `sleep(ms)` / `sleep2(ms)`
- `callsub(label, ...)`
- `callfunc(name, ...)`
- `getarg(index[,default])`
- `getargcount()`

## 6.3 Variables and dynamic helpers

- `set(name, value)`
- `getd(name)`
- `setd(name, value)`
- `setarray(name[,start], ...values)`
- `getarraysize(var)`

## 6.4 Common utility helpers

- `rand(max)` / `rand(min,max)`
- `compare(a,b)`
- `charat(str, idx)`
- `getstrlen(str)`
- `replacestr(src,find,repl)`
- `strcharinfo(type)`
- `strnpcinfo(type)`
- `countitem(item)`
- `checkweight(...)`
- `checkre()`
- `gettime(type)`

## 6.5 NPC/event helpers

- `donpcevent("Npc::Event")`
- `enablenpc([name])`
- `disablenpc([name])`
- `initnpctimer()` / `Initnpctimer()`

## 7. Rules for new Lua scripts

1. Use structured object fields only; do not use legacy `w1/w2/w3/w4` tuple-style keys.
2. Prefer Lua local state and helper functions for control flow.
3. Use scoped variable APIs for persistent/shared data.
4. Keep handlers explicit (`main`, `events`, `labels`, `run`).
5. Keep one gameplay concern per file when possible.

## 7.1 Path and naming conventions

1. Mirror source DSL hierarchy under `lua/npc/...` for traceability.
2. If one DSL file is split into multiple Lua files, use descriptive suffixes (example: `monster_race_single.lua`).
3. Do not use transition suffixes like `_gold` in production Lua filenames.

## 7.2 `_ENV` usage convention

Do not require `local _ENV = ctx:env()` in normal business scripts.
The engine pre-binds runtime environment automatically.

Use `ctx:env()` only when you intentionally want an explicit local alias table for readability, testing, or passing env into helper functions.

## 8. Minimal template

```lua
return {
  warps = {},
  monsters = {},
  mapflags = {},
  shops = {},
  duplicates = {},
  scripts = {
    {
      map = "prontera", x = 150, y = 180, dir = 4,
      name = "Lua Example NPC",
      sprite = 1,
      main = function(ctx)
        mes("[Lua Example NPC]")
        close()
      end,
    },
  },
  functions = {},
}
```

## 9. Validation workflow

1. Syntax check:

```bash
luac -p npc/path/to/file.lua
```

2. Script load check:

```bash
./map-server --npc-script-only --run-once
```

3. Full runtime check:

```bash
./map-server --run-once
```
