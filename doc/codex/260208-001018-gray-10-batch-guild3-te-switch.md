# 260208-001018 gray 10-batch guild3 te switch

## Scope
Performed 10 gray migration batches in one run for guild TE castle scripts.

## Switched entries
Updated `/Users/cholf5/dev/rathena/npc/re/scripts_guild.conf`:

1. `npc/re/guild3/te_aldecas1.txt` -> `npc/re/guild3/te_aldecas1.lua`
2. `npc/re/guild3/te_aldecas2.txt` -> `npc/re/guild3/te_aldecas2.lua`
3. `npc/re/guild3/te_aldecas3.txt` -> `npc/re/guild3/te_aldecas3.lua`
4. `npc/re/guild3/te_aldecas4.txt` -> `npc/re/guild3/te_aldecas4.lua`
5. `npc/re/guild3/te_aldecas5.txt` -> `npc/re/guild3/te_aldecas5.lua`
6. `npc/re/guild3/te_prtcas01.txt` -> `npc/re/guild3/te_prtcas01.lua`
7. `npc/re/guild3/te_prtcas02.txt` -> `npc/re/guild3/te_prtcas02.lua`
8. `npc/re/guild3/te_prtcas03.txt` -> `npc/re/guild3/te_prtcas03.lua`
9. `npc/re/guild3/te_prtcas04.txt` -> `npc/re/guild3/te_prtcas04.lua`
10. `npc/re/guild3/te_prtcas05.txt` -> `npc/re/guild3/te_prtcas05.lua`

## Validation
- Command:
  - `./map-server --npc-script-only --run-once`
- Result:
  - exit code `0`
  - summary: `checked=724 failed=0`
  - `/Users/cholf5/dev/rathena/errors.txt` line count: `0`
