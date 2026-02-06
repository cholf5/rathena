# 260208-215319-annotations-add-param-return-comments

## What changed
Updated `/Users/cholf5/dev/rathena/npc/lua/annotations.lua` to add inline EmmyLua comments to non-obvious `@param` and `@return` tags.

## Highlights
- Added inline comments for core runtime APIs (`get_var/set_var/setarray/input/callsub/callfunc/getarg/checkcell/checkre/gettime/getcastledata/setcastledata`).
- Added inline comments broadly across compatibility exports, especially gameplay-impacting functions (`mapannounce/warp/getitem/delitem/announce`, effect/status helpers, timer helpers, monster/map/unit/shop helpers).
- Kept obvious short parameters (e.g., plain `name`, `value` in straightforward wrappers) concise where comments would be redundant.

## Validation
- `luac -p npc/lua/annotations.lua` passed.
