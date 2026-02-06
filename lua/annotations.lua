---@meta

-- rAthena Lua runtime annotations
-- Source of truth: src/map/lua_engine.cpp (lua_register_builtins)
-- This file is IDE/static-analysis metadata only.

---@alias LuaScopeName
---| 'character'| 'char'| 'player'
---| 'character_temp'| 'char_temp'| 'player_temp'
---| 'account'
---| 'account_global'| 'global_account'| 'account2'
---| 'map_server'| 'server'| 'global'
---| 'npc'
---| 'npc_temp'| 'local'
---| 'instance'

---@class LuaScopeAccessor
--- Scope-backed variable accessor.
--- `get` returns nil when key/index is not present in this scope.
---@field get fun(name:string, index?:integer):any
--- Writes a scalar or indexed element in this scope.
---@field set fun(name:string, value:any, index?:integer)

---@class LuaNpcContext
--- Runtime execution context passed to `main/events/labels/run`.
---@field rid integer # Attached player RID (0 when not attached)
---@field oid integer # Attached object id / invoker object id
---@field npc string # Current NPC/function execution name
---@field env fun(self:LuaNpcContext):LuaNpcEnv # Returns the runtime environment table

---@class LuaNpcEnv
--- Runtime environment table used via `local _ENV = ctx:env()`.
--- Contains all registered command functions and scope tables.
---@field rid integer
---@field oid integer
---@field npc string
---@field var LuaScopeAccessor
---@field character LuaScopeAccessor
---@field character_temp LuaScopeAccessor
---@field account LuaScopeAccessor
---@field account_global LuaScopeAccessor
---@field map_server LuaScopeAccessor
---@field npc_var LuaScopeAccessor
---@field npc_temp LuaScopeAccessor
---@field instance LuaScopeAccessor

---@type LuaScopeAccessor
var = var
---@type LuaScopeAccessor
character = character
---@type LuaScopeAccessor
character_temp = character_temp
---@type LuaScopeAccessor
account = account
---@type LuaScopeAccessor
account_global = account_global
---@type LuaScopeAccessor
map_server = map_server
---@type LuaScopeAccessor
npc_var = npc_var
---@type LuaScopeAccessor
npc_temp = npc_temp
---@type LuaScopeAccessor
instance = instance

-- -----------------------------------------------------------------------------
-- Scoped variable API
-- -----------------------------------------------------------------------------

--- Gets a value from a specific runtime/game-data scope.
---
--- Supported scopes:
--- `character`, `char`, `player`, `character_temp`, `char_temp`, `player_temp`,
--- `account`, `account_global`, `global_account`, `account2`,
--- `map_server`, `server`, `global`, `npc`, `npc_temp`, `local`, `instance`.
---
--- Behavior:
--- - Returns nil when the value does not exist in the selected scope.
--- - When `index` is provided, reads indexed (array-like) slot.
--- - Value type is runtime-dependent (`number|string|boolean|table|nil`).
---
--- Example:
--- ```lua
--- local kills = get_var("character", "mvp_kills") or 0
--- local first = get_var("character_temp", "daily_rewards", 0)
--- ```
---@param scope LuaScopeName @variable scope namespace (`character/account/map_server/...`)
---@param name string @variable name without legacy prefix/suffix
---@param index? integer @optional array index for indexed variables
---@return any @resolved value in scope, or nil when key/index is absent
function get_var(scope, name, index) end

--- Sets a value in a specific runtime/game-data scope.
---
--- Behavior:
--- - Creates the variable when missing.
--- - Overwrites existing scalar value when writing scalar.
--- - When `index` is provided, writes indexed (array-like) slot.
--- - `value=nil` semantics may clear value depending on runtime implementation.
---
--- Example:
--- ```lua
--- set_var("account", "daily_ticket_used", 1)
--- set_var("character_temp", "selected_ids", 3004, 0)
--- ```
---@param scope LuaScopeName @variable scope namespace (`character/account/map_server/...`)
---@param name string @variable name without legacy prefix/suffix
---@param value any @value to write
---@param index? integer @optional array index for indexed writes
function set_var(scope, name, value, index) end

--- Gets character scope variable.
---@param name string
---@param index? integer
---@return any
function get_character_var(name, index) end

