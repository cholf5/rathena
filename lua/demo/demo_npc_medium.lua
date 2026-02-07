-- Lua demo: medium complexity NPC script.
-- Focus: if/else branches, menu flow, scope APIs, helper functions.

local function read_visit_count(env)
	return env.character_temp.get("lua_demo_medium_visits") or 0
end

local function write_visit_count(env, value)
	env.character_temp.set("lua_demo_medium_visits", value)
end

return {
	warps = {},
	monsters = {},
	mapflags = {},
	shops = {},
	duplicates = {},
	scripts = {
		{
			map = "prontera",
			x = 150,
			y = 175,
			dir = 4,
			name = "Lua Demo Medium#lua",
			sprite = 1,
			main = function(ctx)
				local visits = read_visit_count(_ENV) + 1
				write_visit_count(_ENV, visits)

				mes("[Lua Demo: Medium]")
				mes("Session visits: " .. tostring(visits))
				next()

				local choice = select("What can you do?:Show scoped vars:Reset my session count:Leave")
				if choice == 1 then
					mes("I use plain Lua control flow with helper functions.")
					mes("No DSL-style tuple fields and no legacy variable prefix syntax required.")
				elseif choice == 2 then
					local char_name = strcharinfo(0)
					local account_seen = account.get("lua_demo_medium_seen") or 0
					account.set("lua_demo_medium_seen", account_seen + 1)
					mes("Character: " .. tostring(char_name))
					mes("Account visits: " .. tostring(account_seen + 1))
					mes("Map-server flag: " .. tostring(map_server.get("lua_demo_all_objects_loaded") or 0))
				elseif choice == 3 then
					write_visit_count(_ENV, 0)
					mes("Session visit counter reset.")
				else
					mes("See you next time.")
				end

				close()
			end,
			events = {},
			labels = {},
		},
	},
	functions = {},
}
