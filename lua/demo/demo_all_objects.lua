-- Lua demo: all top-level object types in one file.

return {
	warps = {
		{
			map = "prontera",
			x = 156,
			y = 175,
			dir = 0,
			name = "lua_demo_warp_prt_center",
			xs = 1,
			ys = 1,
			to_map = "prontera",
			to_x = 160,
			to_y = 180,
		},
	},
	monsters = {
		{
			map = "guild_vs1",
			x = 0,
			y = 0,
			xs = 0,
			ys = 0,
			name = "Lua Demo Poring",
			mob_id = 1002,
			amount = 1,
			delay1 = 60000,
			delay2 = 60000,
		},
	},
	mapflags = {
		{
			map = "guild_vs1",
			flag = "nosave",
			value = "SavePoint",
		},
	},
	shops = {
		{
			map = "prontera",
			x = 158,
			y = 175,
			dir = 0,
			type = "shop",
			name = "Lua Demo Shop",
			sprite = "1_M_MERCHANT",
			items = "501:2,502:10,503:100",
		},
	},
	scripts = {
		{
			map = "prontera",
			x = 154,
			y = 175,
			dir = 4,
			name = "Lua Demo Simple#lua",
			sprite = "1_M_JOBGUIDER",
			main = function(ctx)
				mes("[Lua Demo: Simple]")
				mes("This file demonstrates every top-level Lua object type.")
				mes("Try talking to the medium and advanced demo NPCs too.")
				next()
				mes(callfunc("LUA_DEMO_HELLO"))
				close()
			end,
			events = {
				OnInit = function(ctx)
					map_server.set("lua_demo_all_objects_loaded", 1)
				end,
			},
			labels = {},
		},
	},
	duplicates = {
		{
			map = "prontera",
			x = 152,
			y = 175,
			dir = 4,
			source = "Lua Demo Simple#lua",
			name = "Lua Demo Simple Copy#lua",
			view = "1_M_MERCHANT",
		},
	},
	functions = {
		{
			name = "LUA_DEMO_HELLO",
			run = function(ctx, ...)
				return "Hello from functions[] in demo_all_objects.lua"
			end,
			labels = {},
		},
	},
}