--- Sets character scope variable.
---@param name string
---@param value any
---@param index? integer
function set_character_var(name, value, index) end

--- Gets character temporary scope variable.
---@param name string
---@param index? integer
---@return any
function get_character_temp_var(name, index) end

--- Sets character temporary scope variable.
---@param name string
---@param value any
---@param index? integer
function set_character_temp_var(name, value, index) end

--- Gets account scope variable.
---@param name string
---@param index? integer
---@return any
function get_account_var(name, index) end

--- Sets account scope variable.
---@param name string
---@param value any
---@param index? integer
function set_account_var(name, value, index) end

--- Gets account-global scope variable.
---@param name string
---@param index? integer
---@return any
function get_account_global_var(name, index) end

--- Sets account-global scope variable.
---@param name string
---@param value any
---@param index? integer
function set_account_global_var(name, value, index) end

--- Gets map-server/global scope variable.
---@param name string
---@param index? integer
---@return any
function get_map_server_var(name, index) end

--- Sets map-server/global scope variable.
---@param name string
---@param value any
---@param index? integer
function set_map_server_var(name, value, index) end

--- Gets NPC scope variable.
---@param name string
---@param index? integer
---@return any
function get_npc_var(name, index) end

--- Sets NPC scope variable.
---@param name string
---@param value any
---@param index? integer
function set_npc_var(name, value, index) end

--- Gets NPC temporary/local scope variable.
---@param name string
---@param index? integer
---@return any
function get_npc_temp_var(name, index) end

--- Sets NPC temporary/local scope variable.
---@param name string
---@param value any
---@param index? integer
function set_npc_temp_var(name, value, index) end

--- Gets instance scope variable.
---@param name string
---@param index? integer
---@return any
function get_instance_var(name, index) end

--- Sets instance scope variable.
---@param name string
---@param value any
---@param index? integer
function set_instance_var(name, value, index) end

-- -----------------------------------------------------------------------------
-- Legacy variable helpers (compatibility surface)
-- -----------------------------------------------------------------------------

--- Assigns a variable by runtime-visible name (legacy-style helper).
---
--- Prefer scope APIs (`set_var` or scope tables) for new scripts.
---@param name string
---@param value any
function set(name, value) end

--- Assigns a variable via dynamic key string.
---
--- Kept for compatibility with converted scripts.
---@param name string
---@param value any
function setd(name, value) end

--- Reads a variable via dynamic key string.
---
--- Kept for compatibility with converted scripts.
---@param name string
---@return any
function getd(name) end

--- Writes an array-like variable.
---
--- Pattern usually mirrors legacy DSL behavior where the second argument may be
--- start index or first value depending on call form.
---@param name string @target variable name
---@param start_or_value any @start index or first value (compat semantics)
---@param ... any @remaining values written sequentially
function setarray(name, start_or_value, ...) end

-- -----------------------------------------------------------------------------
-- Dialog / interaction
-- -----------------------------------------------------------------------------

--- Appends dialog lines to the current NPC dialog window.
---
--- Notes:
--- - Each argument is converted to string by runtime conventions.
--- - Use `next()` to paginate long conversations.
---@param text1 any
---@param text2? any
---@param text3? any
---@param text4? any
function mes(text1, text2, text3, text4) end

--- Advances dialog to next page and may yield until user input.
---
--- This is a suspension point for coroutine-backed NPC execution.
function next() end

--- Clears current dialog contents.
function clear() end

--- Closes dialog and ends script flow.
---
--- Usually used as terminal step in dialogue handlers.
function close() end

--- Closes dialog with continuation semantics used by some flows.
function close2() end

--- Additional close variant for compatibility with existing scripts.
function close3() end

--- Shows a selection menu and returns the 1-based selected option index.
---
--- Accepts either a single colon-delimited string or multiple option arguments.
---
--- Example:
--- ```lua
--- local c = select("Buy:Sell:Leave")
--- if c == 1 then
---   -- Buy
--- end
--- ```
---@param option_or_menu any
---@param option2? any
---@param option3? any
---@param option4? any
---@param option5? any
---@return integer
function select(option_or_menu, option2, option3, option4, option5) end

--- Menu alias with similar behavior to `select`.
---@param option_or_menu any
---@param option2? any
---@param option3? any
---@param option4? any
---@param option5? any
---@return integer
function menu(option_or_menu, option2, option3, option4, option5) end

