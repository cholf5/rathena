# Create `rathena-lua-dev` handoff skill

## Summary
Created a reusable Codex skill so new sessions can continue rAthena Lua engine and gameplay script development with the same conventions and workflow.

## Skill location
- `/Users/cholf5/.codex/skills/rathena-lua-dev/SKILL.md`
- `/Users/cholf5/.codex/skills/rathena-lua-dev/agents/openai.yaml`
- `/Users/cholf5/.codex/skills/rathena-lua-dev/references/current-state.md`

## Included guidance
- Project grounding (`/Users/cholf5/dev/rathena`)
- Lua runtime + schema conventions
- Formal load-chain rules (`lua/scripts_lua.conf`)
- Migration workflow and validation commands
- Documentation and codex log requirements
- Current-state handoff reference for immediate continuation

## Notes
- `skill-creator` init script could not be used directly because local Python environment is missing `yaml` dependency (`ModuleNotFoundError: No module named 'yaml'`).
- Skill was created manually using the same required structure and metadata.
