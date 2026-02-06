-- Lua demo: CRUD for database-backed script variables.
-- Focus: character/account/account_global/map_server/npc scopes.

local NPC_NAME = "Lua DB CRUD Demo#lua"

local function key(scope, suffix)
	return "lua_demo_db_" .. scope .. "_" .. suffix
end

local K = {
	char_n = key("char", "n"),
	char_s = key("char", "s"),
	acc_n = key("acc", "n"),
	acc_s = key("acc", "s"),
	accg_n = key("accg", "n"),
	accg_s = key("accg", "s"),
	srv_n = key("srv", "n"),
	srv_s = key("srv", "s"),
	npc_n = key("npc", "n"),
	npc_s = key("npc", "s"),
	acc_arr = key("acc", "arr"),
}

local function title()
	mes("[Lua DB Variable CRUD]")
end

local function val_or_nil(v)
	if v == nil then
		return "nil"
	end
	return tostring(v)
end

local function create_demo_data()
	set_character_var(K.char_n, 1)
	set_character_var(K.char_s, "char_create")

	set_account_var(K.acc_n, 10)
	set_account_var(K.acc_s, "acc_create")

	set_account_global_var(K.accg_n, 100)
	set_account_global_var(K.accg_s, "accg_create")

	set_map_server_var(K.srv_n, 1000)
	set_map_server_var(K.srv_s, "srv_create")

	set_npc_var(K.npc_n, 5000)
	set_npc_var(K.npc_s, "npc_create")

	-- Array-like variable CRUD using indexed access.
	set_account_var(K.acc_arr, 111, 0)
	set_account_var(K.acc_arr, 222, 1)
	set_account_var(K.acc_arr, 333, 2)
end

local function read_demo_data()
	title()
	mes("character n/s = " .. val_or_nil(get_character_var(K.char_n)) .. " / " .. val_or_nil(get_character_var(K.char_s)))
	mes("account n/s = " .. val_or_nil(get_account_var(K.acc_n)) .. " / " .. val_or_nil(get_account_var(K.acc_s)))
	mes("account_global n/s = " .. val_or_nil(get_account_global_var(K.accg_n)) .. " / " .. val_or_nil(get_account_global_var(K.accg_s)))
	mes("map_server n/s = " .. val_or_nil(get_map_server_var(K.srv_n)) .. " / " .. val_or_nil(get_map_server_var(K.srv_s)))
	mes("npc n/s = " .. val_or_nil(get_npc_var(K.npc_n)) .. " / " .. val_or_nil(get_npc_var(K.npc_s)))
	next()
	mes("account array[0..2] = "
		.. val_or_nil(get_account_var(K.acc_arr, 0)) .. ", "
		.. val_or_nil(get_account_var(K.acc_arr, 1)) .. ", "
		.. val_or_nil(get_account_var(K.acc_arr, 2)))
	mes("generic get_var(account, key, 1) = " .. val_or_nil(get_var("account", K.acc_arr, 1)))
	close()
end

local function update_demo_data()
	set_character_var(K.char_n, (get_character_var(K.char_n) or 0) + 1)
	set_character_var(K.char_s, tostring(get_character_var(K.char_s) or "") .. "_u")

	set_account_var(K.acc_n, (get_account_var(K.acc_n) or 0) + 10)
	set_account_var(K.acc_s, tostring(get_account_var(K.acc_s) or "") .. "_u")

	set_account_global_var(K.accg_n, (get_account_global_var(K.accg_n) or 0) + 100)
	set_account_global_var(K.accg_s, tostring(get_account_global_var(K.accg_s) or "") .. "_u")

	-- Generic API update path.
	set_var("map_server", K.srv_n, (get_map_server_var(K.srv_n) or 0) + 1000)
	set_var("map_server", K.srv_s, tostring(get_map_server_var(K.srv_s) or "") .. "_u")

	set_npc_var(K.npc_n, (get_npc_var(K.npc_n) or 0) + 5000)
	set_npc_var(K.npc_s, tostring(get_npc_var(K.npc_s) or "") .. "_u")

	-- Array update.
	set_account_var(K.acc_arr, (get_account_var(K.acc_arr, 1) or 0) + 1, 1)
end

local function delete_demo_data()
	-- Deletion semantics: set value to nil.
	set_character_var(K.char_n, nil)
	set_character_var(K.char_s, nil)
	set_account_var(K.acc_n, nil)
	set_account_var(K.acc_s, nil)
	set_account_global_var(K.accg_n, nil)
	set_account_global_var(K.accg_s, nil)
	set_map_server_var(K.srv_n, nil)
	set_map_server_var(K.srv_s, nil)
	set_npc_var(K.npc_n, nil)
	set_npc_var(K.npc_s, nil)
	set_account_var(K.acc_arr, nil, 0)
	set_account_var(K.acc_arr, nil, 1)
	set_account_var(K.acc_arr, nil, 2)
end

return {
	scripts = {
		{
			map = "prontera",
			x = 162,
			y = 173,
			dir = 4,
			name = NPC_NAME,
			sprite = 1,
			main = function(ctx)


				title()
				mes("This NPC demonstrates variable CRUD across DB scopes.")
				local choice = select(
					"Create demo data",
					"Read demo data",
					"Update demo data",
					"Delete demo data",
					"Create + Read quick run",
					"Cancel"
				)

				if choice == 1 then
					create_demo_data()
					mes("Create done. Use 'Read demo data' next.")
					close()
				elseif choice == 2 then
					read_demo_data()
				elseif choice == 3 then
					update_demo_data()
					mes("Update done. Use 'Read demo data' to verify.")
					close()
				elseif choice == 4 then
					delete_demo_data()
					mes("Delete done. Read should now show nil values.")
					close()
				elseif choice == 5 then
					create_demo_data()
					read_demo_data()
				else
					close()
				end
			end,
			events = {
				OnInit = function(ctx)
					set_map_server_var("lua_demo_db_crud_loaded", 1)
				end,
			},
			labels = {},
		},
	},
}