--- Requests user input and stores result in the target variable.
---
--- `min/max` are optional bounds for numeric input in compatible paths.
---
--- Example:
--- ```lua
--- input(".@amount", 1, 100)
--- ```
---@param var_name string @destination variable for captured input
---@param min? integer @numeric lower bound (when numeric input path is used)
---@param max? integer @numeric upper bound (when numeric input path is used)
---@return any @captured value (number/string by variable convention)
function input(var_name, min, max) end

-- -----------------------------------------------------------------------------
-- Flow / call utilities
-- -----------------------------------------------------------------------------

--- Suspends current execution for `ms` milliseconds.
---
--- Notes:
--- - This is a yield point.
--- - Runtime resumes the script instance via timer callback.
---
--- Example:
--- ```lua
--- mes("Please wait...")
--- sleep(1000)
--- mes("Done.")
--- ```
---@param ms integer
function sleep(ms) end

--- Compatibility variant of `sleep`.
---@param ms integer
function sleep2(ms) end

--- Calls local label/function in current script scope.
---
--- Typical use is delegating branch logic to local labels.
---@param label string @target local label/function name
---@param ... any @arguments forwarded to target
---@return any @values returned by target label/function
function callsub(label, ...) end

--- Calls a registered global function block by name.
---
--- Returns values produced by the called function when available.
---
--- Example:
--- ```lua
--- local txt = callfunc("LUA_DEMO_HELLO")
--- mes(txt)
--- ```
---@param name string @registered global function name
---@param ... any @arguments forwarded to called function
---@return any @values returned by called function
function callfunc(name, ...) end

--- Gets argument at `index` from current call context.
--- Returns `default` when index is out of range.
---
--- Index is 0-based for compatibility with script command behavior.
---@param index integer @0-based argument index in current call frame
---@param default? any @fallback when index is out of range
---@return any @argument value or fallback
function getarg(index, default) end

--- Returns number of call arguments in current context.
---@return integer
function getargcount() end

--- Initializes NPC timer for current NPC context.
function initnpctimer() end

--- Alias of `initnpctimer` kept for case-compatibility.
function Initnpctimer() end

-- -----------------------------------------------------------------------------
-- Utility / status / map helpers
-- -----------------------------------------------------------------------------

--- Returns random integer.
---
--- Overloads:
--- - `rand(max)` => pseudo-random in `[0, max)` style compatibility range
--- - `rand(min, max)` => pseudo-random between two bounds (engine-compatible semantics)
---@overload fun(max:integer):integer
---@param min integer
---@param max integer
---@return integer
function rand(min, max) end

--- Character info query helper (compatible with script command behavior).
---
--- Typical values for `type` follow script-command conventions
--- (for example: name, party/guild/map-related textual fields).
---@param type integer
---@return string
function strcharinfo(type) end

--- NPC info query helper.
---
--- Common usage is obtaining current NPC name, map, and script metadata.
---@param type integer
---@return string
function strnpcinfo(type) end

--- Compares two values and returns compatibility-style integer result.
---
--- Usually used in converted logic where boolean conditions were expressed via
--- script command integer conventions.
---@param a any
---@param b any
---@return integer
function compare(a, b) end

--- Weight/capacity check helper.
---
--- Returns non-zero when requested operation is allowed by weight/capacity rules.
---@param item_id_or_name integer|string
---@param amount integer
---@param item_id_or_name2? integer|string
---@param amount2? integer
---@return integer
function checkweight(item_id_or_name, amount, item_id_or_name2, amount2) end

--- Cell-state query helper.
---
--- Queries a specific map coordinate for a given cell property/type code.
---@param map string @map name
---@param x integer @x coordinate
---@param y integer @y coordinate
---@param celltype integer @cell check selector (`CELL_CHK*`)
---@return integer @cell check result (compat integer)
function checkcell(map, x, y, celltype) end

--- Counts amount of item in inventory.
---
--- `item_id` is numeric item ID from item DB.
---@param item_id integer
---@return integer
function countitem(item_id) end

--- Checks whether server is in RE mode (compat helper).
---
--- Often used in scripts that branch between pre-RE and RE behavior.
---@param mode? integer @optional compatibility mode selector
---@return integer @non-zero when RE mode is active
function checkre(mode) end

