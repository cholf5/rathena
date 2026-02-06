// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "lua_engine.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <unordered_map>
#include <utility>
#include <vector>

#include <common/showmsg.hpp>
#include <common/timer.hpp>

#include "map.hpp"
#include "battle.hpp"
#include "battleground.hpp"
#include "chat.hpp"
#include "guild.hpp"
#include "intif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "mob.hpp"
#include "npc.hpp"
#include "pc.hpp"
#include "pet.hpp"
#include "script.hpp"
#include "status.hpp"
#include "unit.hpp"
#include "clif.hpp"

#if defined(HAVE_LUA) || defined(WITH_LUA)
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
#endif

namespace {
#if defined(HAVE_LUA) || defined(WITH_LUA)
struct LuaVariable {
	enum class Type {
		Number,
		String,
		Boolean,
		Table,
	};

	Type type = Type::Number;
	double number = 0.0;
	std::string string;
	bool boolean = false;
	int32 table_ref = LUA_NOREF;
};

struct LuaNpcRuntime {
	int32 npc_id = 0;
	std::string exname;
	int32 main_ref = LUA_NOREF;
	std::unordered_map<std::string, int32> event_refs;
	std::unordered_map<std::string, int32> label_refs;
	std::unordered_map<int32, std::string> pos_to_label;
};

struct LuaFunctionRuntime {
	std::string name;
	int32 run_ref = LUA_NOREF;
	std::unordered_map<std::string, int32> label_refs;
};

enum class LuaWaitType : uint8 {
	None = 0,
	Next,
	Menu,
	Input,
	Sleep,
	Close2,
	CloseEnd,
};

struct LuaExecutionContext {
	int32 npc_id = 0;
	int32 rid = 0;
	int32 oid = 0;
	int32 ctx_ref = LUA_NOREF;
	std::string function_name;
	std::vector<int32> arg_refs;
	bool mes_active = false;
	LuaWaitType wait_type = LuaWaitType::None;
	bool clear_cutin_on_close = false;
	std::string input_var;
	bool input_is_string = false;
	int32 input_min = 0;
	int32 input_max = 0;
	int32 sleep_tick = 0;
	int32 backup_npc_id = 0;
	bool backup_disable_atcommand = false;
	bool attached_player = false;
};

struct LuaRuntimeInstance {
	uint32 id = 0;
	int32 thread_ref = LUA_NOREF;
	int32 timer_id = INVALID_TIMER;
	std::string runtime_name;
	LuaExecutionContext execution;
};

lua_State* g_lua_state = nullptr;
int32 g_loaded_table_ref = LUA_NOREF;
int32 g_builtin_table_ref = LUA_NOREF;
std::unordered_map<int32, LuaNpcRuntime> g_npc_registry;
std::unordered_map<std::string, int32> g_exname_registry;
std::unordered_map<std::string, LuaFunctionRuntime> g_function_registry;
std::unordered_map<std::string, LuaVariable> g_var_registry;
std::vector<LuaExecutionContext> g_execution_stack;
std::unordered_map<uint32, LuaRuntimeInstance> g_instances;
std::unordered_map<uint64, uint32> g_pending_dialog_instances;
uint32 g_next_instance_id = 1;
bool g_timer_registered = false;

TIMER_FUNC(lua_instance_resume_timer);
LuaExecutionContext* lua_current_execution(void);
void lua_store_variable(const std::string& key, lua_State* L, int index);
int32 lua_env_index(lua_State* L);
int32 lua_env_newindex(lua_State* L);

std::string lua_normalize_key(const std::string& in) {
	std::string key = in;
	std::transform(key.begin(), key.end(), key.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});
	return key;
}

bool lua_is_local_temp_var(const std::string& key) {
	return key.size() >= 2 && key[0] == '.' && key[1] == '@';
}

std::string lua_scoped_var_key(const std::string& key) {
	if (!lua_is_local_temp_var(key)) {
		return key;
	}
	LuaExecutionContext* execution = lua_current_execution();
	if (execution == nullptr || execution->ctx_ref == LUA_NOREF) {
		return key;
	}
	return "__ctx:" + std::to_string(execution->ctx_ref) + ":" + key;
}

bool lua_has_variable_exact(const std::string& key) {
	const std::string scoped_key = lua_scoped_var_key(key);
	return g_var_registry.find(scoped_key) != g_var_registry.end();
}

bool lua_push_variable_exact(const std::string& key, lua_State* L) {
	const std::string scoped_key = lua_scoped_var_key(key);
	auto it = g_var_registry.find(scoped_key);
	if (it == g_var_registry.end()) {
		return false;
	}

	const LuaVariable& var = it->second;
	switch (var.type) {
		case LuaVariable::Type::String:
			lua_pushstring(L, var.string.c_str());
			break;
		case LuaVariable::Type::Boolean:
			lua_pushboolean(L, var.boolean ? 1 : 0);
			break;
		case LuaVariable::Type::Table:
			lua_rawgeti(L, LUA_REGISTRYINDEX, var.table_ref);
			break;
		case LuaVariable::Type::Number:
		default:
			lua_pushnumber(L, var.number);
			break;
	}
	return true;
}

std::string lua_scope_prefix(const std::string& scope) {
	const std::string normalized = lua_normalize_key(scope);
	if (normalized == "character" || normalized == "char" || normalized == "player") {
		return "";
	}
	if (normalized == "character_temp" || normalized == "char_temp" || normalized == "player_temp") {
		return "@";
	}
	if (normalized == "account") {
		return "#";
	}
	if (normalized == "account_global" || normalized == "global_account" || normalized == "account2") {
		return "##";
	}
	if (normalized == "map_server" || normalized == "server" || normalized == "global") {
		return "$";
	}
	if (normalized == "npc") {
		return ".";
	}
	if (normalized == "npc_temp" || normalized == "local") {
		return ".@";
	}
	if (normalized == "instance") {
		return "'";
	}
	return "";
}

bool lua_scope_supported(const std::string& scope) {
	const std::string normalized = lua_normalize_key(scope);
	return normalized == "character" || normalized == "char" || normalized == "player" ||
	       normalized == "character_temp" || normalized == "char_temp" || normalized == "player_temp" ||
	       normalized == "account" ||
	       normalized == "account_global" || normalized == "global_account" || normalized == "account2" ||
	       normalized == "map_server" || normalized == "server" || normalized == "global" ||
	       normalized == "npc" ||
	       normalized == "npc_temp" || normalized == "local" ||
	       normalized == "instance";
}

std::string lua_build_var_key(const std::string& scope, const std::string& name, bool is_string) {
	std::string key = lua_scope_prefix(scope) + name;
	if (is_string) {
		key.push_back('$');
	}
	return key;
}

int32 lua_builtin_get_var(lua_State* L) {
	if (!lua_isstring(L, 1) || !lua_isstring(L, 2)) {
		lua_pushnil(L);
		return 1;
	}

	const std::string scope = lua_tostring(L, 1);
	const std::string name = lua_tostring(L, 2);
	if (!lua_scope_supported(scope) || name.empty()) {
		lua_pushnil(L);
		return 1;
	}

	const bool has_index = lua_gettop(L) >= 3 && lua_isnumber(L, 3);
	if (has_index) {
		const lua_Integer index = lua_tointeger(L, 3);
		const std::string str_key = lua_build_var_key(scope, name, true);
		const std::string num_key = lua_build_var_key(scope, name, false);
		const std::string table_key = lua_has_variable_exact(str_key) ? str_key : num_key;
		const int top_before = lua_gettop(L);
		if (!lua_push_variable_exact(table_key, L) || !lua_istable(L, -1)) {
			lua_settop(L, top_before);
			lua_pushnil(L);
			return 1;
		}
		lua_geti(L, -1, index);
		lua_remove(L, -2);
		return 1;
	}

	const std::string str_key = lua_build_var_key(scope, name, true);
	if (lua_push_variable_exact(str_key, L)) {
		return 1;
	}
	const std::string num_key = lua_build_var_key(scope, name, false);
	if (lua_push_variable_exact(num_key, L)) {
		return 1;
	}
	lua_pushnil(L);
	return 1;
}

int32 lua_builtin_set_var(lua_State* L) {
	if (!lua_isstring(L, 1) || !lua_isstring(L, 2) || lua_gettop(L) < 3) {
		return 0;
	}

	const std::string scope = lua_tostring(L, 1);
	const std::string name = lua_tostring(L, 2);
	if (!lua_scope_supported(scope) || name.empty()) {
		return 0;
	}

	const bool value_is_string = lua_type(L, 3) == LUA_TSTRING;
	const std::string target_key = lua_build_var_key(scope, name, value_is_string);
	const std::string alt_key = lua_build_var_key(scope, name, !value_is_string);
	const bool has_index = lua_gettop(L) >= 4 && lua_isnumber(L, 4);

	if (!has_index) {
		lua_store_variable(target_key, L, 3);
		lua_pushnil(L);
		lua_store_variable(alt_key, L, -1);
		lua_pop(L, 1);
		return 0;
	}

	const lua_Integer index = lua_tointeger(L, 4);
	const int top_before = lua_gettop(L);
	if (!lua_push_variable_exact(target_key, L) || !lua_istable(L, -1)) {
		lua_settop(L, top_before);
		lua_newtable(L);
	}
	const int32 table_index = lua_gettop(L);
	lua_pushvalue(L, 3);
	lua_seti(L, table_index, index);
	lua_store_variable(target_key, L, table_index);
	lua_pop(L, 1);
	return 0;
}

int32 lua_builtin_get_scoped_var(lua_State* L, const char* scope) {
	if (scope == nullptr || !lua_isstring(L, 1)) {
		lua_pushnil(L);
		return 1;
	}
	lua_pushstring(L, scope);
	lua_insert(L, 1);
	return lua_builtin_get_var(L);
}

int32 lua_builtin_set_scoped_var(lua_State* L, const char* scope) {
	if (scope == nullptr || !lua_isstring(L, 1) || lua_gettop(L) < 2) {
		return 0;
	}
	lua_pushstring(L, scope);
	lua_insert(L, 1);
	return lua_builtin_set_var(L);
}

int32 lua_builtin_get_character_var(lua_State* L) { return lua_builtin_get_scoped_var(L, "character"); }
int32 lua_builtin_set_character_var(lua_State* L) { return lua_builtin_set_scoped_var(L, "character"); }
int32 lua_builtin_get_character_temp_var(lua_State* L) { return lua_builtin_get_scoped_var(L, "character_temp"); }
int32 lua_builtin_set_character_temp_var(lua_State* L) { return lua_builtin_set_scoped_var(L, "character_temp"); }
int32 lua_builtin_get_account_var(lua_State* L) { return lua_builtin_get_scoped_var(L, "account"); }
int32 lua_builtin_set_account_var(lua_State* L) { return lua_builtin_set_scoped_var(L, "account"); }
int32 lua_builtin_get_account_global_var(lua_State* L) { return lua_builtin_get_scoped_var(L, "account_global"); }
int32 lua_builtin_set_account_global_var(lua_State* L) { return lua_builtin_set_scoped_var(L, "account_global"); }
int32 lua_builtin_get_map_server_var(lua_State* L) { return lua_builtin_get_scoped_var(L, "map_server"); }
int32 lua_builtin_set_map_server_var(lua_State* L) { return lua_builtin_set_scoped_var(L, "map_server"); }
int32 lua_builtin_get_npc_var(lua_State* L) { return lua_builtin_get_scoped_var(L, "npc"); }
int32 lua_builtin_set_npc_var(lua_State* L) { return lua_builtin_set_scoped_var(L, "npc"); }
int32 lua_builtin_get_npc_temp_var(lua_State* L) { return lua_builtin_get_scoped_var(L, "npc_temp"); }
int32 lua_builtin_set_npc_temp_var(lua_State* L) { return lua_builtin_set_scoped_var(L, "npc_temp"); }
int32 lua_builtin_get_instance_var(lua_State* L) { return lua_builtin_get_scoped_var(L, "instance"); }
int32 lua_builtin_set_instance_var(lua_State* L) { return lua_builtin_set_scoped_var(L, "instance"); }

void lua_unref(int32 ref) {
	if (g_lua_state == nullptr || ref == LUA_NOREF || ref == LUA_REFNIL) {
		return;
	}
	luaL_unref(g_lua_state, LUA_REGISTRYINDEX, ref);
}

int32 lua_clone_ref(int32 ref) {
	if (g_lua_state == nullptr || ref == LUA_NOREF || ref == LUA_REFNIL) {
		return LUA_NOREF;
	}

	lua_rawgeti(g_lua_state, LUA_REGISTRYINDEX, ref);
	return luaL_ref(g_lua_state, LUA_REGISTRYINDEX);
}

void lua_release_runtime(LuaNpcRuntime& runtime) {
	lua_unref(runtime.main_ref);
	runtime.main_ref = LUA_NOREF;

	for (auto& pair : runtime.event_refs) {
		lua_unref(pair.second);
	}
	runtime.event_refs.clear();

	for (auto& pair : runtime.label_refs) {
		lua_unref(pair.second);
	}
	runtime.label_refs.clear();
	runtime.pos_to_label.clear();
}

void lua_release_function_runtime(LuaFunctionRuntime& runtime) {
	lua_unref(runtime.run_ref);
	runtime.run_ref = LUA_NOREF;

	for (auto& pair : runtime.label_refs) {
		lua_unref(pair.second);
	}
	runtime.label_refs.clear();
}

LuaExecutionContext* lua_current_execution(void) {
	if (g_execution_stack.empty()) {
		return nullptr;
	}
	return &g_execution_stack.back();
}

map_session_data* lua_execution_sd(void) {
	LuaExecutionContext* execution = lua_current_execution();
	if (execution == nullptr || execution->rid <= 0) {
		return nullptr;
	}
	return map_id2sd(execution->rid);
}

uint32 lua_execution_npcid(void) {
	LuaExecutionContext* execution = lua_current_execution();
	if (execution == nullptr) {
		return 0;
	}

	if (execution->oid > 0) {
		return static_cast<uint32>(execution->oid);
	}
	if (execution->npc_id > 0) {
		return static_cast<uint32>(execution->npc_id);
	}
	return 0;
}

uint64 lua_dialog_key(int32 rid, int32 oid) {
	return (static_cast<uint64>(static_cast<uint32>(rid)) << 32) | static_cast<uint32>(oid);
}

void lua_clear_wait_state(LuaExecutionContext& execution) {
	execution.wait_type = LuaWaitType::None;
	execution.clear_cutin_on_close = false;
	execution.input_var.clear();
	execution.input_is_string = false;
	execution.input_min = 0;
	execution.input_max = 0;
	execution.sleep_tick = 0;
}

void lua_attach_player(LuaExecutionContext& execution) {
	if (execution.attached_player || execution.rid <= 0) {
		return;
	}

	map_session_data* sd = map_id2sd(execution.rid);
	if (sd == nullptr) {
		return;
	}

	execution.backup_npc_id = sd->npc_id;
	execution.backup_disable_atcommand = sd->state.disable_atcommand_on_npc != 0;
	execution.attached_player = true;

	sd->npc_id = execution.oid;
	sd->state.disable_atcommand_on_npc =
	    battle_config.atcommand_disable_npc && !pc_has_permission(sd, PC_PERM_ENABLE_COMMAND);
}

