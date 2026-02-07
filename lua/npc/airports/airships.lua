-- Gold manual rewrite from DSL: npc/airports/airships.txt
-- Scope: domestic airship route with timer-driven state + touch warp portal.

local LOCATION_KEY = "gold.airship.domestic.location"

local function set_location(env, value)
	env.map_server.set(LOCATION_KEY, value)
end

local function get_location(env)
	return env.map_server.get(LOCATION_KEY) or 0
end

local function route_announce(env, message)
	env.mapannounce("airplane", message, 2)
end

return {
	scripts = {
		{
			map = "airplane",
			x = 243,
			y = 73,
			dir = 0,
			name = "#GoldAirshipWarp-1",
			sprite = 45,
			trigger_x = 1,
			trigger_y = 1,
			events = {
				OnInit = function(ctx)
					specialeffect(5)
					disablenpc(strnpcinfo(0))
				end,
				OnHide = function(ctx)
					specialeffect(5)
					disablenpc(strnpcinfo(0))
				end,
				OnUnhide = function(ctx)
					enablenpc(strnpcinfo(0))
					specialeffect(854)
				end,
				OnTouch_ = function(ctx)
					local location = get_location(_ENV)
					if location == 0 then
						warp("yuno", 92, 260)
					elseif location == 1 then
						warp("einbroch", 92, 278)
					elseif location == 2 then
						warp("lighthalzen", 302, 75)
					elseif location == 3 then
						warp("hugel", 181, 146)
					end
				end,
			},
			labels = {},
		},
		{
			map = "airplane",
			x = 1,
			y = 1,
			dir = 0,
			name = "Gold Domestic Airship",
			sprite = -1,
			events = {
				OnInit = function(ctx)
					set_location(_ENV, 0)
					initnpctimer()
				end,
				OnTimer20000 = function(ctx)
					route_announce(_ENV, "We are heading to Einbroch.")
				end,
				OnTimer50000 = function(ctx)
					route_announce(_ENV, "We will arrive in Einbroch shortly.")
				end,
				OnTimer60000 = function(ctx)
					set_location(_ENV, 1)
					donpcevent("#GoldAirshipWarp-1::OnUnhide")
					donpcevent("#GoldAirshipWarp-2::OnUnhide")
					route_announce(_ENV, "Welcome to Einbroch. Have a safe trip.")
				end,
				OnTimer80000 = function(ctx)
					donpcevent("#GoldAirshipWarp-1::OnHide")
					donpcevent("#GoldAirshipWarp-2::OnHide")
					route_announce(_ENV, "The Airship is now taking off. Our next destination is Lighthalzen.")
				end,
				OnTimer140000 = function(ctx)
					set_location(_ENV, 2)
					donpcevent("#GoldAirshipWarp-1::OnUnhide")
					donpcevent("#GoldAirshipWarp-2::OnUnhide")
					route_announce(_ENV, "Welcome to Lighthalzen. Have a safe trip.")
				end,
				OnTimer160000 = function(ctx)
					donpcevent("#GoldAirshipWarp-1::OnHide")
					donpcevent("#GoldAirshipWarp-2::OnHide")
					route_announce(_ENV, "The Airship is leaving the ground. Our next destination is Einbroch.")
				end,
				OnTimer220000 = function(ctx)
					set_location(_ENV, 1)
					donpcevent("#GoldAirshipWarp-1::OnUnhide")
					donpcevent("#GoldAirshipWarp-2::OnUnhide")
					route_announce(_ENV, "Welcome to Einbroch. Have a safe trip.")
				end,
				OnTimer240000 = function(ctx)
					donpcevent("#GoldAirshipWarp-1::OnHide")
					donpcevent("#GoldAirshipWarp-2::OnHide")
					route_announce(_ENV, "The Airship is now taking off. Our next destination is Juno.")
				end,
				OnTimer300000 = function(ctx)
					set_location(_ENV, 0)
					donpcevent("#GoldAirshipWarp-1::OnUnhide")
					donpcevent("#GoldAirshipWarp-2::OnUnhide")
					route_announce(_ENV, "Welcome to Juno. Have a safe trip.")
				end,
				OnTimer320000 = function(ctx)
					donpcevent("#GoldAirshipWarp-1::OnHide")
					donpcevent("#GoldAirshipWarp-2::OnHide")
					route_announce(_ENV, "The Airship is leaving the ground. Our next destination is Hugel.")
				end,
				OnTimer380000 = function(ctx)
					set_location(_ENV, 3)
					donpcevent("#GoldAirshipWarp-1::OnUnhide")
					donpcevent("#GoldAirshipWarp-2::OnUnhide")
					route_announce(_ENV, "Welcome to Hugel. Have a safe trip.")
				end,
				OnTimer400000 = function(ctx)
					donpcevent("#GoldAirshipWarp-1::OnHide")
					donpcevent("#GoldAirshipWarp-2::OnHide")
					route_announce(_ENV, "The Airship is leaving the ground. Our next destination is Juno.")
				end,
				OnTimer460000 = function(ctx)
					set_location(_ENV, 0)
					donpcevent("#GoldAirshipWarp-1::OnUnhide")
					donpcevent("#GoldAirshipWarp-2::OnUnhide")
					route_announce(_ENV, "Welcome to Juno. Have a safe trip.")
				end,
				OnTimer480000 = function(ctx)
					donpcevent("#GoldAirshipWarp-1::OnHide")
					donpcevent("#GoldAirshipWarp-2::OnHide")
					route_announce(_ENV, "The Airship is leaving the ground. Our next destination is Einbroch.")
					stopnpctimer()
					initnpctimer()
				end,
			},
			labels = {},
		},
		{
			map = "airplane",
			x = 100,
			y = 69,
			dir = 3,
			name = "Gold Airship Crew#ein-1",
			sprite = 852,
			main = function(ctx)
				mes("[Airship Crew]")
				mes("If we've landed at your destination,")
				mes("please use the stairs to leave the airship.")
				close()
			end,
			events = {},
			labels = {},
		},
	},
	duplicates = {
		{
			map = "airplane",
			x = 243,
			y = 29,
			dir = 0,
			source = "#GoldAirshipWarp-1",
			name = "#GoldAirshipWarp-2",
			view = "45",
		},
	},
}