--- Returns table/array size in compatibility semantics.
---
--- For sparse tables, result follows runtime command behavior, not pure Lua `#`.
---@param arr any
---@return integer
function getarraysize(arr) end

--- Returns character at string index.
---
--- Index semantics follow script-command compatibility behavior.
---@param s string
---@param idx integer
---@return string
function charat(s, idx) end

--- Returns string length.
---@param s string
---@return integer
function getstrlen(s) end

--- Returns time component by type code.
---
--- The `kind` parameter matches script command conventions (year, month, day, etc).
---@param kind integer @time field selector (`DT_*` constants)
---@return integer @selected time value
function gettime(kind) end

--- Replaces occurrences of `find` with `repl` in `src`.
---@param src string
---@param find string
---@param repl string
---@return string
function replacestr(src, find, repl) end

--- WoE status check helper.
---
--- Returns non-zero when WoE mode is active for this check path.
---@return integer
function agitcheck() end

--- Additional WoE status check helper.
---
--- Used by scripts that need alternate WoE mode checks.
---@return integer
function agitcheck3() end

--- Gets castle data field.
---
--- `field` is castle-data field code used by guild/castle scripts.
---@param castle string @castle map name / castle key
---@param field integer @castle field selector (`CD_*`)
---@return any @field value for selected castle data
function getcastledata(castle, field) end

--- Sets castle data field.
---
--- Persists/updates castle state according to field code semantics.
---@param castle string @castle map name / castle key
---@param field integer @castle field selector (`CD_*`)
---@param value any @new value for selected field
function setcastledata(castle, field, value) end

--- Triggers another NPC event (`NpcName::OnEvent`).
---
--- Example:
--- ```lua
--- donpcevent("QuestBoard::OnRefresh")
--- ```
---@param event_name string
function donpcevent(event_name) end

--- Disables NPC interaction/visibility (current or named).
---@param name? string
function disablenpc(name) end

--- Enables NPC interaction/visibility (current or named).
---@param name? string
function enablenpc(name) end

--- Gets battle configuration flag by name.
---
--- Returns runtime-config value for script-side branching.
---@param name string
---@return any
function getbattleflag(name) end

--- Reads variable from another NPC by variable + npc name.
---
--- Compatibility helper mainly for legacy script patterns.
---@param var_name string
---@param npc_name string
---@return any
function getvariableofnpc(var_name, npc_name) end

-- -----------------------------------------------------------------------------
-- Compatibility exports (implemented)
-- -----------------------------------------------------------------------------

--- Broadcasts a message to players on a map.
---
--- Parameters:
--- 1. `map_name`: map name
--- 2. `message`: text to broadcast
--- 3. `flag`: broadcast flag (default `BC_DEFAULT`)
---
--- Current implementation supports the standard `mapannounce(map, msg, flag)` path.
---@param map_name string @target map name receiving broadcast
---@param message string @announcement text sent to players on map
---@param flag? integer @broadcast behavior/color flags (`BC_*`)
function mapannounce(map_name, message, flag) end

--- Warps a player to target map/position.
---
--- Parameters:
--- 1. `map_name`: target map, or `"Random"`, `"SavePoint"`, `"Save"`
--- 2. `x`: target x
--- 3. `y`: target y
--- 4. `char_id?`: optional target char id (defaults to attached RID player)
---
--- Notes:
--- - `"Random"` ignores `x/y`.
--- - `"SavePoint"` and `"Save"` use player save location.
---@param map_name string @destination map name or reserved target (`Random`/`SavePoint`/`Save`)
---@param x integer @destination x coordinate
---@param y integer @destination y coordinate
---@param char_id? integer @optional target character id (defaults to attached player)
function warp(map_name, x, y, char_id) end

--- Gives an item to player inventory.
---
--- Parameters:
--- 1. `item`: item id or item name
--- 2. `amount`: item count
--- 3. `account_map_id?`: optional target player map/account id
---
--- Notes:
--- - Stackable and non-stackable item behavior follows map-server inventory rules.
--- - Pet egg item ids will trigger egg handling path.
---@param item integer|string @item id or item name
---@param amount integer @quantity to grant
---@param account_map_id? integer @optional target player id in compatibility path
function getitem(item, amount, account_map_id) end