void lua_detach_player(const LuaExecutionContext& execution, bool dequeue_event, bool clear_cutin) {
	if (!execution.attached_player || execution.rid <= 0) {
		return;
	}

	map_session_data* sd = map_id2sd(execution.rid);
	if (sd == nullptr) {
		return;
	}

	if (clear_cutin) {
		clif_cutin(*sd, "", 255);
	}

	if (sd->state.using_fake_npc) {
		uint32 clear_npc = sd->npc_id > 0 ? static_cast<uint32>(sd->npc_id) : static_cast<uint32>(execution.oid);
		if (clear_npc != 0) {
			clif_clearunit_single(clear_npc, CLR_OUTSIGHT, *sd);
		}
		sd->state.using_fake_npc = 0;
	}

	sd->state.menu_or_input = 0;
	sd->npc_menu = 0;
	sd->npc_amount = 0;
	sd->npc_str[0] = '\0';
	sd->state.disable_atcommand_on_npc = execution.backup_disable_atcommand ? 1 : 0;
	sd->npc_id = execution.backup_npc_id;

	if (dequeue_event && execution.backup_npc_id == 0) {
		npc_event_dequeue(sd);
	}
}

void lua_release_execution_context(LuaExecutionContext& execution) {
	for (int32 ref : execution.arg_refs) {
		lua_unref(ref);
	}
	execution.arg_refs.clear();
	lua_unref(execution.ctx_ref);
	execution.ctx_ref = LUA_NOREF;
	lua_clear_wait_state(execution);
}

lua_State* lua_instance_thread(int32 thread_ref) {
	if (g_lua_state == nullptr || thread_ref == LUA_NOREF || thread_ref == LUA_REFNIL) {
		return nullptr;
	}

	lua_rawgeti(g_lua_state, LUA_REGISTRYINDEX, thread_ref);
	lua_State* co = lua_tothread(g_lua_state, -1);
	lua_pop(g_lua_state, 1);
	return co;
}

void lua_remove_pending_dialog(const LuaExecutionContext& execution) {
	if (execution.rid <= 0 || execution.oid <= 0) {
		return;
	}
	g_pending_dialog_instances.erase(lua_dialog_key(execution.rid, execution.oid));
}

void lua_set_pending_dialog(uint32 instance_id, const LuaExecutionContext& execution) {
	if (execution.rid <= 0 || execution.oid <= 0) {
		return;
	}
	g_pending_dialog_instances[lua_dialog_key(execution.rid, execution.oid)] = instance_id;
}

void lua_dispose_instance(LuaRuntimeInstance& instance, bool dequeue_event) {
	if (instance.timer_id != INVALID_TIMER) {
		delete_timer(instance.timer_id, lua_instance_resume_timer);
		instance.timer_id = INVALID_TIMER;
	}
	lua_detach_player(instance.execution, dequeue_event, instance.execution.clear_cutin_on_close);
	lua_remove_pending_dialog(instance.execution);
	lua_release_execution_context(instance.execution);
	lua_unref(instance.thread_ref);
	instance.thread_ref = LUA_NOREF;
}

void lua_erase_instance(uint32 instance_id, bool dequeue_event) {
	auto it = g_instances.find(instance_id);
	if (it == g_instances.end()) {
		return;
	}
	lua_dispose_instance(it->second, dequeue_event);
	g_instances.erase(it);
}

bool lua_read_event_name(const char* raw, std::string& exname_out, std::string& label_out) {
	exname_out.clear();
	label_out.clear();

	if (raw == nullptr || raw[0] == '\0') {
		return false;
	}

	const std::string event_name = raw;
	std::size_t pos = event_name.find("::");
	if (pos == std::string::npos) {
		label_out = lua_normalize_key(event_name);
		return true;
	}

	exname_out = lua_normalize_key(event_name.substr(0, pos));
	label_out = lua_normalize_key(event_name.substr(pos + 2));
	return !label_out.empty();
}

std::string lua_value_debug_string(lua_State* L, int index) {
	if (L == nullptr) {
		return "<null lua state>";
	}
	if (index < 0) {
		index = lua_gettop(L) + index + 1;
	}
	if (index <= 0 || index > lua_gettop(L)) {
		return "<empty error stack>";
	}
	const int t = lua_type(L, index);
	const char* type_name = lua_typename(L, t);
	if (lua_isstring(L, index)) {
		const char* s = lua_tostring(L, index);
		return s != nullptr ? s : "<string>";
	}
	luaL_tolstring(L, index, nullptr);
	const char* s = lua_tostring(L, -1);
	std::string out;
	if (s != nullptr) {
		out = s;
	} else {
		out = "<non-string error>";
	}
	lua_pop(L, 1);
	if (type_name != nullptr) {
		out += " [type=";
		out += type_name;
		out += "]";
	}
	return out;
}

std::string lua_traceback_string(lua_State* co, const std::string& message) {
	if (g_lua_state == nullptr || co == nullptr) {
		return message;
	}
	luaL_traceback(g_lua_state, co, message.c_str(), 1);
	const char* tb = lua_tostring(g_lua_state, -1);
	std::string out = (tb != nullptr) ? tb : message;
	lua_pop(g_lua_state, 1);
	return out;
}

int32 lua_ctx_env(lua_State* L) {
	lua_getfield(L, 1, "__env");
	return 1;
}

void lua_push_runtime_env_table(lua_State* L) {
	lua_newtable(L); // env
	lua_newtable(L); // env metatable
	lua_pushcfunction(L, lua_env_index);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, lua_env_newindex);
	lua_setfield(L, -2, "__newindex");
	lua_setmetatable(L, -2);
}

void lua_store_variable(const std::string& key, lua_State* L, int index) {
	const std::string scoped_key = lua_scoped_var_key(key);
	auto old_it = g_var_registry.find(scoped_key);
	if (old_it != g_var_registry.end() && old_it->second.type == LuaVariable::Type::Table) {
		lua_unref(old_it->second.table_ref);
	}

	if (lua_isnil(L, index)) {
		g_var_registry.erase(scoped_key);
		return;
	}

	LuaVariable& var = g_var_registry[scoped_key];
	const int value_type = lua_type(L, index);
	if (value_type == LUA_TNUMBER) {
		var.type = LuaVariable::Type::Number;
		var.number = lua_tonumber(L, index);
	} else if (value_type == LUA_TSTRING) {
		var.type = LuaVariable::Type::String;
		var.string = lua_tostring(L, index);
	} else if (value_type == LUA_TBOOLEAN) {
		var.type = LuaVariable::Type::Boolean;
		var.boolean = lua_toboolean(L, index) != 0;
	} else if (value_type == LUA_TTABLE) {
		var.type = LuaVariable::Type::Table;
		lua_pushvalue(L, index);
		var.table_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	} else {
		g_var_registry.erase(scoped_key);
	}
}

void lua_push_variable(const std::string& key, lua_State* L) {
	const std::string scoped_key = lua_scoped_var_key(key);
	auto it = g_var_registry.find(scoped_key);
	if (it == g_var_registry.end()) {
		if (!key.empty() && key.back() == '$') {
			lua_pushliteral(L, "");
		} else if (key.rfind("$@", 0) == 0) {
			// Global temp array variables in DSL are typically used as indexable tables.
			// Returning an empty table avoids runtime crashes on first indexed access.
			lua_newtable(L);
		} else {
			int64 constant_value = 0;
			if (script_get_constant(key.c_str(), &constant_value)) {
				lua_pushinteger(L, static_cast<lua_Integer>(constant_value));
			} else {
				lua_pushinteger(L, 0);
			}
		}
		return;
	}

	const LuaVariable& var = it->second;
	switch (var.type) {
		case LuaVariable::Type::String:
			lua_pushstring(L, var.string.c_str());
			break;
		case LuaVariable::Type::Boolean:
			lua_pushboolean(L, var.boolean ? 1 : 0);
			break;
		case LuaVariable::Type::Table:
			lua_rawgeti(L, LUA_REGISTRYINDEX, var.table_ref);
			break;
		case LuaVariable::Type::Number:
		default:
			lua_pushnumber(L, var.number);
			break;
	}
}

int32 lua_env_index(lua_State* L) {
	if (!lua_isstring(L, 2)) {
		lua_pushnil(L);
		return 1;
	}

	const std::string key = lua_tostring(L, 2);

	if (g_builtin_table_ref != LUA_NOREF) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, g_builtin_table_ref);
		lua_getfield(L, -1, key.c_str());
		if (!lua_isnil(L, -1)) {
			lua_remove(L, -2);
			return 1;
		}
		lua_pop(L, 2);
	}

	lua_push_variable(key, L);
	return 1;
}

int32 lua_env_newindex(lua_State* L) {
	if (!lua_isstring(L, 2)) {
		return 0;
	}
	lua_store_variable(lua_tostring(L, 2), L, 3);
	return 0;
}

int32 lua_builtin_set(lua_State* L) {
	if (!lua_isstring(L, 1) || lua_gettop(L) < 2) {
		return 0;
	}
	lua_store_variable(lua_tostring(L, 1), L, 2);
	return 0;
}

int32 lua_builtin_getd(lua_State* L) {
	if (!lua_isstring(L, 1)) {
		lua_pushinteger(L, 0);
		return 1;
	}
	lua_push_variable(lua_tostring(L, 1), L);
	return 1;
}

int32 lua_builtin_setd(lua_State* L) {
	if (!lua_isstring(L, 1) || lua_gettop(L) < 2) {
		return 0;
	}
	lua_store_variable(lua_tostring(L, 1), L, 2);
	return 0;
}

int32 lua_builtin_setarray(lua_State* L) {
	if (!lua_isstring(L, 1)) {
		return 0;
	}

	std::string target = lua_tostring(L, 1);
	int32 start = 0;
	size_t open = target.find('[');
	size_t close = target.find(']', open == std::string::npos ? 0 : open + 1);
	if (open != std::string::npos && close != std::string::npos && close > open + 1) {
		start = static_cast<int32>(std::strtol(target.substr(open + 1, close - open - 1).c_str(), nullptr, 10));
		target = target.substr(0, open);
	}

	lua_push_variable(target, L);
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		lua_newtable(L);
	}
	const int32 table_index = lua_gettop(L);

	const int32 argc = static_cast<int32>(lua_gettop(L));
	for (int32 i = 2; i <= argc; ++i) {
		lua_pushvalue(L, i);
		lua_seti(L, table_index, static_cast<lua_Integer>(start + (i - 2)));
	}

	lua_store_variable(target, L, table_index);
	lua_pop(L, 1);
	return 0;
}

int32 lua_builtin_getbattleflag(lua_State* L) {
	(void)L;
	lua_pushinteger(L, 0);
	return 1;
}

int32 lua_builtin_checkcell(lua_State* L) {
	(void)L;
	// Keep movement loops progressing in compatibility mode.
	lua_pushboolean(L, 1);
	return 1;
}

int32 lua_builtin_getvariableofnpc(lua_State* L) {
	if (!lua_isstring(L, 1)) {
		lua_pushinteger(L, 0);
		return 1;
	}

	const char* var_name = lua_tostring(L, 1);
	if (var_name != nullptr && var_name[0] != '\0') {
		const size_t len = std::char_traits<char>::length(var_name);
		if (len > 0 && var_name[len - 1] == '$') {
			lua_pushliteral(L, "");
			return 1;
		}
	}

	lua_pushinteger(L, 0);
	return 1;
}

int32 lua_builtin_mes(lua_State* L) {
	LuaExecutionContext* execution = lua_current_execution();
	map_session_data* sd = lua_execution_sd();
	const uint32 npcid = lua_execution_npcid();
	const int argc = lua_gettop(L);

	for (int i = 1; i <= argc; ++i) {
		if (lua_isnil(L, i)) {
			if (sd != nullptr && npcid != 0) {
				clif_scriptmes(*sd, npcid, "");
			}
			continue;
		}

		const char* message = nullptr;
		if (lua_isstring(L, i)) {
			message = lua_tostring(L, i);
		} else {
			luaL_tolstring(L, i, nullptr);
			message = lua_tostring(L, -1);
		}

		if (sd != nullptr && npcid != 0 && message != nullptr) {
			clif_scriptmes(*sd, npcid, message);
		}

		if (!lua_isstring(L, i)) {
			lua_pop(L, 1);
		}
	}

	if (execution != nullptr) {
		execution->mes_active = true;
	}
	return 0;
}

int32 lua_builtin_next(lua_State* L) {
	(void)L;
	LuaExecutionContext* execution = lua_current_execution();
	map_session_data* sd = lua_execution_sd();
	const uint32 npcid = lua_execution_npcid();
	if (execution == nullptr) {
		return 0;
	}
	if (execution->mes_active && sd != nullptr && npcid != 0) {
		clif_scriptnext(*sd, npcid);
		lua_clear_wait_state(*execution);
		execution->wait_type = LuaWaitType::Next;
		return lua_yield(L, 0);
	}
	return 0;
}

int32 lua_builtin_clear(lua_State* L) {
	(void)L;
	LuaExecutionContext* execution = lua_current_execution();
	map_session_data* sd = lua_execution_sd();
	const uint32 npcid = lua_execution_npcid();
	if (execution != nullptr && execution->mes_active && sd != nullptr && npcid != 0) {
		clif_scriptclear(*sd, static_cast<int32>(npcid));
	}
	return 0;
}

int32 lua_builtin_close_common(lua_State* L, LuaWaitType wait_type, bool close3) {
	LuaExecutionContext* execution = lua_current_execution();
	map_session_data* sd = lua_execution_sd();
	const uint32 npcid = lua_execution_npcid();

	if (execution != nullptr) {
		execution->mes_active = false;
		lua_clear_wait_state(*execution);
		execution->wait_type = wait_type;
		execution->clear_cutin_on_close = close3;
	}

	if (sd != nullptr && npcid != 0) {
		clif_scriptclose(*sd, npcid);
	}
	if (execution != nullptr) {
		return lua_yield(L, 0);
	}
	return 0;
}

int32 lua_builtin_close(lua_State* L) {
	return lua_builtin_close_common(L, LuaWaitType::CloseEnd, false);
}

int32 lua_builtin_close2(lua_State* L) {
	return lua_builtin_close_common(L, LuaWaitType::Close2, false);
}

int32 lua_builtin_close3(lua_State* L) {
	return lua_builtin_close_common(L, LuaWaitType::CloseEnd, true);
}

int32 lua_count_menu_options(const std::string& text) {
	int32 count = 0;
	bool in_option = false;
	for (char ch : text) {
		if (ch == ':') {
			in_option = false;
			continue;
		}
		if (!in_option) {
			in_option = true;
			++count;
		}
	}
	return count;
}

int32 lua_builtin_select(lua_State* L) {
	LuaExecutionContext* execution = lua_current_execution();
	if (execution == nullptr) {
		lua_pushinteger(L, 1);
		return 1;
	}

	int argc = lua_gettop(L);
	std::string menu_text;
	int32 choice_count = 0;

	if (argc <= 0) {
		lua_pushinteger(L, 1);
		return 1;
	}

	if (argc == 1 && lua_isstring(L, 1)) {
		menu_text = lua_tostring(L, 1);
		choice_count = lua_count_menu_options(menu_text);
	} else {
		for (int i = 1; i <= argc; ++i) {
			if (i > 1) {
				menu_text.push_back(':');
			}

			if (lua_isnil(L, i)) {
				continue;
			}
			const char* option = nullptr;
			if (lua_isstring(L, i)) {
				option = lua_tostring(L, i);
			} else {
				luaL_tolstring(L, i, nullptr);
				option = lua_tostring(L, -1);
			}

			if (option != nullptr) {
				menu_text.append(option);
				choice_count += lua_count_menu_options(option);
			}

			if (!lua_isstring(L, i)) {
				lua_pop(L, 1);
			}
		}
	}

	if (choice_count <= 0) {
		choice_count = 1;
	}

	map_session_data* sd = lua_execution_sd();
	const uint32 npcid = lua_execution_npcid();
	if (sd != nullptr && npcid != 0 && !menu_text.empty()) {
		sd->state.menu_or_input = 1;
		sd->npc_menu = choice_count;
		clif_scriptmenu(*sd, npcid, menu_text.c_str());
		lua_clear_wait_state(*execution);
		execution->wait_type = LuaWaitType::Menu;
		return lua_yield(L, 0);
	}

	lua_pushinteger(L, 1);
	return 1;
}

