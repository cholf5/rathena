# Gray Batch: RE job skill quests (2 files)

## Scope
Converted and switched the following files from `.txt` to `.lua`:

- `npc/re/quests/skills/merchant_skills.txt` -> `npc/re/quests/skills/merchant_skills.lua`
- `npc/re/quests/skills/swordman_skills.txt` -> `npc/re/quests/skills/swordman_skills.lua`

Updated config entries in:
- `npc/re/scripts_jobs.conf`

## Validation
- Conversion reports:
  - `.codex_tmp/re_jobs_batch8/merchant_skills.report.md`
  - `.codex_tmp/re_jobs_batch8/swordman_skills.report.md`
- Both reports: `unsupported_items: 0`
- Lua syntax check (`loadfile`) passed
- `make -j4` passed

## Deferred Item
- `npc/re/jobs/3-1/guillotine_cross.txt` remains `.txt` in this batch
- Current conversion report (`.codex_tmp/re_jobs_batch8/guillotine_cross.report.md`) still shows `unsupported_items: 28`

## Notes
- `./map-server` was not executed by Codex in this step.
