# NPC Lua Top-Level Schema

Each NPC Lua file must `return` one table.

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

## Object schemas

## `warps[]`

```lua
{
  map = "prontera",
  x = 156,
  y = 180,
  dir = 0,
  name = "prt_to_inn",
  xs = 2,
  ys = 2,
  to_map = "prt_in",
  to_x = 58,
  to_y = 86,
  type = "warp",      -- optional, default "warp"
  state = "",         -- optional, appends to type as type(state)
}
```

Required: `map`, `name`, `to_map`.

## `monsters[]`

```lua
{
  map = "prt_fild08",
  x = 0,
  y = 0,
  xs = 0,
  ys = 0,
  name = "Poring",
  mob_id = 1002,
  amount = 20,
  delay1 = 0,          -- optional
  delay2 = 0,          -- optional
  event_name = "",    -- optional
  size = -1,           -- optional
  ai = -1,             -- optional
  boss = false,        -- optional: true -> boss_monster
}
```

Required: `map`, `name`, `mob_id`, `amount`.

## `mapflags[]`

```lua
{
  map = "prontera",
  flag = "nobranch",
  value = "",         -- optional
  off = false,         -- optional
}
```

Required: `map`, `flag`.

## `shops[]`

```lua
{
  map = "prontera",
  x = 156,
  y = 180,
  dir = 0,
  type = "shop",      -- optional, default "shop"
  name = "Tool Dealer",
  items = "501:2,502:10,503:100",
}
```

Required: `name`, `items`.

`map = "-"` is allowed for floating definitions.

## `duplicates[]`

```lua
{
  map = "prontera",
  x = 155,
  y = 181,
  dir = 4,
  source = "OriginalNpc",
  name = "DuplicateNpc",
  view = "1_M_MERCHANT",
}
```

Required: `source`, `name`, `view`.

`map = "-"` is allowed for floating definitions.

## `scripts[]` (NPC)

```lua
{
  map = "prontera",   -- or "-"
  x = 150,
  y = 180,
  dir = 4,
  type = "script",    -- optional, default "script"
  state = "",         -- optional, appends to type as type(state)
  name = "Lua Example NPC",
  sprite = 1,
  trigger_x = -1,      -- optional; include with trigger_y
  trigger_y = -1,      -- optional; include with trigger_x

  body = [[
    mes "Hello";
    close;
  ]],

  -- Runtime Lua handlers used by the Lua engine:
  main = function(ctx) end,
  events = {
    OnInit = function(ctx) end,
  },
  labels = {
    S_Label = function(ctx) end,
  },
}
```

Required: `name` and one of `body` or runtime handlers (`main`/`events`/`labels`).

## `functions[]`

```lua
{
  name = "F_MyFunc",
  body = [[
    return;
  ]],

  -- Runtime Lua handlers used by the Lua engine:
  run = function(ctx, ...) end,
  labels = {
    L_Local = function(ctx) end,
  },
}
```

Required: `name` and one of `body` or runtime handlers (`run`/`labels`).

## Notes

- This schema is the only supported authoring format for Lua gameplay objects.
- Do not use legacy `w1/w2/w3/w4` tuple-style fields in new Lua scripts.