int32 lua_builtin_menu(lua_State* L) {
	return lua_builtin_select(L);
}

int32 lua_builtin_input(lua_State* L) {
	LuaExecutionContext* execution = lua_current_execution();
	if (!lua_isstring(L, 1)) {
		lua_pushinteger(L, 0);
		return 1;
	}
	if (execution == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	std::string var_name = lua_tostring(L, 1);
	int32 min = script_config.input_min_value;
	int32 max = script_config.input_max_value;
	if (lua_gettop(L) >= 2 && lua_isnumber(L, 2)) {
		min = static_cast<int32>(lua_tointeger(L, 2));
	}
	if (lua_gettop(L) >= 3 && lua_isnumber(L, 3)) {
		max = static_cast<int32>(lua_tointeger(L, 3));
	}
	if (min > max) {
		std::swap(min, max);
	}

	map_session_data* sd = lua_execution_sd();
	const uint32 npcid = lua_execution_npcid();
	if (sd != nullptr && npcid != 0) {
		sd->state.menu_or_input = 1;
		if (is_string_variable(var_name.c_str())) {
			clif_scriptinputstr(*sd, npcid);
		} else {
			clif_scriptinput(*sd, npcid);
		}
	}

	lua_clear_wait_state(*execution);
	execution->wait_type = LuaWaitType::Input;
	execution->input_var = std::move(var_name);
	execution->input_is_string = is_string_variable(execution->input_var.c_str());
	execution->input_min = min;
	execution->input_max = max;
	return lua_yield(L, 0);
}

int32 lua_builtin_sleep_common(lua_State* L) {
	LuaExecutionContext* execution = lua_current_execution();
	if (execution == nullptr) {
		return 0;
	}

	int32 tick = 0;
	if (lua_gettop(L) >= 1 && lua_isnumber(L, 1)) {
		tick = static_cast<int32>(lua_tointeger(L, 1));
	}
	if (tick <= 0) {
		tick = 1;
	}

	lua_clear_wait_state(*execution);
	execution->wait_type = LuaWaitType::Sleep;
	execution->sleep_tick = tick;
	return lua_yield(L, 0);
}

int32 lua_builtin_sleep(lua_State* L) {
	return lua_builtin_sleep_common(L);
}

int32 lua_builtin_sleep2(lua_State* L) {
	return lua_builtin_sleep_common(L);
}

int32 lua_builtin_rand(lua_State* L) {
	int argc = lua_gettop(L);
	if (argc <= 0) {
		lua_pushinteger(L, 0);
		return 1;
	}

	if (argc == 1) {
		int32 max = static_cast<int32>(luaL_checkinteger(L, 1));
		if (max <= 0) {
			lua_pushinteger(L, 0);
			return 1;
		}
		lua_pushinteger(L, std::rand() % max);
		return 1;
	}

	int32 min = static_cast<int32>(luaL_checkinteger(L, 1));
	int32 max = static_cast<int32>(luaL_checkinteger(L, 2));
	if (max < min) {
		std::swap(min, max);
	}
	if (max == min) {
		lua_pushinteger(L, min);
		return 1;
	}
	lua_pushinteger(L, min + (std::rand() % (max - min + 1)));
	return 1;
}

int32 lua_builtin_strcharinfo(lua_State* L) {
	LuaExecutionContext* execution = lua_current_execution();
	if (execution == nullptr || execution->rid <= 0) {
		lua_pushliteral(L, "");
		return 1;
	}

	map_session_data* sd = map_id2sd(execution->rid);
	if (sd == nullptr) {
		lua_pushliteral(L, "");
		return 1;
	}

	lua_pushstring(L, sd->status.name);
	return 1;
}

int32 lua_builtin_strnpcinfo(lua_State* L) {
	LuaExecutionContext* execution = lua_current_execution();
	if (execution == nullptr || execution->oid <= 0) {
		lua_pushliteral(L, "");
		return 1;
	}

	npc_data* nd = map_id2nd(execution->oid);
	if (nd == nullptr) {
		lua_pushliteral(L, "");
		return 1;
	}

	int32 mode = 0;
	if (lua_gettop(L) >= 1 && lua_isnumber(L, 1)) {
		mode = static_cast<int32>(lua_tointeger(L, 1));
	}

	const char* exname = nd->exname;
	if (mode == 2) {
		const char* marker = std::strchr(exname, '#');
		lua_pushstring(L, marker != nullptr ? marker + 1 : "");
		return 1;
	}

	lua_pushstring(L, exname);
	return 1;
}

int32 lua_builtin_checkweight(lua_State* L) {
	(void)L;
	lua_pushinteger(L, 1);
	return 1;
}

int32 lua_builtin_countitem(lua_State* L) {
	(void)L;
	lua_pushinteger(L, 0);
	return 1;
}

int32 lua_builtin_checkre(lua_State* L) {
	(void)L;
	lua_pushinteger(L, 1);
	return 1;
}

int32 lua_builtin_getarraysize(lua_State* L) {
	if (!lua_istable(L, 1)) {
		lua_pushinteger(L, 0);
		return 1;
	}

	int32 max_index = -1;
	lua_pushnil(L);
	while (lua_next(L, 1) != 0) {
		if (lua_isinteger(L, -2)) {
			const lua_Integer key = lua_tointeger(L, -2);
			if (key >= 0 && key > max_index) {
				max_index = static_cast<int32>(key);
			}
		}
		lua_pop(L, 1);
	}

	lua_pushinteger(L, max_index + 1);
	return 1;
}

int32 lua_builtin_gettime(lua_State* L) {
	const int32 mode = static_cast<int32>(luaL_optinteger(L, 1, 0));
	const std::time_t now = std::time(nullptr);
	const std::tm* tm_now = std::localtime(&now);
	if (tm_now == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	int32 value = 0;
	switch (mode) {
		case 1: value = tm_now->tm_year + 1900; break;
		case 2: value = tm_now->tm_mon + 1; break;
		case 3: value = tm_now->tm_mday; break;
		case 4: value = tm_now->tm_wday; break;
		case 5: value = tm_now->tm_yday + 1; break;
		case 6: value = tm_now->tm_hour; break;
		case 7: value = tm_now->tm_min; break;
		case 8: value = tm_now->tm_sec; break;
		default: value = 0; break;
	}

	lua_pushinteger(L, value);
	return 1;
}

int32 lua_builtin_replacestr(lua_State* L) {
	const std::string input = luaL_optstring(L, 1, "");
	const std::string target = luaL_optstring(L, 2, "");
	const std::string replacement = luaL_optstring(L, 3, "");

	if (target.empty()) {
		lua_pushlstring(L, input.data(), input.size());
		return 1;
	}

	std::string out = input;
	size_t pos = 0;
	while ((pos = out.find(target, pos)) != std::string::npos) {
		out.replace(pos, target.size(), replacement);
		pos += replacement.size();
	}

	lua_pushlstring(L, out.data(), out.size());
	return 1;
}

int32 lua_builtin_charat(lua_State* L) {
	const std::string input = luaL_optstring(L, 1, "");
	const int32 index = static_cast<int32>(luaL_optinteger(L, 2, 0));
	if (index < 0 || static_cast<size_t>(index) >= input.size()) {
		lua_pushliteral(L, "");
		return 1;
	}
	const char ch = input[static_cast<size_t>(index)];
	lua_pushlstring(L, &ch, 1);
	return 1;
}

int32 lua_builtin_getstrlen(lua_State* L) {
	const char* value = luaL_optstring(L, 1, "");
	if (value == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}
	lua_pushinteger(L, static_cast<lua_Integer>(std::strlen(value)));
	return 1;
}

int32 lua_builtin_agitcheck3(lua_State* L) {
	(void)L;
	lua_pushinteger(L, 0);
	return 1;
}

int32 lua_builtin_agitcheck(lua_State* L) {
	(void)L;
	lua_pushinteger(L, 0);
	return 1;
}

int32 lua_builtin_getcastledata(lua_State* L) {
	const char* mapname = luaL_optstring(L, 1, "");
	const int32 index = static_cast<int32>(luaL_optinteger(L, 2, 0));
	if (mapname == nullptr || *mapname == '\0') {
		lua_pushinteger(L, 0);
		return 1;
	}

	std::shared_ptr<guild_castle> gc = castle_db.mapname2gc(mapname);
	if (gc == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	int32 value = 0;
	switch (index) {
		case CD_GUILD_ID: value = gc->guild_id; break;
		case CD_CURRENT_ECONOMY: value = gc->economy; break;
		case CD_CURRENT_DEFENSE: value = gc->defense; break;
		case CD_INVESTED_ECONOMY: value = gc->triggerE; break;
		case CD_INVESTED_DEFENSE: value = gc->triggerD; break;
		case CD_NEXT_TIME: value = gc->nextTime; break;
		case CD_PAY_TIME: value = gc->payTime; break;
		case CD_CREATE_TIME: value = gc->createTime; break;
		case CD_ENABLED_KAFRA: value = gc->visibleC; break;
		default:
			if (index >= CD_ENABLED_GUARDIAN00 && index < CD_MAX) {
				value = gc->guardian[index - CD_ENABLED_GUARDIAN00].visible;
			} else {
				value = 0;
			}
			break;
	}

	lua_pushinteger(L, value);
	return 1;
}

int32 lua_builtin_setcastledata(lua_State* L) {
	const char* mapname = luaL_optstring(L, 1, "");
	const int32 index = static_cast<int32>(luaL_optinteger(L, 2, 0));
	const int32 value = static_cast<int32>(luaL_optinteger(L, 3, 0));
	if (mapname == nullptr || *mapname == '\0') {
		return 0;
	}

	std::shared_ptr<guild_castle> gc = castle_db.mapname2gc(mapname);
	if (gc == nullptr) {
		return 0;
	}
	if (index <= CD_NONE || index >= CD_MAX) {
		return 0;
	}

	guild_castledatasave(gc->castle_id, index, value);
	return 0;
}

int32 lua_builtin_compare(lua_State* L) {
	const char* message = luaL_optstring(L, 1, "");
	const char* needle = luaL_optstring(L, 2, "");
	if (message == nullptr || needle == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	std::string haystack(message);
	std::string target(needle);
	std::transform(haystack.begin(), haystack.end(), haystack.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});
	std::transform(target.begin(), target.end(), target.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});

	lua_pushinteger(L, haystack.find(target) != std::string::npos ? 1 : 0);
	return 1;
}

bool lua_get_item_by_arg(lua_State* L, int32 arg_index, std::shared_ptr<item_data>& out_item_data, t_itemid& out_nameid,
                         const char* func_name) {
	if (lua_gettop(L) < arg_index) {
		return false;
	}

	if (lua_isstring(L, arg_index)) {
		const char* name = lua_tostring(L, arg_index);
		out_item_data = item_db.searchname(name != nullptr ? name : "");
		if (out_item_data == nullptr) {
			ShowError("lua_engine: %s: non-existent item '%s' requested.\n", func_name, name != nullptr ? name : "");
			return false;
		}
		out_nameid = out_item_data->nameid;
		return true;
	}

	if (!lua_isnumber(L, arg_index)) {
		ShowError("lua_engine: %s: item must be item id or item name.\n", func_name);
		return false;
	}

	out_nameid = static_cast<t_itemid>(lua_tointeger(L, arg_index));
	out_item_data = item_db.find(out_nameid);
	if (out_item_data == nullptr) {
		ShowError("lua_engine: %s: non-existent item '%u' requested.\n", func_name, out_nameid);
		return false;
	}
	return true;
}

map_session_data* lua_get_target_sd(lua_State* L, int32 arg_index, bool by_char_id, const char* func_name) {
	if (lua_gettop(L) >= arg_index && lua_isnumber(L, arg_index)) {
		const int32 id = static_cast<int32>(lua_tointeger(L, arg_index));
		map_session_data* sd = by_char_id ? map_charid2sd(id) : map_id2sd(id);
		if (sd == nullptr) {
			ShowError("lua_engine: %s: player with %s '%d' not found.\n",
			          func_name, by_char_id ? "char id" : "account/map id", id);
		}
		return sd;
	}
	return lua_execution_sd();
}

int32 lua_builtin_getitem(lua_State* L) {
	t_itemid nameid = 0;
	std::shared_ptr<item_data> id;
	if (!lua_get_item_by_arg(L, 1, id, nameid, "getitem")) {
		return 0;
	}

	const int32 amount = static_cast<int32>(luaL_optinteger(L, 2, 0));
	if (amount <= 0) {
		return 0;
	}

	map_session_data* sd = lua_get_target_sd(L, 3, false, "getitem");
	if (sd == nullptr) {
		return 0;
	}

	item it = {};
	it.nameid = nameid;
	it.identify = 1;
	it.bound = BOUND_NONE;

	const int32 get_count = itemdb_isstackable2(id.get()) ? amount : 1;
	for (int32 i = 0; i < amount; i += get_count) {
		if (!pet_create_egg(sd, nameid)) {
			e_additem_result flag = pc_additem(sd, &it, get_count, LOG_TYPE_SCRIPT);
			if (flag != ADDITEM_SUCCESS) {
				clif_additem(sd, 0, 0, flag);
				ShowError("lua_engine: getitem: failed to add item %u to '%s'.\n", nameid, sd->status.name);
				return 0;
			}
		}
	}

	return 0;
}

int32 lua_builtin_delitem(lua_State* L) {
	t_itemid nameid = 0;
	std::shared_ptr<item_data> id;
	if (!lua_get_item_by_arg(L, 1, id, nameid, "delitem")) {
		return 0;
	}

	int32 amount = static_cast<int32>(luaL_optinteger(L, 2, 0));
	if (amount <= 0) {
		return 0;
	}

	map_session_data* sd = lua_get_target_sd(L, 3, false, "delitem");
	if (sd == nullptr) {
		return 0;
	}

	// Preserve equip state by deleting inventory stacks before equipped slots.
	for (int32 pass = 0; pass < 2 && amount > 0; ++pass) {
		for (int32 i = 0; i < MAX_INVENTORY && amount > 0; ++i) {
			item& inv = sd->inventory.u.items_inventory[i];
			if (inv.nameid != nameid) {
				continue;
			}
			if ((pass == 0 && inv.equip != 0) || (pass == 1 && inv.equip == 0)) {
				continue;
			}
			const int32 take = amount < inv.amount ? amount : inv.amount;
			if (take <= 0) {
				continue;
			}
			if (pc_delitem(sd, i, take, 0, 0, LOG_TYPE_SCRIPT) == 0) {
				amount -= take;
			}
		}
	}

	if (amount > 0) {
		ShowError("lua_engine: delitem: failed to delete item %u x%d from '%s'.\n", nameid, amount, sd->status.name);
	}

	return 0;
}

int32 lua_builtin_warp(lua_State* L) {
	const char* map_name = luaL_optstring(L, 1, "");
	const int32 x = static_cast<int32>(luaL_optinteger(L, 2, 0));
	const int32 y = static_cast<int32>(luaL_optinteger(L, 3, 0));
	map_session_data* sd = lua_get_target_sd(L, 4, true, "warp");
	if (sd == nullptr || map_name == nullptr || *map_name == '\0') {
		return 0;
	}

	int32 ret = 0;
	if (strcmp(map_name, "Random") == 0) {
		ret = pc_randomwarp(sd, CLR_TELEPORT, true);
	} else if (strcmp(map_name, "SavePoint") == 0 || strcmp(map_name, "Save") == 0) {
		ret = pc_setpos(sd, mapindex_name2id(sd->status.save_point.map), sd->status.save_point.x, sd->status.save_point.y,
		                CLR_TELEPORT);
	} else {
		ret = pc_setpos(sd, mapindex_name2id(map_name), x, y, CLR_OUTSIGHT);
	}

	if (ret != 0) {
		ShowError("lua_engine: warp: moving player '%s' to \"%s\",%d,%d failed.\n", sd->status.name, map_name, x, y);
	}
	return 0;
}

int32 lua_builtin_announce(lua_State* L) {
	const char* message = luaL_optstring(L, 1, "");
	const int32 flag = static_cast<int32>(luaL_optinteger(L, 2, BC_DEFAULT));
	if (message == nullptr || *message == '\0') {
		return 0;
	}

	block_list* bl = nullptr;
	if ((flag & BC_NPC) != 0) {
		LuaExecutionContext* execution = lua_current_execution();
		if (execution != nullptr && execution->oid > 0) {
			bl = map_id2bl(execution->oid);
		}
	} else {
		map_session_data* sd = lua_get_target_sd(L, 3, true, "announce");
		bl = sd;
	}

	send_target target = ALL_CLIENT;
	switch (flag & BC_TARGET_MASK) {
		case BC_MAP: target = ALL_SAMEMAP; break;
		case BC_AREA: target = AREA; break;
		case BC_SELF: target = SELF; break;
		default: target = ALL_CLIENT; break;
	}

	if (target != SELF) {
		intif_broadcast(message, static_cast<int32>(strlen(message)) + 1, flag & BC_COLOR_MASK);
	}
	if (bl != nullptr) {
		clif_broadcast(bl, message, strlen(message) + 1, flag & BC_COLOR_MASK, target);
	}
	return 0;
}

int32 lua_builtin_announce_sub(block_list* bl, va_list ap) {
	const char* mes = va_arg(ap, const char*);
	int32 len = va_arg(ap, int32);
	int32 type = va_arg(ap, int32);
	clif_broadcast(bl, mes, static_cast<size_t>(len), type, SELF);
	return SCRIPT_CMD_SUCCESS;
}

int32 lua_builtin_mapannounce(lua_State* L) {
	const char* mapname = luaL_optstring(L, 1, "");
	const char* message = luaL_optstring(L, 2, "");
	const int32 flag = static_cast<int32>(luaL_optinteger(L, 3, BC_DEFAULT));
	if (mapname == nullptr || message == nullptr || *mapname == '\0' || *message == '\0') {
		return 0;
	}

	const int16 m = map_mapname2mapid(mapname);
	if (m < 0) {
		return 0;
	}

	map_foreachinmap(lua_builtin_announce_sub, m, BL_PC, message, static_cast<int32>(strlen(message)) + 1,
	                 flag & BC_COLOR_MASK);
	return 0;
}

int32 lua_builtin_cutin(lua_State* L) {
	map_session_data* sd = lua_execution_sd();
	if (sd == nullptr) {
		return 0;
	}
	const char* image = luaL_optstring(L, 1, "");
	int32 type = static_cast<int32>(luaL_optinteger(L, 2, 0));
	clif_cutin(*sd, image != nullptr ? image : "", type);
	return 0;
}

int32 lua_builtin_specialeffect(lua_State* L) {
	block_list* bl = map_id2bl(lua_current_execution() ? lua_current_execution()->oid : 0);
	if (bl == nullptr) {
		return 0;
	}
	int32 type = static_cast<int32>(luaL_optinteger(L, 1, 0));
	send_target target = static_cast<send_target>(luaL_optinteger(L, 2, AREA));
	if (type <= EF_NONE || type >= EF_MAX) {
		ShowError("lua_engine: specialeffect: unsupported effect id %d.\n", type);
		return 0;
	}
	if (lua_gettop(L) >= 3 && lua_isstring(L, 3)) {
		npc_data* nd = npc_name2id(lua_tostring(L, 3));
		if (nd != nullptr) {
			bl = nd;
		}
	}
	if (target == SELF) {
		map_session_data* sd = lua_execution_sd();
		if (sd != nullptr) {
			clif_specialeffect_single(bl, type, sd->fd);
		}
	} else {
		clif_specialeffect(bl, type, target);
	}
	return 0;
}

int32 lua_builtin_specialeffect2(lua_State* L) {
	map_session_data* sd = lua_get_target_sd(L, 3, false, "specialeffect2");
	if (sd == nullptr) {
		return 0;
	}
	int32 type = static_cast<int32>(luaL_optinteger(L, 1, 0));
	send_target target = static_cast<send_target>(luaL_optinteger(L, 2, AREA));
	if (type <= EF_NONE || type >= EF_MAX) {
		ShowError("lua_engine: specialeffect2: unsupported effect id %d.\n", type);
		return 0;
	}
	clif_specialeffect(sd, type, target);
	return 0;
}

int32 lua_builtin_getexp(lua_State* L) {
	map_session_data* sd = lua_get_target_sd(L, 3, true, "getexp");
	if (sd == nullptr) {
		return 0;
	}
	int64 base = static_cast<int64>(luaL_optinteger(L, 1, 0));
	int64 job = static_cast<int64>(luaL_optinteger(L, 2, 0));
	if (base < 0 || job < 0 || (base == 0 && job == 0)) {
		ShowError("lua_engine: getexp: invalid base/job exp (%" PRId64 ", %" PRId64 ").\n", base, job);
		return 0;
	}
	double bonus = battle_config.quest_exp_rate / 100.;
	if (base) {
		double scaled = base * bonus;
		if (scaled < 0) scaled = 0;
		if (scaled > static_cast<double>(MAX_EXP)) scaled = static_cast<double>(MAX_EXP);
		base = static_cast<int64>(scaled);
	}
	if (job) {
		double scaled = job * bonus;
		if (scaled < 0) scaled = 0;
		if (scaled > static_cast<double>(MAX_EXP)) scaled = static_cast<double>(MAX_EXP);
		job = static_cast<int64>(scaled);
	}
	pc_gainexp(sd, nullptr, base, job, 1);
	return 0;
}

int32 lua_builtin_atoi(lua_State* L) {
	const char* v = luaL_optstring(L, 1, "");
	lua_pushinteger(L, v != nullptr ? std::atoi(v) : 0);
	return 1;
}

int32 lua_builtin_getmonsterinfo(lua_State* L) {
	std::shared_ptr<s_mob_db> mob = nullptr;
	if (lua_isstring(L, 1)) {
		mob = mobdb_search_aegisname(lua_tostring(L, 1));
	} else if (lua_isnumber(L, 1)) {
		uint16 mob_id = static_cast<uint16>(lua_tointeger(L, 1));
		if (!mob_is_clone(mob_id)) {
			mob = mob_db.find(mob_id);
		}
	}
	int32 type = static_cast<int32>(luaL_optinteger(L, 2, MOB_ID));
	if (mob == nullptr) {
		if (type == MOB_NAME) {
			lua_pushliteral(L, "null");
		} else {
			lua_pushinteger(L, -1);
		}
		return 1;
	}
	switch (type) {
		case MOB_NAME: lua_pushstring(L, mob->jname.c_str()); break;
		case MOB_LV: lua_pushinteger(L, mob->lv); break;
		case MOB_MAXHP: lua_pushinteger(L, mob->status.max_hp); break;
		case MOB_MAXSP: lua_pushinteger(L, mob->status.max_sp); break;
		case MOB_BASEEXP: lua_pushinteger(L, mob->base_exp); break;
		case MOB_JOBEXP: lua_pushinteger(L, mob->job_exp); break;
		case MOB_ATKMIN: lua_pushinteger(L, mob->status.rhw.atk); break;
		case MOB_ATKMAX: lua_pushinteger(L, mob->status.rhw.atk2); break;
		case MOB_DEF: lua_pushinteger(L, mob->status.def); break;
		case MOB_MDEF: lua_pushinteger(L, mob->status.mdef); break;
		case MOB_RES: lua_pushinteger(L, mob->status.res); break;
		case MOB_MRES: lua_pushinteger(L, mob->status.mres); break;
		case MOB_STR: lua_pushinteger(L, mob->status.str); break;
		case MOB_AGI: lua_pushinteger(L, mob->status.agi); break;
		case MOB_VIT: lua_pushinteger(L, mob->status.vit); break;
		case MOB_INT: lua_pushinteger(L, mob->status.int_); break;
		case MOB_DEX: lua_pushinteger(L, mob->status.dex); break;
		case MOB_LUK: lua_pushinteger(L, mob->status.luk); break;
		case MOB_SPEED: lua_pushinteger(L, mob->status.speed); break;
		case MOB_ATKRANGE: lua_pushinteger(L, mob->status.rhw.range); break;
		case MOB_SKILLRANGE: lua_pushinteger(L, mob->range2); break;
		case MOB_CHASERANGE: lua_pushinteger(L, mob->range3); break;
		case MOB_SIZE: lua_pushinteger(L, mob->status.size); break;
		case MOB_RACE: lua_pushinteger(L, mob->status.race); break;
		case MOB_ELEMENT: lua_pushinteger(L, mob->status.def_ele); break;
		case MOB_ELEMENTLV: lua_pushinteger(L, mob->status.ele_lv); break;
		case MOB_MODE: lua_pushinteger(L, mob->status.mode); break;
		case MOB_MVPEXP: lua_pushinteger(L, mob->mexp); break;
		case MOB_ID: lua_pushinteger(L, mob->id); break;
		default: lua_pushinteger(L, -1); break;
	}
	return 1;
}

int32 lua_builtin_getnpcid(lua_State* L) {
	int32 mode = static_cast<int32>(luaL_optinteger(L, 1, 0));
	npc_data* nd = nullptr;
	if (lua_gettop(L) >= 2 && lua_isstring(L, 2)) {
		nd = npc_name2id(lua_tostring(L, 2));
	}
	if (mode == 0) {
		if (nd != nullptr) {
			lua_pushinteger(L, nd->id);
		} else {
			LuaExecutionContext* execution = lua_current_execution();
			lua_pushinteger(L, execution ? execution->oid : 0);
		}
	} else {
		lua_pushinteger(L, 0);
	}
	return 1;
}

int32 lua_builtin_sc_start2(lua_State* L) {
	block_list* bl = nullptr;
	if (lua_gettop(L) >= 7 && lua_isnumber(L, 7)) {
		bl = map_id2bl(static_cast<int32>(lua_tointeger(L, 7)));
	}
	if (bl == nullptr) {
		LuaExecutionContext* execution = lua_current_execution();
		bl = map_id2bl(execution ? execution->rid : 0);
	}
	if (bl == nullptr) {
		return 0;
	}
	sc_type type = static_cast<sc_type>(luaL_optinteger(L, 1, 0));
	int32 tick = static_cast<int32>(luaL_optinteger(L, 2, 0));
	int32 val1 = static_cast<int32>(luaL_optinteger(L, 3, 0));
	int32 val2 = static_cast<int32>(luaL_optinteger(L, 4, 0));
	int32 rate = static_cast<int32>(luaL_optinteger(L, 5, 10000));
	int32 flag = static_cast<int32>(luaL_optinteger(L, 6, SCSTART_NOAVOID));
	status_change_start(bl, bl, type, rate, val1, val2, 0, 0, tick, flag);
	return 0;
}

npc_data* lua_target_npc_by_name_or_context(lua_State* L, int arg_index) {
	if (lua_gettop(L) >= arg_index && lua_isstring(L, arg_index)) {
		const char* npc_name = lua_tostring(L, arg_index);
		if (npc_name != nullptr && *npc_name != '\0') {
			return npc_name2id(npc_name);
		}
	}
	LuaExecutionContext* execution = lua_current_execution();
	return map_id2nd(execution ? execution->oid : 0);
}

int32 lua_builtin_emotion(lua_State* L) {
	int32 type = static_cast<int32>(luaL_optinteger(L, 1, ET_SURPRISE));
	if (type < ET_SURPRISE || type >= ET_MAX) {
		return 0;
	}
	block_list* bl = nullptr;
	if (lua_gettop(L) >= 2 && lua_isnumber(L, 2)) {
		bl = map_id2bl(static_cast<int32>(lua_tointeger(L, 2)));
	}
	if (bl == nullptr) {
		LuaExecutionContext* execution = lua_current_execution();
		bl = map_id2bl(execution ? execution->oid : 0);
	}
	if (bl != nullptr) {
		clif_emotion(*bl, static_cast<emotion_type>(type));
	}
	return 0;
}

int32 lua_builtin_npctalk(lua_State* L) {
	const char* msg = luaL_optstring(L, 1, "");
	npc_data* nd = lua_target_npc_by_name_or_context(L, 2);
	if (nd == nullptr || msg == nullptr) {
		return 0;
	}
	send_target target = AREA;
	if (lua_gettop(L) >= 3 && lua_isnumber(L, 3)) {
		switch (static_cast<int32>(lua_tointeger(L, 3))) {
			case BC_ALL: target = ALL_CLIENT; break;
			case BC_MAP: target = ALL_SAMEMAP; break;
			case BC_SELF: target = SELF; break;
			default: target = AREA; break;
		}
	}
	uint32 color = (lua_gettop(L) >= 4 && lua_isnumber(L, 4)) ? static_cast<uint32>(lua_tointeger(L, 4)) : 0xFFFFFF;
	if (target != SELF) {
		clif_messagecolor(nd, color, msg, true, target);
	} else {
		map_session_data* sd = lua_execution_sd();
		if (sd != nullptr) {
			clif_messagecolor_target(nd, color, msg, true, target, sd);
		}
	}
	return 0;
}

int32 lua_builtin_dispbottom(lua_State* L) {
	map_session_data* sd = lua_get_target_sd(L, 3, true, "dispbottom");
	if (sd == nullptr) {
		return 0;
	}
	const char* message = luaL_optstring(L, 1, "");
	if (lua_gettop(L) >= 2 && lua_isnumber(L, 2)) {
		clif_messagecolor(sd, static_cast<int32>(lua_tointeger(L, 2)), message, true, SELF);
	} else {
		clif_messagecolor(sd, color_table[COLOR_LIGHT_GREEN], message, false, SELF);
	}
	return 0;
}

int32 lua_builtin_cleararray(lua_State* L) {
	if (!lua_isstring(L, 1)) {
		return 0;
	}
	const char* name = lua_tostring(L, 1);
	int32 value = static_cast<int32>(luaL_optinteger(L, 2, 0));
	int32 count = static_cast<int32>(luaL_optinteger(L, 3, 0));
	if (count <= 0) {
		return 0;
	}
	lua_push_variable(name, L);
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		lua_newtable(L);
	}
	int table_idx = lua_gettop(L);
	for (int32 i = 0; i < count; ++i) {
		lua_pushinteger(L, value);
		lua_seti(L, table_idx, i);
	}
	lua_store_variable(name, L, table_idx);
	lua_pop(L, 1);
	return 0;
}

int32 lua_builtin_questinfo(lua_State* L) {
	npc_data* nd = map_id2nd(lua_current_execution() ? lua_current_execution()->oid : 0);
	map_session_data* sd = lua_execution_sd();
	if (nd == nullptr || sd == nullptr) {
		return 0;
	}
	int32 icon = static_cast<int32>(luaL_optinteger(L, 1, QTYPE_QUEST));
	int32 color = static_cast<int32>(luaL_optinteger(L, 2, QMARK_NONE));
	clif_quest_show_event(sd, nd, static_cast<e_questinfo_types>(icon), static_cast<e_questinfo_markcolor>(color));
	return 0;
}

int32 lua_builtin_waitingroom(lua_State* L) {
	npc_data* nd = map_id2nd(lua_current_execution() ? lua_current_execution()->oid : 0);
	if (nd == nullptr) {
		return 0;
	}
	const char* title = luaL_optstring(L, 1, "");
	int32 limit = static_cast<int32>(luaL_optinteger(L, 2, 0));
	const char* event = luaL_optstring(L, 3, "");
	int32 trigger = static_cast<int32>(luaL_optinteger(L, 4, limit));
	int32 zeny = static_cast<int32>(luaL_optinteger(L, 5, 0));
	int32 min_lvl = static_cast<int32>(luaL_optinteger(L, 6, 1));
	int32 max_lvl = static_cast<int32>(luaL_optinteger(L, 7, MAX_LEVEL));
	chat_createnpcchat(nd, title, limit, 1, trigger, event, zeny, min_lvl, max_lvl);
	return 0;
}

int32 lua_builtin_npc_visibility(lua_State* L, e_npcv_status flag) {
	npc_data* nd = lua_target_npc_by_name_or_context(L, 1);
	if (nd == nullptr) {
		return 0;
	}
	int32 char_id = (lua_gettop(L) >= 2 && lua_isnumber(L, 2)) ? static_cast<int32>(lua_tointeger(L, 2)) : 0;
	npc_enable_target(*nd, char_id, flag);
	return 0;
}

int32 lua_builtin_hideonnpc(lua_State* L) { return lua_builtin_npc_visibility(L, NPCVIEW_HIDEON); }
int32 lua_builtin_hideoffnpc(lua_State* L) { return lua_builtin_npc_visibility(L, NPCVIEW_HIDEOFF); }
int32 lua_builtin_cloakonnpc(lua_State* L) { return lua_builtin_npc_visibility(L, NPCVIEW_CLOAKON); }
int32 lua_builtin_cloakoffnpc(lua_State* L) { return lua_builtin_npc_visibility(L, NPCVIEW_CLOAKOFF); }

int32 lua_builtin_unloadnpc(lua_State* L) {
	const char* name = luaL_optstring(L, 1, "");
	npc_data* nd = npc_name2id(name);
	LuaExecutionContext* execution = lua_current_execution();
	if (nd == nullptr || (execution && nd->id == execution->oid)) {
		return 0;
	}
	npc_unload_duplicates(nd);
	npc_unload(nd, true);
	npc_read_event_script();
	return 0;
}

int32 lua_builtin_stopnpctimer(lua_State* L) {
	npc_data* nd = lua_target_npc_by_name_or_context(L, 1);
	if (nd != nullptr) {
		npc_timerevent_stop(nd);
	}
	return 0;
}

int32 lua_builtin_setnpctimer(lua_State* L) {
	int32 tick = static_cast<int32>(luaL_optinteger(L, 1, 0));
	npc_data* nd = lua_target_npc_by_name_or_context(L, 2);
	if (nd != nullptr) {
		npc_settimerevent_tick(nd, tick);
	}
	return 0;
}

int32 lua_builtin_startnpctimer(lua_State* L) {
	npc_data* nd = lua_target_npc_by_name_or_context(L, 1);
	if (nd != nullptr) {
		npc_timerevent_start(nd, lua_current_execution() ? lua_current_execution()->rid : 0);
	}
	return 0;
}

int32 lua_builtin_enablewaitingroomevent(lua_State* L) {
	npc_data* nd = lua_target_npc_by_name_or_context(L, 1);
	if (nd != nullptr) {
		chat_data* cd = reinterpret_cast<chat_data*>(map_id2bl(nd->chat_id));
		if (cd != nullptr) {
			chat_enableevent(cd);
		}
	}
	return 0;
}

int32 lua_builtin_disablewaitingroomevent(lua_State* L) {
	npc_data* nd = lua_target_npc_by_name_or_context(L, 1);
	if (nd != nullptr) {
		chat_data* cd = reinterpret_cast<chat_data*>(map_id2bl(nd->chat_id));
		if (cd != nullptr) {
			chat_disableevent(cd);
		}
	}
	return 0;
}

int32 lua_builtin_monster(lua_State* L) {
	const char* mapn = luaL_optstring(L, 1, "");
	int32 x = static_cast<int32>(luaL_optinteger(L, 2, 0));
	int32 y = static_cast<int32>(luaL_optinteger(L, 3, 0));
	const char* display_name = luaL_optstring(L, 4, "--ja--");
	int32 class_ = 0;
	if (lua_isstring(L, 5)) {
		auto mob = mobdb_search_aegisname(lua_tostring(L, 5));
		if (mob == nullptr) {
			return 0;
		}
		class_ = mob->id;
	} else {
		class_ = static_cast<int32>(luaL_optinteger(L, 5, 0));
		if (class_ >= 0 && !mobdb_checkid(class_)) {
			return 0;
		}
	}
	int32 amount = static_cast<int32>(luaL_optinteger(L, 6, 1));
	const char* event = luaL_optstring(L, 7, "");
	uint32 size = static_cast<uint32>(luaL_optinteger(L, 8, SZ_SMALL));
	mob_ai ai = static_cast<mob_ai>(luaL_optinteger(L, 9, AI_NONE));
	map_session_data* sd = lua_execution_sd();
	int16 m = (sd != nullptr && std::strcmp(mapn, "this") == 0) ? sd->m : map_mapname2mapid(mapn);
	for (int32 i = 0; i < amount; ++i) {
		mob_once_spawn(sd, m, x, y, display_name, class_, 1, event, size, ai);
	}
	return 0;
}

int32 lua_builtin_areamonster(lua_State* L) {
	const char* mapn = luaL_optstring(L, 1, "");
	int32 x0 = static_cast<int32>(luaL_optinteger(L, 2, 0));
	int32 y0 = static_cast<int32>(luaL_optinteger(L, 3, 0));
	int32 x1 = static_cast<int32>(luaL_optinteger(L, 4, 0));
	int32 y1 = static_cast<int32>(luaL_optinteger(L, 5, 0));
	const char* display_name = luaL_optstring(L, 6, "--ja--");
	int32 class_ = 0;
	if (lua_isstring(L, 7)) {
		auto mob = mobdb_search_aegisname(lua_tostring(L, 7));
		if (mob == nullptr) {
			return 0;
		}
		class_ = mob->id;
	} else {
		class_ = static_cast<int32>(luaL_optinteger(L, 7, 0));
		if (class_ >= 0 && !mobdb_checkid(class_)) {
			return 0;
		}
	}
	int32 amount = static_cast<int32>(luaL_optinteger(L, 8, 1));
	const char* event = luaL_optstring(L, 9, "");
	uint32 size = static_cast<uint32>(luaL_optinteger(L, 10, SZ_SMALL));
	mob_ai ai = static_cast<mob_ai>(luaL_optinteger(L, 11, AI_NONE));
	map_session_data* sd = lua_execution_sd();
	int16 m = (sd != nullptr && std::strcmp(mapn, "this") == 0) ? sd->m : map_mapname2mapid(mapn);
	for (int32 i = 0; i < amount; ++i) {
		mob_once_spawn_area(sd, m, x0, y0, x1, y1, display_name, class_, 1, event, size, ai);
	}
	return 0;
}

int32 lua_builtin_killmonster_sub(block_list* bl, va_list ap) {
	const char* event = va_arg(ap, const char*);
	int32 allflag = va_arg(ap, int32);
	mob_data* md = reinterpret_cast<mob_data*>(bl);
	if (!allflag) {
		if (std::strcmp(event, md->npc_event) == 0) {
			status_kill(bl);
		}
	} else if (!md->spawn) {
		status_kill(bl);
	}
	return SCRIPT_CMD_SUCCESS;
}

int32 lua_builtin_killmonster(lua_State* L) {
	const char* mapname = luaL_optstring(L, 1, "");
	const char* event = luaL_optstring(L, 2, "");
	int16 m = map_mapname2mapid(mapname);
	if (m < 0) {
		return 0;
	}
	int32 allflag = (std::strcmp(event, "All") == 0) ? 1 : 0;
	map_foreachinmap(lua_builtin_killmonster_sub, m, BL_MOB, event, allflag);
	return 0;
}

int32 lua_builtin_bg_unbook(lua_State* L) {
	const char* name = luaL_optstring(L, 1, "");
	bg_queue_unbook(name);
	return 0;
}

int32 lua_builtin_removemapflag(lua_State* L) {
	const char* mapname = luaL_optstring(L, 1, "");
	int32 mf = static_cast<int32>(luaL_optinteger(L, 2, 0));
	union u_mapflag_args args = {};
	args.flag_val = static_cast<int32>(luaL_optinteger(L, 3, 0));
	int16 m = map_mapname2mapid(mapname);
	if (m < 0 || mf < MF_MIN || mf > MF_MAX) {
		return 0;
	}
	map_setmapflag_sub(m, static_cast<e_mapflag>(mf), false, &args);
	return 0;
}

int32 lua_builtin_areawarp_sub(block_list* bl, va_list ap) {
	uint32 index = va_arg(ap, uint32);
	int32 x2 = va_arg(ap, int32);
	int32 y2 = va_arg(ap, int32);
	int32 x3 = va_arg(ap, int32);
	int32 y3 = va_arg(ap, int32);
	if (index == 0) {
		pc_randomwarp(reinterpret_cast<TBL_PC*>(bl), CLR_TELEPORT, true);
	} else if (x3 && y3) {
		int32 tx = rnd_value(x2, x3);
		int32 ty = rnd_value(y2, y3);
		pc_setpos(reinterpret_cast<TBL_PC*>(bl), index, tx, ty, CLR_OUTSIGHT);
	} else {
		pc_setpos(reinterpret_cast<TBL_PC*>(bl), index, x2, y2, CLR_OUTSIGHT);
	}
	return 0;
}

int32 lua_builtin_areawarp(lua_State* L) {
	const char* mapname = luaL_optstring(L, 1, "");
	int16 m = map_mapname2mapid(mapname);
	if (m < 0) {
		return 0;
	}
	int16 x0 = static_cast<int16>(luaL_optinteger(L, 2, 0));
	int16 y0 = static_cast<int16>(luaL_optinteger(L, 3, 0));
	int16 x1 = static_cast<int16>(luaL_optinteger(L, 4, 0));
	int16 y1 = static_cast<int16>(luaL_optinteger(L, 5, 0));
	const char* to_map = luaL_optstring(L, 6, "");
	int32 x2 = static_cast<int32>(luaL_optinteger(L, 7, 0));
	int32 y2 = static_cast<int32>(luaL_optinteger(L, 8, 0));
	int32 x3 = static_cast<int32>(luaL_optinteger(L, 9, 0));
	int32 y3 = static_cast<int32>(luaL_optinteger(L, 10, 0));
	uint32 index = (std::strcmp(to_map, "Random") == 0) ? 0 : mapindex_name2id(to_map);
	map_foreachinallarea(lua_builtin_areawarp_sub, m, x0, y0, x1, y1, BL_PC, index, x2, y2, x3, y3);
	return 0;
}

int32 lua_builtin_mapwarp_sub(block_list* bl, va_list ap) {
	uint32 index = va_arg(ap, uint32);
	int32 x = va_arg(ap, int32);
	int32 y = va_arg(ap, int32);
	pc_setpos(reinterpret_cast<TBL_PC*>(bl), index, x, y, CLR_OUTSIGHT);
	return 0;
}

int32 lua_builtin_mapwarp(lua_State* L) {
	const char* mapname = luaL_optstring(L, 1, "");
	const char* to_map = luaL_optstring(L, 2, "");
	int32 x = static_cast<int32>(luaL_optinteger(L, 3, 0));
	int32 y = static_cast<int32>(luaL_optinteger(L, 4, 0));
	int16 m = map_mapname2mapid(mapname);
	if (m < 0) {
		return 0;
	}
	map_foreachinmap(lua_builtin_mapwarp_sub, m, BL_PC, mapindex_name2id(to_map), x, y);
	return 0;
}

int32 lua_builtin_areapercentheal_sub(block_list* bl, va_list ap) {
	int32 hp = va_arg(ap, int32);
	int32 sp = va_arg(ap, int32);
	pc_percentheal(reinterpret_cast<TBL_PC*>(bl), hp, sp);
	return 0;
}

int32 lua_builtin_areapercentheal(lua_State* L) {
	const char* mapname = luaL_optstring(L, 1, "");
	int16 m = map_mapname2mapid(mapname);
	if (m < 0) {
		return 0;
	}
	int16 x0 = static_cast<int16>(luaL_optinteger(L, 2, 0));
	int16 y0 = static_cast<int16>(luaL_optinteger(L, 3, 0));
	int16 x1 = static_cast<int16>(luaL_optinteger(L, 4, 0));
	int16 y1 = static_cast<int16>(luaL_optinteger(L, 5, 0));
	int32 hp = static_cast<int32>(luaL_optinteger(L, 6, 0));
	int32 sp = static_cast<int32>(luaL_optinteger(L, 7, 0));
	map_foreachinallarea(lua_builtin_areapercentheal_sub, m, x0, y0, x1, y1, BL_PC, hp, sp);
	return 0;
}

int32 lua_builtin_setcell(lua_State* L) {
	int16 m = map_mapname2mapid(luaL_optstring(L, 1, ""));
	int16 x1 = static_cast<int16>(luaL_optinteger(L, 2, 0));
	int16 y1 = static_cast<int16>(luaL_optinteger(L, 3, 0));
	int16 x2 = static_cast<int16>(luaL_optinteger(L, 4, 0));
	int16 y2 = static_cast<int16>(luaL_optinteger(L, 5, 0));
	cell_t type = static_cast<cell_t>(luaL_optinteger(L, 6, 0));
	bool flag = lua_toboolean(L, 7) != 0;
	if (m < 0) {
		return 0;
	}
	if (x1 > x2) std::swap(x1, x2);
	if (y1 > y2) std::swap(y1, y2);
	for (int32 y = y1; y <= y2; ++y) {
		for (int32 x = x1; x <= x2; ++x) {
			map_setcell(m, x, y, type, flag);
		}
	}
	return 0;
}

int32 lua_builtin_setunitdata(lua_State* L) {
	int32 gid = static_cast<int32>(luaL_optinteger(L, 1, 0));
	int32 type = static_cast<int32>(luaL_optinteger(L, 2, 0));
	block_list* bl = map_id2bl(gid);
	if (bl == nullptr) {
		return 0;
	}
	unit_data* ud = unit_bl2ud(bl);
	int32 value = static_cast<int32>(luaL_optinteger(L, 3, 0));
	if (type == UMOB_LOOKDIR || type == UHOM_LOOKDIR || type == UPET_LOOKDIR || type == UMER_LOOKDIR ||
	    type == UELE_LOOKDIR || type == UNPC_LOOKDIR) {
		unit_setdir(bl, static_cast<uint8>(value));
	} else if (type == UMOB_CANMOVETICK || type == UHOM_CANMOVETICK || type == UPET_CANMOVETICK ||
	           type == UMER_CANMOVETICK || type == UELE_CANMOVETICK) {
		if (ud) ud->canmove_tick = value > 0 ? static_cast<uint32>(value) : 0;
	} else {
		ShowWarning("lua_engine: setunitdata: unsupported type %d for Lua path.\n", type);
	}
	return 0;
}

int32 lua_builtin_setunittitle(lua_State* L) {
	int32 gid = static_cast<int32>(luaL_optinteger(L, 1, 0));
	const char* title = luaL_optstring(L, 2, "");
	block_list* bl = map_id2bl(gid);
	if (bl == nullptr) {
		return 0;
	}
	unit_data* ud = unit_bl2ud(bl);
	if (ud == nullptr) {
		return 0;
	}
	safestrncpy(ud->title, title, NAME_LENGTH);
	clif_name_area(bl);
	return 0;
}

int32 lua_builtin_npcshopdelitem(lua_State* L) {
	if (!lua_isstring(L, 1)) {
		return 0;
	}
	npc_data* nd = npc_name2id(lua_tostring(L, 1));
	if (!nd || (nd->subtype != NPCTYPE_SHOP && nd->subtype != NPCTYPE_CASHSHOP && nd->subtype != NPCTYPE_ITEMSHOP &&
	            nd->subtype != NPCTYPE_POINTSHOP && nd->subtype != NPCTYPE_MARKETSHOP)) {
		return 0;
	}
	int32 size = nd->u.shop.count;
	for (int i = 2; i <= lua_gettop(L); ++i) {
		t_itemid nameid = static_cast<t_itemid>(luaL_optinteger(L, i, 0));
		int32 n = 0;
		ARR_FIND(0, size, n, nd->u.shop.shop_item[n].nameid == nameid);
		if (n < size) {
			if (n + 1 != size) {
				memmove(&nd->u.shop.shop_item[n], &nd->u.shop.shop_item[n + 1],
				        sizeof(nd->u.shop.shop_item[0]) * (size - (n + 1)));
			}
			--size;
		}
	}
	RECREATE(nd->u.shop.shop_item, struct npc_item_list, size);
	nd->u.shop.count = size;
	return 0;
}

int32 lua_builtin_npcshopupdate(lua_State* L) {
	const char* npcname = luaL_optstring(L, 1, "");
	t_itemid nameid = static_cast<t_itemid>(luaL_optinteger(L, 2, 0));
	int32 price = static_cast<int32>(luaL_optinteger(L, 3, 0));
	npc_data* nd = npc_name2id(npcname);
	if (!nd || (nd->subtype != NPCTYPE_SHOP && nd->subtype != NPCTYPE_CASHSHOP && nd->subtype != NPCTYPE_ITEMSHOP &&
	            nd->subtype != NPCTYPE_POINTSHOP && nd->subtype != NPCTYPE_MARKETSHOP)) {
		return 0;
	}
	for (int32 i = 0; i < nd->u.shop.count; ++i) {
		if (nd->u.shop.shop_item[i].nameid == nameid && price != 0) {
			nd->u.shop.shop_item[i].value = price;
		}
	}
	return 0;
}

int32 lua_builtin_movenpc(lua_State* L) {
	const char* npcname = luaL_optstring(L, 1, "");
	int32 x = static_cast<int32>(luaL_optinteger(L, 2, 0));
	int32 y = static_cast<int32>(luaL_optinteger(L, 3, 0));
	npc_data* nd = npc_name2id(npcname);
	if (nd == nullptr) {
		return 0;
	}
	if (lua_gettop(L) >= 4 && lua_isnumber(L, 4)) {
		nd->ud.dir = static_cast<uint8>(lua_tointeger(L, 4) % 8);
	}
	npc_movenpc(nd, x, y);
	return 0;
}

int32 lua_builtin_setwall(lua_State* L) {
	const char* mapname = luaL_optstring(L, 1, "");
	int32 x = static_cast<int32>(luaL_optinteger(L, 2, 0));
	int32 y = static_cast<int32>(luaL_optinteger(L, 3, 0));
	int32 size = static_cast<int32>(luaL_optinteger(L, 4, 0));
	int32 dir = static_cast<int32>(luaL_optinteger(L, 5, 0));
	bool shootable = lua_toboolean(L, 6) != 0;
	const char* wall_name = luaL_optstring(L, 7, "");
	int16 m = map_mapname2mapid(mapname);
	if (m >= 0) {
		map_iwall_set(m, x, y, size, dir, shootable, wall_name);
	}
	return 0;
}

int32 lua_builtin_flagemblem(lua_State* L) {
	int32 guild_id = static_cast<int32>(luaL_optinteger(L, 1, 0));
	npc_data* nd = map_id2nd(lua_current_execution() ? lua_current_execution()->oid : 0);
	if (nd == nullptr || nd->subtype != NPCTYPE_SCRIPT || guild_id < 0) {
		return 0;
	}
	bool changed = (nd->u.scr.guild_id != guild_id);
	nd->u.scr.guild_id = guild_id;
	clif_guild_emblem_area(nd);
	if (guild_id) {
		guild_flag_add(nd);
	} else if (changed) {
		guild_flag_remove(nd);
	}
	return 0;
}

int32 lua_builtin_npcspeed(lua_State* L) {
	int32 speed = static_cast<int32>(luaL_optinteger(L, 1, 0));
	npc_data* nd = lua_target_npc_by_name_or_context(L, 2);
	if (nd == nullptr || speed < MIN_WALK_SPEED || speed > MAX_WALK_SPEED) {
		return 0;
	}
	nd->speed = speed;
	return 0;
}

int32 lua_builtin_unitstopwalk(lua_State* L) {
	int32 unit_id = static_cast<int32>(luaL_optinteger(L, 1, 0));
	int32 flag = static_cast<int32>(luaL_optinteger(L, 2, USW_NONE));
	block_list* bl = map_id2bl(unit_id);
	if (bl == nullptr) {
		return 0;
	}
	unit_data* ud = unit_bl2ud(bl);
	if (ud != nullptr) {
		ud->state.force_walk = false;
	}
	unit_stop_walking(bl, flag);
	return 0;
}

int32 lua_builtin_unitwalk(lua_State* L) {
	int32 unit_id = static_cast<int32>(luaL_optinteger(L, 1, 0));
	block_list* bl = map_id2bl(unit_id);
	if (bl == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}
	int32 x = static_cast<int32>(luaL_optinteger(L, 2, bl->x));
	int32 y = static_cast<int32>(luaL_optinteger(L, 3, bl->y));
	int32 ok = unit_walktoxy(bl, static_cast<int16>(x), static_cast<int16>(y), 0);
	lua_pushinteger(L, ok ? 1 : 0);
	return 1;
}

int32 lua_builtin_noop(lua_State* L) {
	(void)L;
	return 0;
}

int32 lua_builtin_donpcevent(lua_State* L) {
	if (!lua_isstring(L, 1)) {
		return 0;
	}
	npc_event_do(lua_tostring(L, 1));
	return 0;
}

int32 lua_builtin_disablenpc(lua_State* L) {
	(void)L;
	LuaExecutionContext* execution = lua_current_execution();
	if (execution == nullptr || execution->oid <= 0) {
		return 0;
	}

	npc_data* nd = map_id2nd(execution->oid);
	if (nd != nullptr) {
		npc_enable_target(*nd, 0, NPCVIEW_DISABLE);
	}
	return 0;
}

int32 lua_builtin_enablenpc(lua_State* L) {
	(void)L;
	LuaExecutionContext* execution = lua_current_execution();
	if (execution == nullptr || execution->oid <= 0) {
		return 0;
	}

	npc_data* nd = map_id2nd(execution->oid);
	if (nd != nullptr) {
		npc_enable_target(*nd, 0, NPCVIEW_ENABLE);
	}
	return 0;
}

int32 lua_builtin_initnpctimer(lua_State* L) {
	(void)L;
	LuaExecutionContext* execution = lua_current_execution();
	if (execution == nullptr || execution->oid <= 0) {
		return 0;
	}

	npc_data* nd = map_id2nd(execution->oid);
	if (nd != nullptr && nd->subtype == NPCTYPE_SCRIPT) {
		npc_timerevent_start(nd, execution->rid);
	}
	return 0;
}

int32 lua_builtin_callsub(lua_State* L) {
	if (!lua_isstring(L, 1)) {
		return 0;
	}

	LuaExecutionContext* execution = lua_current_execution();
	if (execution == nullptr || execution->ctx_ref == LUA_NOREF) {
		return 0;
	}
	const int32 parent_ctx_ref = execution->ctx_ref;

	const std::string label = lua_normalize_key(lua_tostring(L, 1));
	int32 target_ref = LUA_NOREF;
	std::string scope_name = "<unknown>";

	if (execution->npc_id > 0) {
		auto runtime_it = g_npc_registry.find(execution->npc_id);
		if (runtime_it == g_npc_registry.end()) {
			return 0;
		}

		auto ref_it = runtime_it->second.label_refs.find(label);
		if (ref_it == runtime_it->second.label_refs.end()) {
			return 0;
		}

		target_ref = ref_it->second;
		scope_name = runtime_it->second.exname;
	} else if (!execution->function_name.empty()) {
		auto function_it = g_function_registry.find(lua_normalize_key(execution->function_name));
		if (function_it == g_function_registry.end()) {
			return 0;
		}

		auto ref_it = function_it->second.label_refs.find(label);
		if (ref_it == function_it->second.label_refs.end()) {
			return 0;
		}

		target_ref = ref_it->second;
		scope_name = function_it->second.name;
	} else {
		return 0;
	}

	const int argc = lua_gettop(L);

	LuaExecutionContext child;
	child.npc_id = execution->npc_id;
	child.rid = execution->rid;
	child.oid = execution->oid;
	child.ctx_ref = execution->ctx_ref;
	child.function_name = execution->function_name;
	child.mes_active = execution->mes_active;
	for (int i = 2; i <= argc; ++i) {
		lua_pushvalue(L, i);
		child.arg_refs.push_back(luaL_ref(L, LUA_REGISTRYINDEX));
	}

	g_execution_stack.push_back(std::move(child));
	int top = lua_gettop(L);
	lua_rawgeti(L, LUA_REGISTRYINDEX, target_ref);
	lua_rawgeti(L, LUA_REGISTRYINDEX, parent_ctx_ref);
	if (lua_pcall(L, 1, LUA_MULTRET, 0) != LUA_OK) {
		ShowError("lua_engine: callsub('%s') failed on scope '%s': %s\n",
		          label.c_str(), scope_name.c_str(), lua_tostring(L, -1));
		lua_settop(L, top);
	}

	const int32 returns = static_cast<int32>(lua_gettop(L) - top);
	LuaExecutionContext finished = std::move(g_execution_stack.back());
	g_execution_stack.pop_back();
	for (int32 arg_ref : finished.arg_refs) {
		lua_unref(arg_ref);
	}
	if (!g_execution_stack.empty()) {
		g_execution_stack.back().mes_active = finished.mes_active;
	}

	return returns;
}

int32 lua_builtin_getarg(lua_State* L) {
	LuaExecutionContext* execution = lua_current_execution();
	if (execution == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	int32 index = 0;
	if (lua_gettop(L) >= 1 && lua_isnumber(L, 1)) {
		index = static_cast<int32>(lua_tointeger(L, 1));
	}

	if (index >= 0 && static_cast<size_t>(index) < execution->arg_refs.size()) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, execution->arg_refs[static_cast<size_t>(index)]);
		return 1;
	}

	if (lua_gettop(L) >= 2) {
		lua_pushvalue(L, 2);
	} else {
		lua_pushinteger(L, 0);
	}
	return 1;
}