--- Deletes item(s) from player inventory.
---
--- Parameters:
--- 1. `item`: item id or item name
--- 2. `amount`: count to delete
--- 3. `account_map_id?`: optional target player map/account id
---
--- Notes:
--- - Deletion prefers non-equipped stacks, then equipped stacks.
--- - If quantity is insufficient, remaining amount is not deleted and runtime logs an error.
---@param item integer|string @item id or item name
---@param amount integer @quantity to remove
---@param account_map_id? integer @optional target player id in compatibility path
function delitem(item, amount, account_map_id) end

--- Sends a broadcast message.
---
--- Parameters:
--- 1. `message`: text
--- 2. `flag`: broadcast flag (default `BC_DEFAULT`)
--- 3. `char_id?`: optional source player char id for non-`BC_NPC` modes
---
--- Notes:
--- - Targeting behavior follows `BC_ALL/BC_MAP/BC_AREA/BC_SELF`.
--- - When `BC_NPC` is set, current NPC is used as source when available.
---@param message string @broadcast text payload
---@param flag? integer @broadcast scope/style flags (`BC_*`)
---@param char_id? integer @optional source player char id for non-`BC_NPC` routes
function announce(message, flag, char_id) end

-- -----------------------------------------------------------------------------
-- Compatibility exports
-- -----------------------------------------------------------------------------

-- These names are exported for gameplay compatibility and have runtime behavior in Lua mode.

--- Client portrait/cutin helper.
---@param image string @cutin resource key / image filename
---@param mode integer @client cutin mode selector
function cutin(image, mode) end

--- Visual effect helper.
---@param effect_id integer @visual effect id (`EF_*`)
---@param target? integer @send target (`AREA`/`SELF`/...)
---@param npc_name? string @optional NPC source override
function specialeffect(effect_id, target, npc_name) end

--- Visual effect helper (variant).
---@param effect_id integer @visual effect id (`EF_*`)
---@param target? integer @send target (`AREA`/`SELF`/...)
---@param account_id? integer @optional target account/map id
function specialeffect2(effect_id, target, account_id) end

--- Award EXP.
---@param base_exp integer @base EXP before quest rate scaling
---@param job_exp integer @job EXP before quest rate scaling
---@param char_id? integer @optional target character id
function getexp(base_exp, job_exp, char_id) end
-- builtin `end` is exported by runtime but cannot be declared as a Lua identifier.
--- String to integer conversion helper.
---@param value string
---@return integer
function atoi(value) end

--- Query monster info.
---@param mob_id_or_name integer|string
---@param query_type integer @selector (`MOB_*` constants)
---@return integer|string @resolved field value by selector
function getmonsterinfo(mob_id_or_name, query_type) end

--- Query NPC ID.
---@param mode integer @query mode (`0` => NPC GID)
---@param npc_name? string @optional NPC unique name
---@return integer @resolved NPC GID (or `0` on failure)
function getnpcid(mode, npc_name) end

--- Start status change helper.
---@param effect_id integer @status id (`SC_*`)
---@param duration_ms integer @status duration in milliseconds
---@param val1 integer @status payload value #1
---@param val2 integer @status payload value #2
---@param rate? integer @success chance in 1/10000 units
---@param flag? integer @start flags (`SCSTART_*`)
---@param unit_id? integer @optional explicit target unit id
function sc_start2(effect_id, duration_ms, val1, val2, rate, flag, unit_id) end

--- Emotion bubble helper.
---@param emotion_id integer @emotion id (`ET_*`)
---@param target_id? integer @optional target gid (defaults to current context object)
function emotion(emotion_id, target_id) end

--- NPC talk bubble helper.
---@param message string @text shown as NPC chat bubble
---@param npc_name? string @optional source NPC name
---@param flag? integer @chat target mode (`BC_AREA/BC_MAP/BC_ALL/BC_SELF`)
---@param color? integer @RGB hex color for bubble text
function npctalk(message, npc_name, flag, color) end

--- Bottom message helper.
---@param message string @bottom/system message text
---@param color? integer @optional RGB hex color
---@param char_id? integer @optional target character id
function dispbottom(message, color, char_id) end

