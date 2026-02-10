-- Lua demo: advanced script style.
-- Focus: structured data, closures, pcall, sorting, deterministic pseudo-random behavior,
-- and composition through global callfunc helpers.

local function mk_counter(scope, key)
	return function(env, delta)
		local current = env.var.get(scope, key) or 0
		local next_value = current + (delta or 1)
		env.var.set(scope, key, next_value)
		return next_value
	end
end

local function stable_shuffle(list, seed)
	local out = {}
	for i = 1, #list do
		out[i] = list[i]
	end

	local x = math.abs(tonumber(seed) or 1)
	for i = #out, 2, -1 do
		x = (1103515245 * x + 12345) % 2147483647
		local j = (x % i) + 1
		out[i], out[j] = out[j], out[i]
	end
	return out
end

local function build_menu(options)
	local labels = {}
	for i = 1, #options do
		labels[#labels + 1] = options[i].label
	end
	return table.concat(labels, ":")
end

local tips = {
	{ label = "Explain advanced features", fn = function(env)
		env.mes("This demo uses closures, deterministic shuffle, pcall, and helper composition.")
		env.mes("It also keeps state in scoped storage APIs instead of legacy prefixed names.")
	end },
	{ label = "Run safe arithmetic test", fn = function(env)
		local ok, result = pcall(function()
			local a = 42
			local b = 7
			return a / b
		end)
		env.mes(ok and ("Safe arithmetic result: " .. tostring(result)) or "Arithmetic test failed.")
	end },
	{ label = "Show rotating tip set", fn = function(env)
		local base = {
			"Use local helpers for readability.",
			"Keep NPC state in explicit scopes.",
			"Use small composable functions.",
			"Prefer table-driven branching when menus grow.",
		}
		local seed = (env.character.get("lua_demo_adv_seed") or 1) + 17
		env.character.set("lua_demo_adv_seed", seed)
		local shuffled = stable_shuffle(base, seed)
		env.mes("Tip 1: " .. shuffled[1])
		env.mes("Tip 2: " .. shuffled[2])
	end },
	{ label = "Timed message (sleep)", fn = function(env)
		env.mes("Pausing briefly...")
		env.sleep(300)
		env.mes("Resumed after sleep(300).")
	end },
	{ label = "Leave", fn = function(env)
		env.mes("Advanced demo finished.")
	end },
}

local inc_session_counter = mk_counter("character_temp", "lua_demo_advanced_visits")

return {
	warps = {},
	monsters = {},
	mapflags = {},
	shops = {},
	duplicates = {},
	scripts = {
		{
			map = "prontera",
			x = 148,
			y = 175,
			dir = 4,
			name = "Lua Demo Advanced#lua",
			sprite = 84,
			main = function(ctx)
				local visits = inc_session_counter(_ENV, 1)

				mes("[Lua Demo: Advanced]")
				mes("Session visits: " .. tostring(visits))
				next()

				local sorted = {}
				for i = 1, #tips do
					sorted[i] = tips[i]
				end
				table.sort(sorted, function(a, b)
					return a.label < b.label
				end)

				local menu = build_menu(sorted)
				local picked = select(menu)
				local item = sorted[picked]
				if item and item.fn then
					item.fn(_ENV)
				end

				if picked and picked < #sorted then
					next()
					local score = callfunc("LUA_DEMO_ADV_SCORE", visits, picked) or 0
					mes(callfunc("LUA_DEMO_ADV_FORMAT", score))
				end

				close()
			end,
			events = {
				OnInit = function(ctx)
					map_server.set("lua_demo_advanced_ready", 1)
				end,
			},
			labels = {},
		},
	},
	functions = {
		{
			name = "LUA_DEMO_ADV_SCORE",
			run = function(ctx, ...)
				local visits = getarg(0, 0)
				local branch = getarg(1, 0)
				return visits * 10 + branch
			end,
			labels = {},
		},
		{
			name = "LUA_DEMO_ADV_FORMAT",
			run = function(ctx, ...)
				local score = getarg(0, 0)
				return string.format("Computed demo score: %d", score)
			end,
			labels = {},
		},
	},
}