int32 lua_builtin_getargcount(lua_State* L) {
	(void)L;
	LuaExecutionContext* execution = lua_current_execution();
	if (execution == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}
	lua_pushinteger(L, static_cast<lua_Integer>(execution->arg_refs.size()));
	return 1;
}

int32 lua_builtin_callfunc(lua_State* L) {
	if (!lua_isstring(L, 1)) {
		return 0;
	}

	const std::string function_key = lua_normalize_key(lua_tostring(L, 1));
	auto function_it = g_function_registry.find(function_key);
	if (function_it == g_function_registry.end()) {
		return 0;
	}

	LuaExecutionContext* parent_execution = lua_current_execution();
	const int32 rid = parent_execution ? parent_execution->rid : 0;
	const int32 oid = parent_execution ? parent_execution->oid : 0;

	lua_newtable(L);
	lua_pushinteger(L, rid);
	lua_setfield(L, -2, "rid");
	lua_pushinteger(L, oid);
	lua_setfield(L, -2, "oid");
	lua_pushstring(L, function_it->second.name.c_str());
	lua_setfield(L, -2, "npc");

	lua_push_runtime_env_table(L);
	lua_setfield(L, -2, "__env");

	lua_pushcfunction(L, lua_ctx_env);
	lua_setfield(L, -2, "env");

	int32 ctx_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	LuaExecutionContext execution;
	execution.npc_id = 0;
	execution.rid = rid;
	execution.oid = oid;
	execution.ctx_ref = ctx_ref;
	execution.function_name = function_it->second.name;

	const int argc = lua_gettop(L);
	for (int i = 2; i <= argc; ++i) {
		lua_pushvalue(L, i);
		execution.arg_refs.push_back(luaL_ref(L, LUA_REGISTRYINDEX));
	}

	g_execution_stack.push_back(std::move(execution));
	const int top = lua_gettop(L);
	lua_rawgeti(L, LUA_REGISTRYINDEX, function_it->second.run_ref);
	lua_rawgeti(L, LUA_REGISTRYINDEX, ctx_ref);
	if (lua_pcall(L, 1, LUA_MULTRET, 0) != LUA_OK) {
		ShowError("lua_engine: callfunc('%s') failed: %s\n",
		          function_it->second.name.c_str(), lua_tostring(L, -1));
		lua_settop(L, top);
	}
	const int32 returns = static_cast<int32>(lua_gettop(L) - top);

	LuaExecutionContext finished = std::move(g_execution_stack.back());
	g_execution_stack.pop_back();
	for (int32 arg_ref : finished.arg_refs) {
		lua_unref(arg_ref);
	}
	luaL_unref(L, LUA_REGISTRYINDEX, finished.ctx_ref);
	return returns;
}

