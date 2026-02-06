return {
	warps = {},
	monsters = {},
	mapflags = {},
	shops = {},
	duplicates = {},
	scripts = {
		{
			line = 1,
			w1 = "prontera,158,182,4",
			w2 = "script",
			w3 = "Lua Scope Demo",
			w4 = "1_M_MERCHANT,",
			main = function(ctx)
				local _ENV = ctx:env()
				local player_name = strcharinfo(0)

				local session_visits = character_temp.get("lua_scope_demo_visits") or 0
				session_visits = session_visits + 1
				character_temp.set("lua_scope_demo_visits", session_visits)

				local server_visits = map_server.get("lua_scope_demo_total") or 0
				server_visits = server_visits + 1
				map_server.set("lua_scope_demo_total", server_visits)

				mes("[Lua Scope Demo]")
				mes("Hello, " .. player_name .. ".")
				mes("Character temp visits: " .. session_visits)
				mes("Map-server total visits: " .. server_visits)
				close()
			end,
			events = {},
			labels = {},
		},
	},
	functions = {},
}
