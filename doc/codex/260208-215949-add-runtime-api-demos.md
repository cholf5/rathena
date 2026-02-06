# 260208-215949-add-runtime-api-demos

## What changed
Added richer Lua demo coverage for newly exported runtime compatibility APIs.

## Files changed
- Added `/Users/cholf5/dev/rathena/npc/lua/demo_runtime_exports.lua`
  - New interactive NPC demo covering grouped runtime APIs:
    - visual/info (`cutin`, `specialeffect`, `specialeffect2`, `emotion`, `npctalk`, `dispbottom`, `atoi`, `getmonsterinfo`, `getnpcid`)
    - inventory/status (`getitem`, `delitem`, `countitem`, `checkweight`, `sc_start2`)
    - spawn/broadcast (`monster`, `areamonster`, `killmonster`, `mapannounce`, `announce`)
    - map/area ops (`setcell`, `removemapflag`, `areapercentheal`, `areawarp`, `mapwarp`)
    - npc/unit ops (`hideonnpc`, `hideoffnpc`, `cloakonnpc`, `cloakoffnpc`, `npcspeed`, `movenpc`, `setunittitle`, `setunitdata`, `unitwalk`, `unitstopwalk`, `flagemblem`)
    - waitingroom/shop/timer (`waitingroom`, `enablewaitingroomevent`, `disablewaitingroomevent`, `npcshopupdate`, `npcshopdelitem`, `setnpctimer`, `startnpctimer`, `stopnpctimer`, `bg_unbook`, `questinfo`)
    - unload demo (`unloadnpc`) behind explicit confirmation menu
- Updated `/Users/cholf5/dev/rathena/npc/lua/scripts_demo.conf`
  - Added: `npc: npc/lua/demo_runtime_exports.lua`
- Updated `/Users/cholf5/dev/rathena/npc/lua/README.md`
  - Documented the new runtime demo file.
  - Added warning that this demo is side-effectful and test-server only.

## Validation
- `luac -p` passed for:
  - `npc/lua/demo_all_objects.lua`
  - `npc/lua/demo_npc_medium.lua`
  - `npc/lua/demo_npc_advanced.lua`
  - `npc/lua/demo_runtime_exports.lua`
  - `npc/lua/annotations.lua`
- `./map-server --npc-script-only --run-once` passed with `EXIT:0`.