void lua_register_builtin(lua_State* L, const char* name, lua_CFunction func) {
	lua_pushcfunction(L, func);
	lua_setfield(L, -2, name);
}

void lua_register_scope_table(lua_State* L, const char* table_name, lua_CFunction getter,
                              lua_CFunction setter) {
	lua_newtable(L);
	lua_pushcfunction(L, getter);
	lua_setfield(L, -2, "get");
	lua_pushcfunction(L, setter);
	lua_setfield(L, -2, "set");
	lua_setfield(L, -2, table_name);
}

void lua_register_builtins(void) {
	if (g_lua_state == nullptr) {
		return;
	}

	if (g_builtin_table_ref != LUA_NOREF) {
		luaL_unref(g_lua_state, LUA_REGISTRYINDEX, g_builtin_table_ref);
		g_builtin_table_ref = LUA_NOREF;
	}

	lua_newtable(g_lua_state);
	lua_register_builtin(g_lua_state, "set", lua_builtin_set);
	lua_register_builtin(g_lua_state, "setd", lua_builtin_setd);
	lua_register_builtin(g_lua_state, "getd", lua_builtin_getd);
	lua_register_builtin(g_lua_state, "setarray", lua_builtin_setarray);
	lua_register_builtin(g_lua_state, "get_var", lua_builtin_get_var);
	lua_register_builtin(g_lua_state, "set_var", lua_builtin_set_var);
	lua_register_builtin(g_lua_state, "get_character_var", lua_builtin_get_character_var);
	lua_register_builtin(g_lua_state, "set_character_var", lua_builtin_set_character_var);
	lua_register_builtin(g_lua_state, "get_character_temp_var", lua_builtin_get_character_temp_var);
	lua_register_builtin(g_lua_state, "set_character_temp_var", lua_builtin_set_character_temp_var);
	lua_register_builtin(g_lua_state, "get_account_var", lua_builtin_get_account_var);
	lua_register_builtin(g_lua_state, "set_account_var", lua_builtin_set_account_var);
	lua_register_builtin(g_lua_state, "get_account_global_var", lua_builtin_get_account_global_var);
	lua_register_builtin(g_lua_state, "set_account_global_var", lua_builtin_set_account_global_var);
	lua_register_builtin(g_lua_state, "get_map_server_var", lua_builtin_get_map_server_var);
	lua_register_builtin(g_lua_state, "set_map_server_var", lua_builtin_set_map_server_var);
	lua_register_builtin(g_lua_state, "get_npc_var", lua_builtin_get_npc_var);
	lua_register_builtin(g_lua_state, "set_npc_var", lua_builtin_set_npc_var);
	lua_register_builtin(g_lua_state, "get_npc_temp_var", lua_builtin_get_npc_temp_var);
	lua_register_builtin(g_lua_state, "set_npc_temp_var", lua_builtin_set_npc_temp_var);
	lua_register_builtin(g_lua_state, "get_instance_var", lua_builtin_get_instance_var);
	lua_register_builtin(g_lua_state, "set_instance_var", lua_builtin_set_instance_var);
	lua_register_scope_table(g_lua_state, "var", lua_builtin_get_var, lua_builtin_set_var);
	lua_register_scope_table(g_lua_state, "character", lua_builtin_get_character_var,
	                         lua_builtin_set_character_var);
	lua_register_scope_table(g_lua_state, "character_temp", lua_builtin_get_character_temp_var,
	                         lua_builtin_set_character_temp_var);
	lua_register_scope_table(g_lua_state, "account", lua_builtin_get_account_var,
	                         lua_builtin_set_account_var);
	lua_register_scope_table(g_lua_state, "account_global", lua_builtin_get_account_global_var,
	                         lua_builtin_set_account_global_var);
	lua_register_scope_table(g_lua_state, "map_server", lua_builtin_get_map_server_var,
	                         lua_builtin_set_map_server_var);
	lua_register_scope_table(g_lua_state, "npc_var", lua_builtin_get_npc_var,
	                         lua_builtin_set_npc_var);
	lua_register_scope_table(g_lua_state, "npc_temp", lua_builtin_get_npc_temp_var,
	                         lua_builtin_set_npc_temp_var);
	lua_register_scope_table(g_lua_state, "instance", lua_builtin_get_instance_var,
	                         lua_builtin_set_instance_var);
	lua_register_builtin(g_lua_state, "mes", lua_builtin_mes);
	lua_register_builtin(g_lua_state, "next", lua_builtin_next);
	lua_register_builtin(g_lua_state, "clear", lua_builtin_clear);
	lua_register_builtin(g_lua_state, "close", lua_builtin_close);
	lua_register_builtin(g_lua_state, "close2", lua_builtin_close2);
	lua_register_builtin(g_lua_state, "close3", lua_builtin_close3);
	lua_register_builtin(g_lua_state, "select", lua_builtin_select);
	lua_register_builtin(g_lua_state, "menu", lua_builtin_menu);
	lua_register_builtin(g_lua_state, "input", lua_builtin_input);
	lua_register_builtin(g_lua_state, "sleep", lua_builtin_sleep);
	lua_register_builtin(g_lua_state, "sleep2", lua_builtin_sleep2);
	lua_register_builtin(g_lua_state, "rand", lua_builtin_rand);
	lua_register_builtin(g_lua_state, "strcharinfo", lua_builtin_strcharinfo);
	lua_register_builtin(g_lua_state, "strnpcinfo", lua_builtin_strnpcinfo);
	lua_register_builtin(g_lua_state, "compare", lua_builtin_compare);
	lua_register_builtin(g_lua_state, "checkweight", lua_builtin_checkweight);
	lua_register_builtin(g_lua_state, "checkcell", lua_builtin_checkcell);
	lua_register_builtin(g_lua_state, "countitem", lua_builtin_countitem);
	lua_register_builtin(g_lua_state, "checkre", lua_builtin_checkre);
	lua_register_builtin(g_lua_state, "getarraysize", lua_builtin_getarraysize);
	lua_register_builtin(g_lua_state, "charat", lua_builtin_charat);
	lua_register_builtin(g_lua_state, "getstrlen", lua_builtin_getstrlen);
	lua_register_builtin(g_lua_state, "gettime", lua_builtin_gettime);
	lua_register_builtin(g_lua_state, "replacestr", lua_builtin_replacestr);
	lua_register_builtin(g_lua_state, "agitcheck", lua_builtin_agitcheck);
	lua_register_builtin(g_lua_state, "agitcheck3", lua_builtin_agitcheck3);
	lua_register_builtin(g_lua_state, "getcastledata", lua_builtin_getcastledata);
	lua_register_builtin(g_lua_state, "setcastledata", lua_builtin_setcastledata);
	lua_register_builtin(g_lua_state, "donpcevent", lua_builtin_donpcevent);
	lua_register_builtin(g_lua_state, "disablenpc", lua_builtin_disablenpc);
	lua_register_builtin(g_lua_state, "enablenpc", lua_builtin_enablenpc);
	lua_register_builtin(g_lua_state, "initnpctimer", lua_builtin_initnpctimer);
	lua_register_builtin(g_lua_state, "Initnpctimer", lua_builtin_initnpctimer);
	lua_register_builtin(g_lua_state, "callsub", lua_builtin_callsub);
	lua_register_builtin(g_lua_state, "callfunc", lua_builtin_callfunc);
	lua_register_builtin(g_lua_state, "getarg", lua_builtin_getarg);
	lua_register_builtin(g_lua_state, "getargcount", lua_builtin_getargcount);
	lua_register_builtin(g_lua_state, "mapannounce", lua_builtin_mapannounce);
	lua_register_builtin(g_lua_state, "warp", lua_builtin_warp);
	lua_register_builtin(g_lua_state, "getitem", lua_builtin_getitem);
	lua_register_builtin(g_lua_state, "delitem", lua_builtin_delitem);
	lua_register_builtin(g_lua_state, "announce", lua_builtin_announce);
	lua_register_builtin(g_lua_state, "cutin", lua_builtin_cutin);
	lua_register_builtin(g_lua_state, "specialeffect", lua_builtin_specialeffect);
	lua_register_builtin(g_lua_state, "specialeffect2", lua_builtin_specialeffect2);
	lua_register_builtin(g_lua_state, "getexp", lua_builtin_getexp);
	lua_register_builtin(g_lua_state, "atoi", lua_builtin_atoi);
	lua_register_builtin(g_lua_state, "getmonsterinfo", lua_builtin_getmonsterinfo);
	lua_register_builtin(g_lua_state, "getnpcid", lua_builtin_getnpcid);
	lua_register_builtin(g_lua_state, "sc_start2", lua_builtin_sc_start2);
	lua_register_builtin(g_lua_state, "emotion", lua_builtin_emotion);
	lua_register_builtin(g_lua_state, "npctalk", lua_builtin_npctalk);
	lua_register_builtin(g_lua_state, "dispbottom", lua_builtin_dispbottom);
	lua_register_builtin(g_lua_state, "cleararray", lua_builtin_cleararray);
	lua_register_builtin(g_lua_state, "questinfo", lua_builtin_questinfo);
	lua_register_builtin(g_lua_state, "waitingroom", lua_builtin_waitingroom);
	lua_register_builtin(g_lua_state, "hideonnpc", lua_builtin_hideonnpc);
	lua_register_builtin(g_lua_state, "hideoffnpc", lua_builtin_hideoffnpc);
	lua_register_builtin(g_lua_state, "unloadnpc", lua_builtin_unloadnpc);
	lua_register_builtin(g_lua_state, "stopnpctimer", lua_builtin_stopnpctimer);
	lua_register_builtin(g_lua_state, "setnpctimer", lua_builtin_setnpctimer);
	lua_register_builtin(g_lua_state, "startnpctimer", lua_builtin_startnpctimer);
	lua_register_builtin(g_lua_state, "cloakonnpc", lua_builtin_cloakonnpc);
	lua_register_builtin(g_lua_state, "cloakoffnpc", lua_builtin_cloakoffnpc);
	lua_register_builtin(g_lua_state, "enablewaitingroomevent", lua_builtin_enablewaitingroomevent);
	lua_register_builtin(g_lua_state, "disablewaitingroomevent", lua_builtin_disablewaitingroomevent);
	lua_register_builtin(g_lua_state, "monster", lua_builtin_monster);
	lua_register_builtin(g_lua_state, "areamonster", lua_builtin_areamonster);
	lua_register_builtin(g_lua_state, "killmonster", lua_builtin_killmonster);
	lua_register_builtin(g_lua_state, "bg_unbook", lua_builtin_bg_unbook);
	lua_register_builtin(g_lua_state, "removemapflag", lua_builtin_removemapflag);
	lua_register_builtin(g_lua_state, "areawarp", lua_builtin_areawarp);
	lua_register_builtin(g_lua_state, "mapwarp", lua_builtin_mapwarp);
	lua_register_builtin(g_lua_state, "areapercentheal", lua_builtin_areapercentheal);
	lua_register_builtin(g_lua_state, "setcell", lua_builtin_setcell);
	lua_register_builtin(g_lua_state, "setunitdata", lua_builtin_setunitdata);
	lua_register_builtin(g_lua_state, "setunittitle", lua_builtin_setunittitle);
	lua_register_builtin(g_lua_state, "npcshopdelitem", lua_builtin_npcshopdelitem);
	lua_register_builtin(g_lua_state, "npcshopupdate", lua_builtin_npcshopupdate);
	lua_register_builtin(g_lua_state, "movenpc", lua_builtin_movenpc);
	lua_register_builtin(g_lua_state, "setwall", lua_builtin_setwall);
	lua_register_builtin(g_lua_state, "flagemblem", lua_builtin_flagemblem);
	lua_register_builtin(g_lua_state, "npcspeed", lua_builtin_npcspeed);
	lua_register_builtin(g_lua_state, "unitstopwalk", lua_builtin_unitstopwalk);
	lua_register_builtin(g_lua_state, "unitwalk", lua_builtin_unitwalk);

	static const char* const noop_builtins[] = {
	    "end",
	};
	for (const char* name : noop_builtins) {
		lua_register_builtin(g_lua_state, name, lua_builtin_noop);
	}
	lua_register_builtin(g_lua_state, "getbattleflag", lua_builtin_getbattleflag);
	lua_register_builtin(g_lua_state, "getvariableofnpc", lua_builtin_getvariableofnpc);

	g_builtin_table_ref = luaL_ref(g_lua_state, LUA_REGISTRYINDEX);
}

