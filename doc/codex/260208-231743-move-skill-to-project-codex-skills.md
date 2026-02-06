# Move `rathena-lua-dev` skill to project-level `.codex/skills`

## Summary
Repositioned the skill into repository-local `.codex/skills` so teammates can receive it via Git.

## Changes
- Moved:
  - from: `skills/rathena-lua-dev`
  - to: `.codex/skills/rathena-lua-dev`
- Updated local discovery symlink:
  - `~/.codex/skills/rathena-lua-dev` -> `/Users/cholf5/dev/rathena/.codex/skills/rathena-lua-dev`

## Result
- Skill is now project-scoped and can be versioned with repository content.
- Teammates can get the skill through normal `git pull` (assuming they include `.codex/` in repo tracking).
