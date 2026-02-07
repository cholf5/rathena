-- Gold manual rewrite from DSL: npc/other/monster_race.txt
-- Scope: single-race timer flow + ticket helper + runner control.

local function ms_key(prefix, idx)
	return string.format("gold.mr.single.%s.%d", prefix, idx)
end

local function get_luk(env, idx)
	return env.map_server.get(ms_key("luk", idx)) or 0
end

local function set_luk(env, idx, value)
	env.map_server.set(ms_key("luk", idx), value)
end

local function get_tire(env, idx)
	return env.map_server.get(ms_key("tire", idx)) or 0
end

local function set_tire(env, idx, value)
	env.map_server.set(ms_key("tire", idx), value)
end

local function set_race_open_state(env, state)
	env.map_server.set("gold.mr.single.open_state", state)
end

local function get_race_open_state(env)
	return env.map_server.get("gold.mr.single.open_state") or 0
end

local function suffix(n)
	if n == 1 then
		return "1st"
	elseif n == 2 then
		return "2nd"
	elseif n == 3 then
		return "3rd"
	end
	return tostring(n) .. "th"
end

local function runner_spawn_y(runner_id)
	local ys = { 38, 36, 34, 32, 30, 28 }
	return ys[runner_id] or 38
end

return {
	scripts = {
		{
			map = "hugel",
			x = 51,
			y = 61,
			dir = 0,
			name = "#gold_race_timer_single",
			sprite = -1,
			events = {
				OnEnable = function(ctx)
					enablenpc("#gold_race_timer_single")
				end,
				OnInit = function(ctx)
					set_race_open_state(_ENV, 2)
					initnpctimer()
				end,
				OnTimer10000 = function(ctx)
					mapannounce("hugel", "The Single Monster Race will soon begin. We hope to see many of you participate!", 2)
				end,
				OnTimer30000 = function(ctx)
					mapannounce("hugel", "The Single Monster Race Arena has just opened.", 2)
					set_race_open_state(_ENV, 1)
					donpcevent("Gold Race Progress Timer::OnEnable")
				end,
				OnTimer90000 = function(ctx)
					mapannounce("hugel", "The Single Monster Race arena is now open. Participants should enter the Arena as soon as they can.", 2)
				end,
				OnTimer210000 = function(ctx)
					mapannounce("hugel", "The entrance to the Single Monster Race Arena will close shortly. Participants, please enter the arena now.", 2)
				end,
				OnTimer270000 = function(ctx)
					mapannounce("hugel", "The Single Monster Race Arena's entrance will soon close.", 2)
				end,
				OnTimer272000 = function(ctx)
					mapannounce("hugel", "Participants, please enter the Arena before the doors close.", 2)
				end,
				OnTimer330000 = function(ctx)
					mapannounce("hugel", "The race is now starting. If you missed your chance to enter this race, please try again next time~!", 2)
					set_race_open_state(_ENV, 0)
					disablenpc("#gold_race_timer_single")
					stopnpctimer()
				end,
			},
			labels = {},
		},
		{
			map = "p_track01",
			x = 58,
			y = 0,
			dir = 0,
			name = "Gold Race Progress Timer",
			sprite = -1,
			events = {
				OnEnable = function(ctx)
					initnpctimer()
					enablenpc("Gold Race Progress Timer")
					for i = 1, 6 do
						local line = rand(1, 70)
						enablenpc("Gold Runner Start#" .. i)
						enablenpc("Gold Runner Tire" .. i .. "#1")

						local tired = 0
						if line <= 10 then
							tired = rand(50, 60)
							enablenpc("Gold Runner Luck" .. i .. "#5")
							enablenpc("Gold Runner Luck" .. i .. "#6")
						elseif line <= 30 then
							tired = rand(40, 60)
							enablenpc("Gold Runner Luck" .. i .. "#5")
							if tired >= 50 then
								enablenpc("Gold Runner Tire" .. i .. "#2")
							end
						elseif line <= 40 then
							tired = rand(30, 50)
							enablenpc("Gold Runner Luck" .. i .. "#1")
							enablenpc("Gold Runner Tire" .. i .. "#2")
							if tired < 40 then
								enablenpc("Gold Runner Tire" .. i .. "#3")
							end
						elseif line <= 50 then
							tired = rand(20, 40)
							enablenpc("Gold Runner Luck" .. i .. "#1")
							enablenpc("Gold Runner Luck" .. i .. "#2")
							enablenpc("Gold Runner Tire" .. i .. "#2")
							enablenpc("Gold Runner Tire" .. i .. "#3")
							if tired < 30 then
								enablenpc("Gold Runner Tire" .. i .. "#4")
							end
						elseif line <= 60 then
							tired = rand(10, 30)
							for j = 1, 3 do
								enablenpc("Gold Runner Luck" .. i .. "#" .. j)
							end
							for j = 2, 4 do
								enablenpc("Gold Runner Tire" .. i .. "#" .. j)
							end
							if tired < 20 then
								enablenpc("Gold Runner Tire" .. i .. "#5")
							end
						else
							tired = rand(0, 20)
							for j = 1, 4 do
								enablenpc("Gold Runner Luck" .. i .. "#" .. j)
							end
							for j = 2, 5 do
								enablenpc("Gold Runner Tire" .. i .. "#" .. j)
							end
							if tired < 10 then
								enablenpc("Gold Runner Tire" .. i .. "#6")
							end
						end

						set_luk(_ENV, i, line)
						set_tire(_ENV, i, tired)
					end
				end,
				OnTimer1000 = function(ctx)
					enablenpc("Gold Ticket Helper#single")
				end,
				OnTimer7000 = function(ctx)
					mapannounce("p_track01", "Welcome to the Monster Race Arena.", 2)
				end,
				OnTimer120000 = function(ctx)
					mapannounce("p_track01", "The Single Monster Race will start in 3 minutes.", 2)
				end,
				OnTimer240000 = function(ctx)
					mapannounce("p_track01", "The Single Monster Race will start shortly.", 2)
				end,
				OnTimer300000 = function(ctx)
					mapannounce("p_track01", "The Monster Race has already begun. Good luck to all the participants.", 2)
					map_server.set("gold.mr.single.winner", 0)
					disablenpc("Gold Ticket Helper#single")
					for i = 1, 6 do
						donpcevent("Gold Runner No. " .. i .. "#" .. i .. "::OnEnable")
					end
					stopnpctimer()
				end,
				OnInit = function(ctx)
					disablenpc("Gold Race Progress Timer")
				end,
				OnDisable = function(ctx)
					disablenpc("Gold Race Progress Timer")
				end,
			},
			labels = {},
		},
		{
			map = "p_track01",
			x = 73,
			y = 22,
			dir = 1,
			name = "Gold Ticket Helper#single",
			sprite = 899,
			main = function(ctx)
				mes("[Ticket Helper]")
				mes("Welcome to the Monster Race Arena.")
				mes("Pick one of six monsters for the single race.")
				next()

				if checkweight("Spawn", 200) == 0 then
					mes("[Ticket Helper]")
					mes("I can't issue your betting ticket right now.")
					mes("Please reduce your carried items first.")
					close()
					return
				end

				local pick = select("Monster Status", "Monster 1", "Monster 2", "Monster 3", "Monster 4", "Monster 5", "Monster 6") - 1
				if pick == 0 then
					for i = 1, 6 do
						mes(string.format("Monster %d [Luck: %d] [HP: %d]", i, get_luk(_ENV, i), get_tire(_ENV, i)))
					end
					close()
					return
				end

				if get_race_open_state(_ENV) ~= 1 then
					mes("[Ticket Helper]")
					mes("A race is currently in progress.")
					mes("Please wait for the next round.")
					close()
					return
				end

				local chosen = character.get("gold_mr_single_pick") or 0
				if chosen > 0 then
					mes("[Ticket Helper]")
					mes("You already selected Monster " .. tostring(chosen) .. " for this round.")
					close()
					return
				end

				mes("[Ticket Helper]")
				mes("You've chosen Monster " .. tostring(pick) .. ".")
				mes("Please hold this ticket until race results are announced.")
				getitem(7514, 1)
				character.set("gold_mr_single_pick", pick)
				close()
			end,
			events = {
				OnInit = function(ctx)
					disablenpc("Gold Ticket Helper#single")
				end,
			},
			labels = {},
		},
		{
			map = "-",
			x = 0,
			y = 0,
			dir = 0,
			name = "Gold Runner Main",
			sprite = -1,
			events = {
				OnEnable = function(ctx)
					emotion(3)
					enablenpc(strnpcinfo(0))

					local runner_id = atoi(strnpcinfo(2))
					local mob_ids = { 1725, 1726, 1727, 1728, 1730, 1729 }
					monster(
						"p_track01",
						58,
						runner_spawn_y(runner_id),
						"The " .. suffix(runner_id) .. " Racer",
						mob_ids[runner_id],
						1,
						strnpcinfo(0) .. "::OnMyMobDead"
					)
				end,
				OnDisable = function(ctx)
					disablenpc(strnpcinfo(0))
					killmonster("p_track01", strnpcinfo(0) .. "::OnMyMobDead")
				end,
				OnTouchNPC = function(ctx)
					local winner = atoi(strnpcinfo(2))
					map_server.set("gold.mr.single.winner", winner)

					for i = 1, 6 do
						if i ~= winner then
							donpcevent("Gold Runner No. " .. i .. "#" .. i .. "::OnDisable")
						end
					end

					sleep(1000)
					mapannounce("p_track01", "We have a winner...!", 2)
					sleep(1000)
					mapannounce("p_track01", "Monster " .. tostring(winner) .. " is the winner of this race!", 2)
					sleep(2000)
					mapannounce("p_track01", "If you wagered on Monster " .. tostring(winner) .. ", redeem your prize now!", 2)
					donpcevent(strnpcinfo(0) .. "::OnDisable")
					killmonster("p_track01", strnpcinfo(0) .. "::OnMyMobDead")
				end,
				OnInit = function(ctx)
					if strnpcinfo(2) ~= "" then
						disablenpc(strnpcinfo(0))
					end
				end,
			},
			labels = {},
		},
	},
	duplicates = {
		{ map = "p_track01", x = 30, y = 38, dir = 0, source = "Gold Runner Main", name = "Gold Runner No. 1#1", view = "-1" },
		{ map = "p_track01", x = 30, y = 36, dir = 0, source = "Gold Runner Main", name = "Gold Runner No. 2#2", view = "-1" },
		{ map = "p_track01", x = 30, y = 34, dir = 0, source = "Gold Runner Main", name = "Gold Runner No. 3#3", view = "-1" },
		{ map = "p_track01", x = 30, y = 32, dir = 0, source = "Gold Runner Main", name = "Gold Runner No. 4#4", view = "-1" },
		{ map = "p_track01", x = 30, y = 30, dir = 0, source = "Gold Runner Main", name = "Gold Runner No. 5#5", view = "-1" },
		{ map = "p_track01", x = 30, y = 28, dir = 0, source = "Gold Runner Main", name = "Gold Runner No. 6#6", view = "-1" },
	},
}