TIMER_FUNC(lua_instance_resume_timer) {
	(void)tid;
	(void)tick;
	(void)id;

	uint32 instance_id = static_cast<uint32>(data);
	auto it = g_instances.find(instance_id);
	if (it != g_instances.end()) {
		it->second.timer_id = INVALID_TIMER;
	}
	lua_engine_resume_instance(instance_id);
	return 0;
}

bool lua_mark_instance_waiting(LuaRuntimeInstance& instance) {
	if (instance.timer_id != INVALID_TIMER) {
		delete_timer(instance.timer_id, lua_instance_resume_timer);
		instance.timer_id = INVALID_TIMER;
	}
	lua_remove_pending_dialog(instance.execution);

	switch (instance.execution.wait_type) {
		case LuaWaitType::Next:
		case LuaWaitType::Menu:
		case LuaWaitType::Input:
		case LuaWaitType::Close2:
		case LuaWaitType::CloseEnd:
			lua_set_pending_dialog(instance.id, instance.execution);
			return true;
		case LuaWaitType::Sleep: {
			int32 sleep_tick = instance.execution.sleep_tick;
			if (sleep_tick <= 0) {
				sleep_tick = 1;
			}
			instance.timer_id = add_timer(gettick() + sleep_tick, lua_instance_resume_timer,
			                              instance.execution.rid, static_cast<intptr_t>(instance.id));
			return instance.timer_id != INVALID_TIMER;
		}
		case LuaWaitType::None:
		default:
			return false;
	}
}

