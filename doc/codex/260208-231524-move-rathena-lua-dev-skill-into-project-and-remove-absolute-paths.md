# Move `rathena-lua-dev` skill into project and remove absolute paths

## Summary
Moved the handoff skill into this repository and removed hardcoded absolute paths from skill content.

## Changes
- Moved skill directory:
  - from: `/Users/cholf5/.codex/skills/rathena-lua-dev`
  - to: `/Users/cholf5/dev/rathena/skills/rathena-lua-dev`
- Updated `/Users/cholf5/dev/rathena/skills/rathena-lua-dev/SKILL.md`
  - Replaced repository absolute path text with workspace-relative wording.
- Added symlink for auto-discovery compatibility:
  - `/Users/cholf5/.codex/skills/rathena-lua-dev` -> `/Users/cholf5/dev/rathena/skills/rathena-lua-dev`

## Validation
- No remaining `/Users/cholf5` absolute paths in skill files:
  - `SKILL.md`
  - `references/current-state.md`
  - `agents/openai.yaml`
- Skill validation passed:
  - `python3 /Users/cholf5/.codex/skills/.system/skill-creator/scripts/quick_validate.py /Users/cholf5/dev/rathena/skills/rathena-lua-dev`
