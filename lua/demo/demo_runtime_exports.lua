-- Lua demo: runtime-exported compatibility helpers.
-- WARNING: This file intentionally exercises side-effectful commands.
-- Use only on a test server.

local DEMO_NPC = "Lua Runtime APIs#lua"
local DEMO_WALKER = "Lua Demo Walker#lua"
local DEMO_UNLOAD_TARGET = "Lua Demo Unload Target#lua"
local DEMO_SHOP = "Lua Demo Runtime Shop"

local function title()
	mes("[Lua Runtime API Demo]")
end

local function pause()
	next()
end

local function confirm_dangerous()
	mes("This action can affect nearby players/NPC state.")
	return select("Run it on this test server:Cancel") == 1
end

local function demo_visual_and_info()
	title()
	mes("Running cutin/specialeffect/emotion/npctalk/dispbottom helpers.")
	pause()

	cutin("", 255)
	specialeffect(1)
	specialeffect2(2)
	emotion(0)
	npctalk("Runtime API demo message.", DEMO_NPC, 2, 0x66FFCC)
	dispbottom("Runtime API demo: visual helpers executed.", 0x66FFCC)

	local n = atoi("12345")
	local mob_lv = getmonsterinfo(1002, 1)
	local npc_gid = getnpcid(0)
	local re_mode = checkre()

	title()
	mes(string.format("atoi('12345') = %d", n))
	mes(string.format("getmonsterinfo(1002, MOB_LV) = %s", tostring(mob_lv)))
	mes(string.format("getnpcid(0) = %d", npc_gid))
	mes(string.format("checkre() = %d", re_mode))
	close()
end

local function demo_inventory_and_status()
	title()
	mes("Running getitem/delitem/checkweight/sc_start2.")
	pause()

	getitem(501, 1)
	local apples = countitem(501)
	local can_hold = checkweight(501, 100)
	-- Keep effect id conservative for demo stability.
	sc_start2(0, 1000, 1, 0)
	delitem(501, 1)

	title()
	mes(string.format("countitem(501) right after getitem = %d", apples))
	mes(string.format("checkweight(501,100) = %d", can_hold))
	mes("sc_start2(0, ...) invoked for API smoke test.")
	close()
end

local function demo_spawn_and_broadcast()
	title()
	mes("Spawning and killing demo mobs on guild_vs1.")
	pause()

	monster("guild_vs1", 50, 50, "Lua Demo Poring", 1002, 1)
	areamonster("guild_vs1", 45, 45, 55, 55, "Lua Area Poring", 1002, 1)
	killmonster("guild_vs1", "All", 1)
	mapannounce("prontera", "[Lua Demo] mapannounce() called", 0)
	announce("[Lua Demo] announce() called", 0)

	title()
	mes("monster/areamonster/killmonster/mapannounce/announce executed.")
	close()
end

local function demo_map_and_area_ops()
	title()
	mes("This section uses areawarp/mapwarp/setcell/removemapflag/areapercentheal.")
	if not confirm_dangerous() then
		close()
		return
	end

	setcell("guild_vs1", 10, 10, 12, 12, 1, 1)
	removemapflag("guild_vs1", 1, 0)
	areapercentheal("prontera", 145, 165, 170, 190, 1, 1)
	-- Narrow test area around this NPC.
	areawarp("prontera", 145, 165, 170, 190, "prontera", 156, 180)
	mapwarp("guild_vs1", "prontera", 156, 180)

	title()
	mes("Map/area helpers executed.")
	close()
end

local function demo_npc_and_unit_ops()
	title()
	mes("Running NPC visibility/move/unit helpers.")
	pause()

	hideonnpc(DEMO_UNLOAD_TARGET)
	hideoffnpc(DEMO_UNLOAD_TARGET)
	cloakonnpc(DEMO_UNLOAD_TARGET)
	cloakoffnpc(DEMO_UNLOAD_TARGET)
	npcspeed(180, DEMO_WALKER)
	movenpc(DEMO_WALKER, 165, 176, 2)
	setunittitle(getnpcid(0), "[Lua Runtime]")
	-- Data type is intentionally generic for smoke-path coverage.
	setunitdata(getnpcid(0), 13, 0)
	unitwalk(getnpcid(0), 160, 172)
	unitstopwalk(getnpcid(0), 0)
	flagemblem(0)

	title()
	mes("NPC/unit helpers executed.")
	close()
end

local function demo_shop_waitingroom_timer()
	title()
	mes("Running waitingroom/shop/timer/bg_unbook/questinfo helpers.")
	pause()

	waitingroom("Lua Runtime Queue", 5, "", 5, 0, 1, 200)
	enablewaitingroomevent()
	disablewaitingroomevent()
	npcshopupdate(DEMO_SHOP, 501, 2)
	npcshopdelitem(DEMO_SHOP, 503)
	setnpctimer(0)
	startnpctimer()
	stopnpctimer()
	bg_unbook("guild_vs1")
	questinfo(1, 0)

	title()
	mes("waitingroom/shop/timer/bg_unbook/questinfo executed.")
	close()
end

local function demo_unloadnpc()
	title()
	mes("This will unload '" .. DEMO_UNLOAD_TARGET .. "'.")
	if not confirm_dangerous() then
		close()
		return
	end
	unloadnpc(DEMO_UNLOAD_TARGET)
	mes("Unload command executed.")
	close()
end

return {
	shops = {
		{
			map = "prontera",
			x = 161,
			y = 173,
			dir = 0,
			type = "shop",
			name = DEMO_SHOP,
			sprite = "1_M_MERCHANT",
			items = "501:2,502:10,503:100",
		},
	},
	scripts = {
		{
			map = "prontera",
			x = 160,
			y = 173,
			dir = 4,
			name = DEMO_NPC,
			sprite = "1_M_JOBGUIDER",
			main = function(ctx)


				title()
				mes("Select a runtime API group to test.")
				local choice = select(
					"Visual + Info",
					"Inventory + Status",
					"Spawn + Broadcast",
					"Map + Area Ops",
					"NPC + Unit Ops",
					"Shop + Waiting + Timer",
					"Unload NPC (dangerous)",
					"Cancel"
				)

				if choice == 1 then
					demo_visual_and_info()
				elseif choice == 2 then
					demo_inventory_and_status()
				elseif choice == 3 then
					demo_spawn_and_broadcast()
				elseif choice == 4 then
					demo_map_and_area_ops()
				elseif choice == 5 then
					demo_npc_and_unit_ops()
				elseif choice == 6 then
					demo_shop_waitingroom_timer()
				elseif choice == 7 then
					demo_unloadnpc()
				else
					close()
				end
			end,
			events = {
				OnInit = function(ctx)
					map_server.set("lua_demo_runtime_exports_loaded", 1)
				end,
			},
			labels = {},
		},
		{
			map = "prontera",
			x = 165,
			y = 173,
			dir = 2,
			name = DEMO_WALKER,
			sprite = "4_M_HUMAN_01",
			main = function(ctx)
				mes("[Lua Demo Walker]")
				mes("Used by demo_runtime_exports.lua for movenpc/unitwalk tests.")
				close()
			end,
			events = {},
			labels = {},
		},
		{
			map = "prontera",
			x = 166,
			y = 173,
			dir = 2,
			name = DEMO_UNLOAD_TARGET,
			sprite = "4_F_TELEPORTER",
			main = function(ctx)
				mes("[Lua Demo Unload Target]")
				mes("This NPC is intentionally used by unloadnpc() demo.")
				close()
			end,
			events = {},
			labels = {},
		},
	},
}
