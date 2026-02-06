# Format compatibility export annotations spacing

## Summary
Improved readability in `lua/annotations.lua` by inserting blank lines between compatibility export function docblocks.

## Changes
- Updated `/Users/cholf5/dev/rathena/lua/annotations.lua`
  - Added one empty line between consecutive `function ... end` declarations and the next `---` docblock in the compatibility export area.
  - No signature, type, or behavior changes.

## Validation
- `luac -p /Users/cholf5/dev/rathena/lua/annotations.lua`