--- Array clear helper.
---@param var_name string @target array-like variable name
---@param value integer|string @fill value used to overwrite slots
---@param count integer @number of slots to clear/fill
function cleararray(var_name, value, count) end

--- Quest marker/info helper.
---@param icon integer @quest icon type (`QTYPE_*`)
---@param color? integer @map marker color (`QMARK_*`)
---@param condition? string @optional legacy condition expression
function questinfo(icon, color, condition) end

--- Waiting room creation/update helper.
---@param title string @waiting room title
---@param limit integer @maximum allowed users
---@param event_name? string @event label triggered on threshold
---@param trigger? integer @user-count threshold for event trigger
---@param zeny? integer @entry fee
---@param min_level? integer @minimum level requirement
---@param max_level? integer @maximum level allowed
function waitingroom(title, limit, event_name, trigger, zeny, min_level, max_level) end

--- Hide NPC helper.
---@param npc_name? string @target NPC name (defaults to current NPC)
---@param char_id? integer @optional per-character visibility target
function hideonnpc(npc_name, char_id) end

--- Unhide NPC helper.
---@param npc_name? string @target NPC name (defaults to current NPC)
---@param char_id? integer @optional per-character visibility target
function hideoffnpc(npc_name, char_id) end

--- Unload NPC helper.
---@param npc_name string @target NPC unique name to unload
function unloadnpc(npc_name) end

--- Stop NPC timer helper.
---@param npc_name? string @target NPC name (defaults to current NPC)
function stopnpctimer(npc_name) end

--- Set NPC timer helper.
---@param tick integer @new timer tick in milliseconds
---@param npc_name? string @target NPC name (defaults to current NPC)
function setnpctimer(tick, npc_name) end

--- Start NPC timer helper.
---@param npc_name_or_attach? string|integer @NPC name or attach-flag shorthand
---@param attach_flag? integer @when non-zero, attach current RID to timer
function startnpctimer(npc_name_or_attach, attach_flag) end

--- Cloak NPC helper.
---@param npc_name? string @target NPC name (defaults to current NPC)
---@param char_id? integer @optional per-character visibility target
function cloakonnpc(npc_name, char_id) end

--- Uncloak NPC helper.
---@param npc_name? string @target NPC name (defaults to current NPC)
---@param char_id? integer @optional per-character visibility target
function cloakoffnpc(npc_name, char_id) end

--- Enable waiting-room event helper.
---@param npc_name? string @target NPC name (defaults to current NPC)
function enablewaitingroomevent(npc_name) end

--- Disable waiting-room event helper.
---@param npc_name? string @target NPC name (defaults to current NPC)
function disablewaitingroomevent(npc_name) end

--- Spawn monster helper.
---@param map_name string @spawn map (`"this"` supported when player is attached)
---@param x integer @spawn center x
---@param y integer @spawn center y
---@param visible_name string @name shown on spawned mobs
---@param mob_id_or_name integer|string @mob class id or Aegis name
---@param amount integer @number of mobs to spawn
---@param event_name? string @event label triggered by mob death
---@param size? integer @spawn size override (`SZ_*`)
---@param ai? integer @AI mode override (`AI_*`)
function monster(map_name, x, y, visible_name, mob_id_or_name, amount, event_name, size, ai) end

--- Area monster spawn helper.
---@param map_name string @spawn map (`"this"` supported when player is attached)
---@param x0 integer @area min x
---@param y0 integer @area min y
---@param x1 integer @area max x
---@param y1 integer @area max y
---@param visible_name string @name shown on spawned mobs
---@param mob_id_or_name integer|string @mob class id or Aegis name
---@param amount integer @number of mobs to spawn
---@param event_name? string @event label triggered by mob death
---@param size? integer @spawn size override (`SZ_*`)
---@param ai? integer @AI mode override (`AI_*`)
function areamonster(map_name, x0, y0, x1, y1, visible_name, mob_id_or_name, amount, event_name, size, ai) end

--- Kill monster helper.
---@param map_name string @target map to scan
---@param event_name string @event label to match, or `"All"` for non-spawn mobs
---@param keep_event? integer @when `1`, keep event mapping instead of stripping
function killmonster(map_name, event_name, keep_event) end

--- Battleground unbook helper.
---@param queue_name string @battleground queue/map name to unbook
function bg_unbook(queue_name) end

