# 260208-212857-implement-all-lua-compat-exports

## Summary
Implemented all previously `lua_builtin_noop` compatibility exports in Lua runtime so they now execute real gameplay logic paths.

## Engine changes
Updated `/Users/cholf5/dev/rathena/src/map/lua_engine.cpp`:

- Added concrete runtime implementations for previously no-op commands, including:
  - UI/effects: `cutin`, `specialeffect`, `specialeffect2`
  - progression/helpers: `getexp`, `atoi`, `getmonsterinfo`, `getnpcid`, `sc_start2`
  - messaging/dialog-world: `emotion`, `npctalk`, `dispbottom`, `cleararray`, `questinfo`, `waitingroom`
  - npc visibility/control: `hideonnpc`, `hideoffnpc`, `cloakonnpc`, `cloakoffnpc`, `unloadnpc`
  - npc timers: `startnpctimer`, `stopnpctimer`, `setnpctimer`
  - waitingroom controls: `enablewaitingroomevent`, `disablewaitingroomevent`
  - world gameplay ops: `monster`, `areamonster`, `killmonster`, `bg_unbook`, `removemapflag`, `areawarp`, `mapwarp`, `areapercentheal`, `setcell`
  - unit/shop/npc ops: `setunitdata` (Lua path subset), `setunittitle`, `npcshopdelitem`, `npcshopupdate`, `movenpc`, `setwall`, `flagemblem`, `npcspeed`, `unitstopwalk`, `unitwalk`

- Registered all above builtins explicitly in `lua_register_builtins`.
- Shrunk `noop_builtins[]` to only `"end"` (reserved keyword compatibility name).
- Added required includes for these paths: battleground/chat/mob/status/unit headers.

## Annotation changes
Updated `/Users/cholf5/dev/rathena/npc/lua/annotations.lua`:

- Replaced "Compatibility no-op exports" wording with runtime-enabled compatibility exports wording.
- Removed `[No-op export]` labels from compatibility function docs.

## Validation
- Build passed: `make -C src/map -j4 map-server`
- Runtime smoke passed: `./map-server --npc-script-only --run-once` -> `EXIT:0`
- Annotation parse passed: `luac -p npc/lua/annotations.lua`

## Notes
- `setunitdata` in Lua currently covers core movement/direction/canmove-tick compatibility paths and logs warning for unsupported type ids in Lua path.