bool lua_resume_instance_internal(uint32 instance_id, int32 nargs) {
	auto instance_it = g_instances.find(instance_id);
	if (instance_it == g_instances.end()) {
		return false;
	}

	LuaRuntimeInstance& instance = instance_it->second;
	lua_State* co = lua_instance_thread(instance.thread_ref);
	if (co == nullptr) {
		ShowError("lua_engine: missing coroutine for NPC '%s' instance %u.\n",
		          instance.runtime_name.c_str(), instance_id);
		lua_erase_instance(instance_id, true);
		return false;
	}

	g_execution_stack.push_back(instance.execution);

	int32 result_count = 0;
	const int32 status = lua_resume(co, g_lua_state, nargs, &result_count);

	if (g_execution_stack.empty()) {
		ShowError("lua_engine: execution stack underflow while resuming NPC '%s' instance %u.\n",
		          instance.runtime_name.c_str(), instance_id);
		lua_erase_instance(instance_id, true);
		return false;
	}

	instance.execution = std::move(g_execution_stack.back());
	g_execution_stack.pop_back();

	if (status == LUA_OK) {
		if (result_count > 0) {
			lua_pop(co, result_count);
		}
		lua_erase_instance(instance_id, true);
		return true;
	}

	if (status == LUA_YIELD) {
		if (result_count > 0) {
			lua_pop(co, result_count);
		}
		if (!lua_mark_instance_waiting(instance)) {
			ShowError("lua_engine: script on NPC '%s' yielded without wait state.\n",
			          instance.runtime_name.c_str());
			lua_erase_instance(instance_id, true);
			return false;
		}
		return true;
	}

	if (lua_gettop(co) <= 0) {
		ShowError("lua_engine: runtime error on NPC '%s': lua_resume status=%d with empty stack.\n",
		          instance.runtime_name.c_str(), status);
		lua_erase_instance(instance_id, true);
		return false;
	}

	const std::string err = lua_value_debug_string(co, -1);
	const std::string tb = lua_traceback_string(co, err);
	ShowError("lua_engine: runtime error on NPC '%s': %s\n",
	          instance.runtime_name.c_str(), tb.c_str());
	lua_pop(co, 1);
	lua_erase_instance(instance_id, true);
	return false;
}

bool lua_run_ref(const LuaNpcRuntime& runtime, int32 function_ref, int32 rid, int32 oid) {
	if (g_lua_state == nullptr || function_ref == LUA_NOREF || function_ref == LUA_REFNIL) {
		return false;
	}
	lua_State* co = lua_newthread(g_lua_state);
	if (co == nullptr) {
		ShowError("lua_engine: failed to allocate Lua coroutine for NPC '%s'.\n", runtime.exname.c_str());
		return false;
	}
	int32 thread_ref = luaL_ref(g_lua_state, LUA_REGISTRYINDEX);

	lua_newtable(g_lua_state);
	lua_pushinteger(g_lua_state, rid);
	lua_setfield(g_lua_state, -2, "rid");
	lua_pushinteger(g_lua_state, oid);
	lua_setfield(g_lua_state, -2, "oid");
	lua_pushstring(g_lua_state, runtime.exname.c_str());
	lua_setfield(g_lua_state, -2, "npc");

	lua_push_runtime_env_table(g_lua_state);
	lua_setfield(g_lua_state, -2, "__env");

	lua_pushcfunction(g_lua_state, lua_ctx_env);
	lua_setfield(g_lua_state, -2, "env");

	int32 ctx_ref = luaL_ref(g_lua_state, LUA_REGISTRYINDEX);

	uint32 instance_id = 0;
	do {
		instance_id = g_next_instance_id++;
		if (g_next_instance_id == 0) {
			g_next_instance_id = 1;
		}
	} while (instance_id == 0 || g_instances.find(instance_id) != g_instances.end());

	LuaRuntimeInstance instance;
	instance.id = instance_id;
	instance.thread_ref = thread_ref;
	instance.runtime_name = runtime.exname;
	instance.execution.npc_id = runtime.npc_id;
	instance.execution.rid = rid;
	instance.execution.oid = oid;
	instance.execution.ctx_ref = ctx_ref;
	lua_attach_player(instance.execution);

	g_instances[instance_id] = std::move(instance);

	lua_rawgeti(co, LUA_REGISTRYINDEX, function_ref);
	lua_rawgeti(co, LUA_REGISTRYINDEX, ctx_ref);
	return lua_resume_instance_internal(instance_id, 1);
}

int32 lua_resolve_ref(const LuaNpcRuntime& runtime, const std::string& label) {
	const auto event_it = runtime.event_refs.find(label);
	if (event_it != runtime.event_refs.end()) {
		return event_it->second;
	}

	const auto label_it = runtime.label_refs.find(label);
	if (label_it != runtime.label_refs.end()) {
		return label_it->second;
	}

	return LUA_NOREF;
}
#endif
} // namespace

bool lua_engine_init(void) {
#if defined(HAVE_LUA) || defined(WITH_LUA)
	if (g_lua_state != nullptr) {
		return true;
	}

	g_lua_state = luaL_newstate();
	if (g_lua_state == nullptr) {
		ShowError("lua_engine_init: failed to create Lua state.\n");
		return false;
	}

	luaL_openlibs(g_lua_state);
	if (!g_timer_registered) {
		add_timer_func_list(lua_instance_resume_timer, "lua_instance_resume_timer");
		g_timer_registered = true;
	}
	lua_register_builtins();
	return true;
#else
	ShowError("lua_engine_init: Lua support is disabled at build time.\n");
	return false;
#endif
}

