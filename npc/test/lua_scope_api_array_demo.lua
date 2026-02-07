return {
	warps = {},
	monsters = {},
	mapflags = {},
	shops = {},
	duplicates = {},
	scripts = {
		{
			line = 1,
			w1 = "prontera,160,182,4",
			w2 = "script",
			w3 = "Lua Scope Array Demo",
			w4 = "1_M_MERCHANT,",
			main = function(ctx)
				local _ENV = ctx:env()
				local slot = 1
				local seen = account.get("lua_scope_array_seen", slot) or 0
				seen = seen + 1
				account.set("lua_scope_array_seen", seen, slot)

				local legacy_seen = get_var("account", "lua_scope_array_seen", slot) or 0

				mes("[Lua Scope Array Demo]")
				mes("Account slot " .. slot .. " seen count: " .. seen)
				mes("Generic API readback: " .. legacy_seen)
				close()
			end,
			events = {},
			labels = {},
		},
	},
	functions = {},
}
