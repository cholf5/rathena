# Gray Batch: RE mapflags (all enabled entries)

## Scope
Converted and switched all enabled entries from `.txt` to `.lua` in:
- `npc/re/scripts_mapflags.conf`

Switched entries include:
- `nopvp`, `gvg`, `hidemobhpbar`, `nobranch`, `nocostume`, `noicewall`, `nolockon`, `nomemo`, `nopenalty`, `norenewalpenalty`, `nosave`, `noteleport`, `nowarpto`, `privateairship`, `night`, `restricted`, `town`, `reset`, `skill_duration`, `nodynamicnpc`, `specialpopup`.

Also aligned commented entry:
- `//npc: npc/re/mapflag/partylock.lua`

Generated/updated files under:
- `npc/re/mapflag/*.lua`

## Validation
- Batch conversion report: `.codex_tmp/re_mapflag_batch1/report.md`
- `unsupported_items: 0` (23/23)
- Lua syntax check (`loadfile`) passed for all generated mapflag lua files
- `make -j4` passed

## Notes
- `./map-server` was not run by Codex (per workflow).