--- Remove mapflag helper.
---@param map_name string @target map name
---@param mapflag integer @mapflag id (`MF_*`)
---@param value? integer @optional payload for valued mapflags
function removemapflag(map_name, mapflag, value) end

--- Area warp helper.
---@param map_name string @source map
---@param x0 integer @source min x
---@param y0 integer @source min y
---@param x1 integer @source max x
---@param y1 integer @source max y
---@param to_map string @destination map (`"Random"` supported)
---@param to_x integer @destination x or random-area min x
---@param to_y integer @destination y or random-area min y
---@param to_x2? integer @destination random-area max x
---@param to_y2? integer @destination random-area max y
function areawarp(map_name, x0, y0, x1, y1, to_map, to_x, to_y, to_x2, to_y2) end

--- Map-wide warp helper.
---@param from_map string @source map name
---@param to_map string @destination map name
---@param x integer @destination x
---@param y integer @destination y
function mapwarp(from_map, to_map, x, y) end

--- Area percent-heal helper.
---@param map_name string @target map name
---@param x0 integer @area min x
---@param y0 integer @area min y
---@param x1 integer @area max x
---@param y1 integer @area max y
---@param hp_percent integer @HP heal percent
---@param sp_percent integer @SP heal percent
function areapercentheal(map_name, x0, y0, x1, y1, hp_percent, sp_percent) end

--- Set map cell helper.
---@param map_name string @target map name
---@param x0 integer @area min x
---@param y0 integer @area min y
---@param x1 integer @area max x
---@param y1 integer @area max y
---@param cell_type integer @cell type constant (`CELL_*`)
---@param enabled integer|boolean @true/1 to set, false/0 to clear
function setcell(map_name, x0, y0, x1, y1, cell_type, enabled) end

--- Set unit data helper.
---@param unit_id integer @target unit gid
---@param data_type integer @field selector (`U*_*` constants)
---@param value integer|string @new field value
function setunitdata(unit_id, data_type, value) end

--- Set unit title helper.
---@param unit_id integer @target unit gid
---@param title string @new displayed title
function setunittitle(unit_id, title) end

--- NPC shop remove item helper.
---@param shop_npc_name string @shop NPC unique name
---@param item_id integer @item id to remove
---@param item_id2? integer @optional item id #2 to remove
---@param item_id3? integer @optional item id #3 to remove
function npcshopdelitem(shop_npc_name, item_id, item_id2, item_id3) end

--- NPC shop update helper.
---@param shop_npc_name string @shop NPC unique name
---@param item_id integer @target shop item id
---@param price integer @new item price (or keep existing when `0`)
---@param stock? integer @market stock value for market shop paths
function npcshopupdate(shop_npc_name, item_id, price, stock) end

--- Move NPC helper.
---@param npc_name string @target NPC unique name
---@param x integer @destination x
---@param y integer @destination y
---@param dir? integer @optional facing direction 0..7
function movenpc(npc_name, x, y, dir) end

--- Set wall helper.
---@param map_name string @map where wall is created
---@param x integer @wall origin x
---@param y integer @wall origin y
---@param size integer @wall length in cells
---@param dir integer @wall direction
---@param shootable integer|boolean @whether projectiles can pass through
---@param wall_name string @unique wall identifier
function setwall(map_name, x, y, size, dir, shootable, wall_name) end

--- Set guild flag emblem helper.
---@param guild_id integer @guild id whose emblem is shown on flag NPC
function flagemblem(guild_id) end

--- Set NPC speed helper.
---@param speed integer @walk speed value (`MIN_WALK_SPEED..MAX_WALK_SPEED`)
---@param npc_name? string @target NPC name (defaults to current NPC)
function npcspeed(speed, npc_name) end

--- Stop unit walk helper.
---@param unit_id integer @target unit gid
---@param flag? integer @stop mode flags (`USW_*`)
function unitstopwalk(unit_id, flag) end

--- Move unit walk helper.
---@param unit_id integer @target unit gid
---@param x integer @destination x
---@param y integer @destination y
---@param done_event? string @optional event label fired on walk completion
---@return integer @non-zero when walk request accepted
function unitwalk(unit_id, x, y, done_event) end

---@type LuaNpcContext
ctx = ctx
