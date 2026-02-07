// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef LUA_ENGINE_HPP
#define LUA_ENGINE_HPP

#include <string>
#include <vector>

#include <common/cbasetypes.hpp>

struct lua_State;
class map_session_data;

struct LuaWarpDef {
	std::string map;
	int16 x = 0;
	int16 y = 0;
	int16 dir = 0;
	std::string name;
	std::string type = "warp";
	int16 xs = 0;
	int16 ys = 0;
	std::string to_map;
	int16 to_x = 0;
	int16 to_y = 0;
	std::string state;
};

struct LuaMonsterDef {
	std::string map;
	int16 x = 0;
	int16 y = 0;
	int16 xs = 0;
	int16 ys = 0;
	std::string name;
	int32 mob_id = 0;
	int32 amount = 0;
	uint32 delay1 = 0;
	uint32 delay2 = 0;
	std::string event_name;
	int32 size = -1;
	int32 ai = -1;
	bool boss = false;
};

struct LuaMapflagDef {
	std::string map;
	std::string flag;
	std::string value;
	bool off = false;
};

struct LuaShopDef {
	std::string w1;
	std::string type = "shop";
	std::string name;
	std::string w4;
};

struct LuaDuplicateDef {
	std::string w1;
	std::string source;
	std::string name;
	std::string w4;
};

struct LuaScriptInstance {
	uint32 id = 0;
	uint32 instance_id = 0;
	int32 rid = 0;
	int32 oid = 0;
	std::string event_name;
};

struct LuaNpcDef {
	std::string map;
	int16 x = 0;
	int16 y = 0;
	int16 dir = 0;
	std::string name;
	int32 sprite = -1;
	int16 trigger_x = -1;
	int16 trigger_y = -1;
	std::string state;
	std::string body;
	std::string exname;
	int32 npc_id = 0;
	int32 main_ref = -2; // LUA_NOREF
	struct LuaLabelBinding {
		std::string name;
		int32 ref = -2; // LUA_NOREF
		int32 pos = -1;
	};
	std::vector<LuaLabelBinding> events;
	std::vector<LuaLabelBinding> labels;
};

struct LuaFunctionDef {
	std::string name;
	int32 run_ref = -2; // LUA_NOREF
	std::vector<LuaNpcDef::LuaLabelBinding> labels;
};

bool lua_engine_init(void);
void lua_engine_final(void);
bool lua_engine_load_file(const std::string& filepath);
bool lua_engine_register_npc(const LuaNpcDef& def);
bool lua_engine_register_function(const LuaFunctionDef& def);
bool lua_engine_run_event(const char* event_name, int32 rid, int32 oid);
bool lua_engine_run_npc(int32 npc_id, const char* exname, int32 pos, int32 rid, int32 oid);
void lua_engine_unregister_npc(int32 npc_id);
void lua_engine_clone_npc(int32 source_npc_id, int32 target_npc_id, const char* target_exname);
void lua_engine_clear_npc_registry(void);
void lua_engine_clear_function_registry(void);
bool lua_engine_resume_instance(uint32 instance_id);
bool lua_engine_continue_dialog(map_session_data* sd, int32 npc_id, bool closing);

lua_State* lua_engine_get_state(void);
bool lua_engine_push_loaded_table(void);

#endif /* LUA_ENGINE_HPP */
