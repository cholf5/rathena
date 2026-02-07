-- Gold manual rewrite from DSL: npc/quests/quests_alberta.txt
-- Scope: Turtle Island sailor flow (dialog + zeny fee + warp + RE/PRE branch).

local TURTLE_FLAG = "TURTLE"
local MISC_QUEST_FLAG = "MISC_QUEST"
local TURTLE_MISC_MASK = 65536

local function has_turtle_quest_context(env)
	local turtle = env.character.get(TURTLE_FLAG) or 0
	local misc = env.character.get(MISC_QUEST_FLAG) or 0
	return turtle ~= 0 or (misc & TURTLE_MISC_MASK) ~= 0
end

local function read_zeny(env)
	return env.get_var("character", "Zeny") or 0
end

local function write_zeny(env, value)
	env.set_var("character", "Zeny", value)
end

return {
	scripts = {
		{
			map = "alberta",
			x = 246,
			y = 113,
			dir = 4,
			name = "Gold Gotanblue#tur",
			sprite = 709,
			main = function(ctx)


				if not has_turtle_quest_context(_ENV) then
					mes("[Gotanblue]")
					mes("Just look at that ocean...")
					mes("Tell me that's not one of the most beautiful sights you've seen.")
					close()
					return
				end

				mes("[Gotanblue]")
				mes("I can tell you're curious about Turtle Island.")
				next()
				local choice = select("Do you know about Turtle Island?:How can I get there?:Stop talking")

				if choice == 1 then
					mes("[Gotanblue]")
					mes("I was one of Jornadan Niliria's crew.")
					mes("We reached Turtle Island through dense fog and paid a huge price.")
					next()
					mes("[Gotanblue]")
					mes("If you want deeper details, talk to the scholar on Alberta's eastern port.")
					close()
					return
				end

				if choice == 2 then
					mes("[Gotanblue]")
					mes("I can guide you there safely for 10,000 zeny.")
					next()
					if select("Turtle island -> 10000 zeny:Cancel") == 1 then
						local zeny = read_zeny(_ENV)
						if zeny >= 10000 then
							mes("[Gotanblue]")
							mes("All right, hold tight. We're heading out.")
							write_zeny(_ENV, zeny - 10000)
							warp("tur_dun01", 157, 39)
							close()
							return
						end
						mes("[Gotanblue]")
						mes("You don't have enough zeny right now.")
						close()
						return
					end
					mes("[Gotanblue]")
					mes("Come back when you're ready for the trip.")
					close()
					return
				end

				mes("[Gotanblue]")
				mes("Come back whenever you're ready to hear more.")
				close()
			end,
			events = {},
			labels = {},
		},
		{
			map = "tur_dun01",
			x = 165,
			y = 29,
			dir = 4,
			name = "Gold Sailor#tur2",
			sprite = 709,
			main = function(ctx)
				mes("[Gotanblue]")
				mes("Do you want to return to Alberta?")
				next()
				if select("Go to Alberta:Stop talking") == 1 then
					mes("[Gotanblue]")
					mes("All right, let's head back.")
					if checkre(0) ~= 0 then
						warp("alberta", 238, 112)
					else
						warp("alberta", 241, 115)
					end
					close()
					return
				end
				close()
			end,
			events = {},
			labels = {},
		},
	},
}