void lua_engine_final(void) {
#if defined(HAVE_LUA) || defined(WITH_LUA)
	if (g_lua_state == nullptr) {
		return;
	}

	if (g_loaded_table_ref != LUA_NOREF) {
		luaL_unref(g_lua_state, LUA_REGISTRYINDEX, g_loaded_table_ref);
		g_loaded_table_ref = LUA_NOREF;
	}

	lua_engine_clear_npc_registry();
	lua_engine_clear_function_registry();

	if (g_builtin_table_ref != LUA_NOREF) {
		luaL_unref(g_lua_state, LUA_REGISTRYINDEX, g_builtin_table_ref);
		g_builtin_table_ref = LUA_NOREF;
	}

	lua_close(g_lua_state);
	g_lua_state = nullptr;
#endif
}

bool lua_engine_load_file(const std::string& filepath) {
#if defined(HAVE_LUA) || defined(WITH_LUA)
	if (!lua_engine_init()) {
		return false;
	}

	if (luaL_loadfile(g_lua_state, filepath.c_str()) != LUA_OK) {
		ShowError("lua_engine_load_file: failed to load '%s': %s\n",
		          filepath.c_str(), lua_tostring(g_lua_state, -1));
		lua_pop(g_lua_state, 1);
		return false;
	}

	// Bind chunk _ENV to runtime environment so script handlers can use
	// exported gameplay functions without declaring `local _ENV = ctx:env()`.
	lua_push_runtime_env_table(g_lua_state);      // ..., chunk, env
	lua_pushvalue(g_lua_state, -1);               // ..., chunk, env, env
	const char* upvalue_name = lua_setupvalue(g_lua_state, -3, 1); // pops one env copy
	if (upvalue_name == nullptr || std::strcmp(upvalue_name, "_ENV") != 0) {
		ShowError("lua_engine_load_file: failed to bind chunk environment for '%s'.\n",
		          filepath.c_str());
		lua_pop(g_lua_state, 2); // env + chunk
		return false;
	}
	lua_pop(g_lua_state, 1); // pop env; keep chunk on stack top

	if (lua_pcall(g_lua_state, 0, 1, 0) != LUA_OK) {
		ShowError("lua_engine_load_file: failed to execute '%s': %s\n",
		          filepath.c_str(), lua_tostring(g_lua_state, -1));
		lua_pop(g_lua_state, 1);
		return false;
	}

	if (!lua_istable(g_lua_state, -1)) {
		ShowError("lua_engine_load_file: '%s' must return a table.\n", filepath.c_str());
		lua_pop(g_lua_state, 1);
		return false;
	}

	if (g_loaded_table_ref != LUA_NOREF) {
		luaL_unref(g_lua_state, LUA_REGISTRYINDEX, g_loaded_table_ref);
		g_loaded_table_ref = LUA_NOREF;
	}

	g_loaded_table_ref = luaL_ref(g_lua_state, LUA_REGISTRYINDEX);
	return true;
#else
	ShowError("lua_engine_load_file: Lua support is disabled (requested file: %s).\n", filepath.c_str());
	return false;
#endif
}

bool lua_engine_register_npc(const LuaNpcDef& def) {
#if defined(HAVE_LUA) || defined(WITH_LUA)
	if (g_lua_state == nullptr && !lua_engine_init()) {
		return false;
	}

	if (def.npc_id <= 0 || def.exname.empty()) {
		return false;
	}

	lua_engine_unregister_npc(def.npc_id);

	LuaNpcRuntime runtime;
	runtime.npc_id = def.npc_id;
	runtime.exname = def.exname;
	runtime.main_ref = lua_clone_ref(def.main_ref);

	for (const LuaNpcDef::LuaLabelBinding& binding : def.events) {
		const std::string key = lua_normalize_key(binding.name);
		int32 ref = lua_clone_ref(binding.ref);
		if (ref == LUA_NOREF || ref == LUA_REFNIL) {
			continue;
		}
		runtime.event_refs[key] = ref;
		if (binding.pos >= 0) {
			runtime.pos_to_label[binding.pos] = key;
		}
	}

	for (const LuaNpcDef::LuaLabelBinding& binding : def.labels) {
		const std::string key = lua_normalize_key(binding.name);
		int32 ref = lua_clone_ref(binding.ref);
		if (ref == LUA_NOREF || ref == LUA_REFNIL) {
			continue;
		}
		runtime.label_refs[key] = ref;
		if (binding.pos >= 0) {
			runtime.pos_to_label[binding.pos] = key;
		}
	}

	g_exname_registry[lua_normalize_key(def.exname)] = def.npc_id;
	g_npc_registry[def.npc_id] = std::move(runtime);
	return true;
#else
	(void)def;
	return false;
#endif
}

bool lua_engine_register_function(const LuaFunctionDef& def) {
#if defined(HAVE_LUA) || defined(WITH_LUA)
	if (g_lua_state == nullptr && !lua_engine_init()) {
		return false;
	}

	if (def.name.empty() || def.run_ref == LUA_NOREF || def.run_ref == LUA_REFNIL) {
		return false;
	}

	const std::string key = lua_normalize_key(def.name);
	auto existing_it = g_function_registry.find(key);
	if (existing_it != g_function_registry.end()) {
		lua_release_function_runtime(existing_it->second);
		g_function_registry.erase(existing_it);
	}

	LuaFunctionRuntime runtime;
	runtime.name = def.name;
	runtime.run_ref = lua_clone_ref(def.run_ref);
	for (const LuaNpcDef::LuaLabelBinding& binding : def.labels) {
		const std::string label_key = lua_normalize_key(binding.name);
		int32 ref = lua_clone_ref(binding.ref);
		if (ref == LUA_NOREF || ref == LUA_REFNIL) {
			continue;
		}
		runtime.label_refs[label_key] = ref;
	}

	g_function_registry[key] = std::move(runtime);
	return true;
#else
	(void)def;
	return false;
#endif
}

bool lua_engine_run_event(const char* event_name, int32 rid, int32 oid) {
#if defined(HAVE_LUA) || defined(WITH_LUA)
	if (g_lua_state == nullptr || event_name == nullptr || event_name[0] == '\0') {
		return false;
	}

	std::string exname;
	std::string label;
	if (!lua_read_event_name(event_name, exname, label)) {
		return false;
	}

	const LuaNpcRuntime* runtime = nullptr;
	if (oid > 0) {
		auto it = g_npc_registry.find(oid);
		if (it != g_npc_registry.end()) {
			runtime = &it->second;
		}
	}

	if (runtime == nullptr && !exname.empty()) {
		auto id_it = g_exname_registry.find(exname);
		if (id_it != g_exname_registry.end()) {
			auto runtime_it = g_npc_registry.find(id_it->second);
			if (runtime_it != g_npc_registry.end()) {
				runtime = &runtime_it->second;
			}
		}
	}

	if (runtime == nullptr) {
		return false;
	}

	int32 ref = lua_resolve_ref(*runtime, label);
	if (ref == LUA_NOREF || ref == LUA_REFNIL) {
		return false;
	}
	return lua_run_ref(*runtime, ref, rid, oid);
#else
	(void)event_name;
	(void)rid;
	(void)oid;
	return false;
#endif
}

bool lua_engine_run_npc(int32 npc_id, const char* exname, int32 pos, int32 rid, int32 oid) {
#if defined(HAVE_LUA) || defined(WITH_LUA)
	if (g_lua_state == nullptr) {
		return false;
	}

	const LuaNpcRuntime* runtime = nullptr;
	auto runtime_it = g_npc_registry.find(npc_id);
	if (runtime_it != g_npc_registry.end()) {
		runtime = &runtime_it->second;
	} else if (exname != nullptr && exname[0] != '\0') {
		auto id_it = g_exname_registry.find(lua_normalize_key(exname));
		if (id_it != g_exname_registry.end()) {
			runtime_it = g_npc_registry.find(id_it->second);
			if (runtime_it != g_npc_registry.end()) {
				runtime = &runtime_it->second;
			}
		}
	}

	if (runtime == nullptr) {
		return false;
	}

	int32 ref = LUA_NOREF;
	if (pos == 0) {
		ref = runtime->main_ref;
	} else {
		auto label_it = runtime->pos_to_label.find(pos);
		if (label_it != runtime->pos_to_label.end()) {
			ref = lua_resolve_ref(*runtime, label_it->second);
		}
	}

	if (ref == LUA_NOREF || ref == LUA_REFNIL) {
		return false;
	}

	return lua_run_ref(*runtime, ref, rid, oid);
#else
	(void)npc_id;
	(void)exname;
	(void)pos;
	(void)rid;
	(void)oid;
	return false;
#endif
}

void lua_engine_unregister_npc(int32 npc_id) {
#if defined(HAVE_LUA) || defined(WITH_LUA)
	auto it = g_npc_registry.find(npc_id);
	if (it == g_npc_registry.end()) {
		return;
	}

	if (!it->second.exname.empty()) {
		auto ex_it = g_exname_registry.find(lua_normalize_key(it->second.exname));
		if (ex_it != g_exname_registry.end() && ex_it->second == npc_id) {
			g_exname_registry.erase(ex_it);
		}
	}

	lua_release_runtime(it->second);
	g_npc_registry.erase(it);
#else
	(void)npc_id;
#endif
}

void lua_engine_clone_npc(int32 source_npc_id, int32 target_npc_id, const char* target_exname) {
#if defined(HAVE_LUA) || defined(WITH_LUA)
	if (g_lua_state == nullptr || target_npc_id <= 0) {
		return;
	}

	auto source_it = g_npc_registry.find(source_npc_id);
	if (source_it == g_npc_registry.end()) {
		return;
	}

	lua_engine_unregister_npc(target_npc_id);

	LuaNpcRuntime clone;
	clone.npc_id = target_npc_id;
	clone.exname = target_exname != nullptr ? target_exname : source_it->second.exname;
	clone.main_ref = lua_clone_ref(source_it->second.main_ref);
	clone.pos_to_label = source_it->second.pos_to_label;

	for (const auto& pair : source_it->second.event_refs) {
		int32 ref = lua_clone_ref(pair.second);
		if (ref != LUA_NOREF && ref != LUA_REFNIL) {
			clone.event_refs[pair.first] = ref;
		}
	}

	for (const auto& pair : source_it->second.label_refs) {
		int32 ref = lua_clone_ref(pair.second);
		if (ref != LUA_NOREF && ref != LUA_REFNIL) {
			clone.label_refs[pair.first] = ref;
		}
	}

	g_exname_registry[lua_normalize_key(clone.exname)] = target_npc_id;
	g_npc_registry[target_npc_id] = std::move(clone);
#else
	(void)source_npc_id;
	(void)target_npc_id;
	(void)target_exname;
#endif
}

void lua_engine_clear_npc_registry(void) {
#if defined(HAVE_LUA) || defined(WITH_LUA)
	while (!g_instances.empty()) {
		lua_erase_instance(g_instances.begin()->first, false);
	}
	g_pending_dialog_instances.clear();

	for (auto& pair : g_npc_registry) {
		lua_release_runtime(pair.second);
	}
	g_npc_registry.clear();
	g_exname_registry.clear();
	g_var_registry.clear();
	g_execution_stack.clear();
#endif
}

void lua_engine_clear_function_registry(void) {
#if defined(HAVE_LUA) || defined(WITH_LUA)
	for (auto& pair : g_function_registry) {
		lua_release_function_runtime(pair.second);
	}
	g_function_registry.clear();
#endif
}

bool lua_engine_resume_instance(uint32 instance_id) {
#if defined(HAVE_LUA) || defined(WITH_LUA)
	if (g_lua_state == nullptr) {
		return false;
	}

	auto instance_it = g_instances.find(instance_id);
	if (instance_it == g_instances.end()) {
		return false;
	}

	lua_remove_pending_dialog(instance_it->second.execution);
	lua_clear_wait_state(instance_it->second.execution);
	return lua_resume_instance_internal(instance_id, 0);
#else
	(void)instance_id;
	return false;
#endif
}

bool lua_engine_continue_dialog(map_session_data* sd, int32 npc_id, bool closing) {
#if defined(HAVE_LUA) || defined(WITH_LUA)
	if (g_lua_state == nullptr || sd == nullptr || npc_id <= 0) {
		return false;
	}

	const uint64 key = lua_dialog_key(sd->id, npc_id);
	auto pending_it = g_pending_dialog_instances.find(key);
	if (pending_it == g_pending_dialog_instances.end()) {
		return false;
	}

	const uint32 instance_id = pending_it->second;
	auto instance_it = g_instances.find(instance_id);
	if (instance_it == g_instances.end()) {
		g_pending_dialog_instances.erase(pending_it);
		return true;
	}

	LuaRuntimeInstance& instance = instance_it->second;
	lua_State* co = lua_instance_thread(instance.thread_ref);
	if (co == nullptr) {
		lua_erase_instance(instance_id, true);
		return true;
	}

	LuaWaitType wait_type = instance.execution.wait_type;
	if (wait_type == LuaWaitType::CloseEnd) {
		lua_erase_instance(instance_id, true);
		return true;
	}

	if (closing && wait_type != LuaWaitType::Close2) {
		lua_erase_instance(instance_id, true);
		return true;
	}

	int32 nargs = 0;

	if (wait_type == LuaWaitType::Menu) {
		int32 selection = sd->npc_menu;
		if (closing) {
			selection = 0xff;
		}
		lua_pushinteger(co, selection);
		nargs = 1;
		sd->npc_menu = 0;
		sd->state.menu_or_input = 0;
	} else if (wait_type == LuaWaitType::Input) {
		int32 result = 0;
		if (instance.execution.input_is_string) {
			std::string value = closing ? "" : sd->npc_str;
			const int32 len = static_cast<int32>(value.length());
			if (len > instance.execution.input_max) {
				result = 1;
			} else if (len < instance.execution.input_min) {
				result = -1;
			}
			lua_pushstring(co, value.c_str());
			lua_store_variable(instance.execution.input_var, co, -1);
			lua_pop(co, 1);
			sd->npc_str[0] = '\0';
		} else {
			int32 amount = closing ? 0 : sd->npc_amount;
			if (amount > instance.execution.input_max) {
				result = 1;
			} else if (amount < instance.execution.input_min) {
				result = -1;
			}

			int32 capped = amount;
			if (capped > instance.execution.input_max) {
				capped = instance.execution.input_max;
			}
			if (capped < instance.execution.input_min) {
				capped = instance.execution.input_min;
			}

			lua_pushinteger(co, capped);
			lua_store_variable(instance.execution.input_var, co, -1);
			lua_pop(co, 1);
			sd->npc_amount = 0;
		}
		lua_pushinteger(co, result);
		nargs = 1;
		sd->state.menu_or_input = 0;
	}

	lua_remove_pending_dialog(instance.execution);
	lua_clear_wait_state(instance.execution);
	return lua_resume_instance_internal(instance_id, nargs);
#else
	(void)sd;
	(void)npc_id;
	(void)closing;
	return false;
#endif
}

lua_State* lua_engine_get_state(void) {
#if defined(HAVE_LUA) || defined(WITH_LUA)
	return g_lua_state;
#else
	return nullptr;
#endif
}

bool lua_engine_push_loaded_table(void) {
#if defined(HAVE_LUA) || defined(WITH_LUA)
	if (g_lua_state == nullptr || g_loaded_table_ref == LUA_NOREF) {
		return false;
	}

	lua_rawgeti(g_lua_state, LUA_REGISTRYINDEX, g_loaded_table_ref);
	return lua_istable(g_lua_state, -1);
#else
	return false;
#endif
}
