// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npc_lua.hpp"

#include "lua_engine.hpp"
#include "mob.hpp"
#include "npc.hpp"
#include "script.hpp"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#if !defined(_WIN32)
#include <unistd.h>
#endif

#include <common/showmsg.hpp>
#include <common/strlib.hpp>

#if defined(HAVE_LUA) || defined(WITH_LUA)
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

namespace {

constexpr int32 NPC_LUA_FIELD_BUFFER = 1024;

void append_with_newline(std::string& out, const std::string& value) {
	if (value.empty()) {
		return;
	}

	out.append(value);
	if (out.back() != '\n') {
		out.push_back('\n');
	}
}

std::string table_get_string(lua_State* L, int index, const char* key, const std::string& def = "", bool* found = nullptr) {
	std::string out = def;

	lua_getfield(L, index, key);
	if (lua_isstring(L, -1)) {
		out = lua_tostring(L, -1);
		if (found != nullptr) {
			*found = true;
		}
	} else if (found != nullptr) {
		*found = false;
	}
	lua_pop(L, 1);

	return out;
}

int32 table_get_int(lua_State* L, int index, const char* key, int32 def = 0, bool* found = nullptr) {
	int32 out = def;

	lua_getfield(L, index, key);
	if (lua_isinteger(L, -1) || lua_isnumber(L, -1)) {
		out = static_cast<int32>(lua_tointeger(L, -1));
		if (found != nullptr) {
			*found = true;
		}
	} else if (found != nullptr) {
		*found = false;
	}
	lua_pop(L, 1);

	return out;
}

bool table_get_bool(lua_State* L, int index, const char* key, bool def = false, bool* found = nullptr) {
	bool out = def;

	lua_getfield(L, index, key);
	if (lua_isboolean(L, -1)) {
		out = lua_toboolean(L, -1) != 0;
		if (found != nullptr) {
			*found = true;
		}
	} else if (found != nullptr) {
		*found = false;
	}
	lua_pop(L, 1);

	return out;
}

bool table_has_function(lua_State* L, int index, const char* key) {
	bool result = false;
	lua_getfield(L, index, key);
	if (lua_isfunction(L, -1)) {
		result = true;
	}
	lua_pop(L, 1);
	return result;
}

void collect_table_function_keys(lua_State* L, int index, const char* key, std::vector<std::string>& out) {
	lua_getfield(L, index, key);
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return;
	}

	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		if (lua_isstring(L, -2) && lua_isfunction(L, -1)) {
			out.emplace_back(lua_tostring(L, -2));
		}
		lua_pop(L, 1);
	}

	lua_pop(L, 1);
	std::sort(out.begin(), out.end());
}

std::string resolve_npc_exname(const std::string& raw_name) {
	if (raw_name.empty()) {
		return raw_name;
	}

	std::size_t pos = raw_name.find("::");
	if (pos == std::string::npos || pos + 2 >= raw_name.size()) {
		return raw_name;
	}

	return raw_name.substr(pos + 2);
}

std::string resolve_npc_display_name(const std::string& raw_name) {
	if (raw_name.empty()) {
		return raw_name;
	}

	std::size_t pos = raw_name.find("::");
	if (pos == std::string::npos) {
		return raw_name;
	}
	if (pos == 0) {
		return "";
	}
	return raw_name.substr(0, pos);
}

void collect_lua_label_refs(lua_State* L, int index, const char* key, std::vector<LuaNpcDef::LuaLabelBinding>& out) {
	lua_getfield(L, index, key);
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return;
	}

	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		if (lua_isstring(L, -2) && lua_isfunction(L, -1)) {
			LuaNpcDef::LuaLabelBinding binding;
			binding.name = lua_tostring(L, -2);
			binding.ref = luaL_ref(L, LUA_REGISTRYINDEX);
			out.push_back(std::move(binding));
			continue;
		}
		lua_pop(L, 1);
	}

	lua_pop(L, 1);
}

void release_lua_npc_bindings(lua_State* L, std::vector<LuaNpcDef>& out) {
	for (LuaNpcDef& binding : out) {
		if (binding.main_ref != LUA_NOREF && binding.main_ref != LUA_REFNIL) {
			luaL_unref(L, LUA_REGISTRYINDEX, binding.main_ref);
			binding.main_ref = LUA_NOREF;
		}
		for (LuaNpcDef::LuaLabelBinding& label : binding.events) {
			if (label.ref != LUA_NOREF && label.ref != LUA_REFNIL) {
				luaL_unref(L, LUA_REGISTRYINDEX, label.ref);
				label.ref = LUA_NOREF;
			}
		}
		for (LuaNpcDef::LuaLabelBinding& label : binding.labels) {
			if (label.ref != LUA_NOREF && label.ref != LUA_REFNIL) {
				luaL_unref(L, LUA_REGISTRYINDEX, label.ref);
				label.ref = LUA_NOREF;
			}
		}
	}
}

void collect_lua_script_bindings(lua_State* L, int table_index, std::vector<LuaNpcDef>& out) {
	lua_getfield(L, table_index, "scripts");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return;
	}

	size_t count = lua_rawlen(L, -1);
	for (size_t i = 1; i <= count; ++i) {
		lua_rawgeti(L, -1, static_cast<lua_Integer>(i));
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			continue;
		}

		LuaNpcDef def;
		const std::string raw_name = table_get_string(L, -1, "name");
		def.name = resolve_npc_display_name(raw_name);
		def.exname = resolve_npc_exname(raw_name);
		def.map = table_get_string(L, -1, "map", "-");
		def.x = static_cast<int16>(table_get_int(L, -1, "x"));
		def.y = static_cast<int16>(table_get_int(L, -1, "y"));
		def.dir = static_cast<int16>(table_get_int(L, -1, "dir"));
		def.state = table_get_string(L, -1, "state");
		def.trigger_x = static_cast<int16>(table_get_int(L, -1, "trigger_x", -1));
		def.trigger_y = static_cast<int16>(table_get_int(L, -1, "trigger_y", -1));

		// Support both numeric and string sprite IDs
		lua_getfield(L, -1, "sprite");
		if (lua_isnumber(L, -1)) {
			def.sprite = static_cast<int32>(lua_tointeger(L, -1));
		} else if (lua_isstring(L, -1)) {
			const char* sprite_name = lua_tostring(L, -1);
			int64 constant_val = 0;
			if (script_get_constant(sprite_name, &constant_val)) {
				def.sprite = static_cast<int32>(constant_val);
			} else {
				// Try to find mob with same name
				std::shared_ptr<s_mob_db> mob = mobdb_search_aegisname(sprite_name);
				if (mob != nullptr) {
					def.sprite = static_cast<int32>(mob->id);
				} else {
					ShowWarning("npc_lua: Invalid sprite constant '%s', defaulting to -1.\n", sprite_name);
					def.sprite = -1;
				}
			}
		} else {
			def.sprite = -1;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "main");
		if (lua_isfunction(L, -1)) {
			def.main_ref = luaL_ref(L, LUA_REGISTRYINDEX);
		} else {
			lua_pop(L, 1);
		}

		collect_lua_label_refs(L, -1, "events", def.events);
		collect_lua_label_refs(L, -1, "labels", def.labels);
		int32 pos_seed = 1;
		for (LuaNpcDef::LuaLabelBinding& label : def.events) {
			label.pos = pos_seed++;
		}
		for (LuaNpcDef::LuaLabelBinding& label : def.labels) {
			label.pos = pos_seed++;
		}

		const bool has_functions =
		    (def.main_ref != LUA_NOREF && def.main_ref != LUA_REFNIL) ||
		    !def.events.empty() || !def.labels.empty();
		if (has_functions && !def.exname.empty()) {
			out.push_back(std::move(def));
		} else {
			std::vector<LuaNpcDef> tmp;
			tmp.push_back(std::move(def));
			release_lua_npc_bindings(L, tmp);
		}

		lua_pop(L, 1);
	}

	lua_pop(L, 1);
}

void release_lua_function_bindings(lua_State* L, std::vector<LuaFunctionDef>& out) {
	for (LuaFunctionDef& binding : out) {
		if (binding.run_ref != LUA_NOREF && binding.run_ref != LUA_REFNIL) {
			luaL_unref(L, LUA_REGISTRYINDEX, binding.run_ref);
			binding.run_ref = LUA_NOREF;
		}
		for (LuaNpcDef::LuaLabelBinding& label : binding.labels) {
			if (label.ref != LUA_NOREF && label.ref != LUA_REFNIL) {
				luaL_unref(L, LUA_REGISTRYINDEX, label.ref);
				label.ref = LUA_NOREF;
			}
		}
	}
}

void collect_lua_function_bindings(lua_State* L, int table_index, std::vector<LuaFunctionDef>& out) {
	lua_getfield(L, table_index, "functions");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return;
	}

	size_t count = lua_rawlen(L, -1);
	for (size_t i = 1; i <= count; ++i) {
		lua_rawgeti(L, -1, static_cast<lua_Integer>(i));
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			continue;
		}

		LuaFunctionDef def;
		def.name = table_get_string(L, -1, "name");

		lua_getfield(L, -1, "run");
		if (lua_isfunction(L, -1)) {
			def.run_ref = luaL_ref(L, LUA_REGISTRYINDEX);
		} else {
			lua_pop(L, 1);
		}

		collect_lua_label_refs(L, -1, "labels", def.labels);
		const bool has_lua_body = (def.run_ref != LUA_NOREF && def.run_ref != LUA_REFNIL) || !def.labels.empty();
		if (has_lua_body && !def.name.empty()) {
			out.push_back(std::move(def));
		} else {
			std::vector<LuaFunctionDef> tmp;
			tmp.push_back(std::move(def));
			release_lua_function_bindings(L, tmp);
		}

		lua_pop(L, 1);
	}

	lua_pop(L, 1);
}

void append_raw_lines(lua_State* L, int table_index, const char* key, std::string& out) {
	lua_getfield(L, table_index, key);
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return;
	}

	size_t count = lua_rawlen(L, -1);
	for (size_t i = 1; i <= count; ++i) {
		lua_rawgeti(L, -1, static_cast<lua_Integer>(i));
		if (lua_isstring(L, -1)) {
			append_with_newline(out, lua_tostring(L, -1));
		}
		lua_pop(L, 1);
	}

	lua_pop(L, 1);
}

void append_typed_warps(lua_State* L, int table_index, std::string& out) {
	lua_getfield(L, table_index, "warps");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return;
	}

	size_t count = lua_rawlen(L, -1);
	for (size_t i = 1; i <= count; ++i) {
		lua_rawgeti(L, -1, static_cast<lua_Integer>(i));
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			continue;
		}

		const std::string raw = table_get_string(L, -1, "raw");
		if (!raw.empty()) {
			append_with_newline(out, raw);
			lua_pop(L, 1);
			continue;
		}

		const std::string map = table_get_string(L, -1, "map");
		const std::string name = table_get_string(L, -1, "name");
		const std::string to_map = table_get_string(L, -1, "to_map");
		if (map.empty() || name.empty() || to_map.empty()) {
			ShowError("npc_parseluafile: 'warps[%zu]' missing required fields (map/name/to_map).\n", i);
			lua_pop(L, 1);
			continue;
		}

		const int32 x = table_get_int(L, -1, "x");
		const int32 y = table_get_int(L, -1, "y");
		const int32 dir = table_get_int(L, -1, "dir");
		const int32 xs = table_get_int(L, -1, "xs");
		const int32 ys = table_get_int(L, -1, "ys");
		const int32 to_x = table_get_int(L, -1, "to_x");
		const int32 to_y = table_get_int(L, -1, "to_y");
		std::string type = table_get_string(L, -1, "type", "warp");
		const std::string state = table_get_string(L, -1, "state");
		if (!state.empty()) {
			type += "(" + state + ")";
		}

		char buffer[NPC_LUA_FIELD_BUFFER];
		safesnprintf(buffer, sizeof(buffer), "%s,%d,%d,%d\t%s\t%s\t%d,%d,%s,%d,%d\n",
		             map.c_str(), x, y, dir, type.c_str(), name.c_str(), xs, ys, to_map.c_str(), to_x, to_y);
		out.append(buffer);

		lua_pop(L, 1);
	}

	lua_pop(L, 1);
}

void append_typed_monsters(lua_State* L, int table_index, std::string& out) {
	lua_getfield(L, table_index, "monsters");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return;
	}

	size_t count = lua_rawlen(L, -1);
	for (size_t i = 1; i <= count; ++i) {
		lua_rawgeti(L, -1, static_cast<lua_Integer>(i));
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			continue;
		}

		const std::string raw = table_get_string(L, -1, "raw");
		if (!raw.empty()) {
			append_with_newline(out, raw);
			lua_pop(L, 1);
			continue;
		}

		const std::string map = table_get_string(L, -1, "map");
		const std::string name = table_get_string(L, -1, "name");
		if (map.empty() || name.empty()) {
			ShowError("npc_parseluafile: 'monsters[%zu]' missing required fields (map/name).\n", i);
			lua_pop(L, 1);
			continue;
		}

		bool has_mob_id = false;
		bool has_amount = false;
		const int32 mob_id = table_get_int(L, -1, "mob_id", 0, &has_mob_id);
		const int32 amount = table_get_int(L, -1, "amount", 0, &has_amount);
		if (!has_mob_id || !has_amount) {
			ShowError("npc_parseluafile: 'monsters[%zu]' missing required fields (mob_id/amount).\n", i);
			lua_pop(L, 1);
			continue;
		}

		const int32 x = table_get_int(L, -1, "x");
		const int32 y = table_get_int(L, -1, "y");
		const int32 xs = table_get_int(L, -1, "xs");
		const int32 ys = table_get_int(L, -1, "ys");
		const int32 delay1 = table_get_int(L, -1, "delay1");
		const int32 delay2 = table_get_int(L, -1, "delay2");
		const std::string event_name = table_get_string(L, -1, "event_name");
		const int32 size = table_get_int(L, -1, "size", -1);
		const int32 ai = table_get_int(L, -1, "ai", -1);
		const bool boss = table_get_bool(L, -1, "boss", false);

		std::ostringstream line;
		line << map << "," << x << "," << y << "," << xs << "," << ys << "\t";
		line << (boss ? "boss_monster" : "monster") << "\t";
		line << name << "\t";
		line << mob_id << "," << amount;

		const bool has_optional = (delay1 != 0 || delay2 != 0 || !event_name.empty() || size != -1 || ai != -1);
		if (has_optional) {
			line << "," << delay1 << "," << delay2 << "," << (event_name.empty() ? " " : event_name) << "," << size << "," << ai;
		}

		append_with_newline(out, line.str());
		lua_pop(L, 1);
	}

	lua_pop(L, 1);
}

void append_typed_mapflags(lua_State* L, int table_index, std::string& out) {
	lua_getfield(L, table_index, "mapflags");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return;
	}

	size_t count = lua_rawlen(L, -1);
	for (size_t i = 1; i <= count; ++i) {
		lua_rawgeti(L, -1, static_cast<lua_Integer>(i));
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			continue;
		}

		const std::string raw = table_get_string(L, -1, "raw");
		if (!raw.empty()) {
			append_with_newline(out, raw);
			lua_pop(L, 1);
			continue;
		}

		const std::string map = table_get_string(L, -1, "map");
		const std::string flag = table_get_string(L, -1, "flag");
		if (map.empty() || flag.empty()) {
			ShowError("npc_parseluafile: 'mapflags[%zu]' missing required fields (map/flag).\n", i);
			lua_pop(L, 1);
			continue;
		}

		const bool off = table_get_bool(L, -1, "off", false);
		const std::string value = table_get_string(L, -1, "value");

		std::ostringstream line;
		line << map << "\tmapflag\t" << flag;
		if (off) {
			line << "\toff";
		} else if (!value.empty()) {
			line << "\t" << value;
		}
		append_with_newline(out, line.str());

		lua_pop(L, 1);
	}

	lua_pop(L, 1);
}

void append_typed_shops(lua_State* L, int table_index, std::string& out) {
	lua_getfield(L, table_index, "shops");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return;
	}

	size_t count = lua_rawlen(L, -1);
	for (size_t i = 1; i <= count; ++i) {
		lua_rawgeti(L, -1, static_cast<lua_Integer>(i));
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			continue;
		}

		const std::string raw = table_get_string(L, -1, "raw");
		if (!raw.empty()) {
			append_with_newline(out, raw);
			lua_pop(L, 1);
			continue;
		}

		const std::string type = table_get_string(L, -1, "type", "shop");
		const std::string name = table_get_string(L, -1, "name");
		const std::string items = table_get_string(L, -1, "items");

		// Get sprite - support both numeric and string constants (like scripts)
		std::string sprite_str;
		lua_getfield(L, -1, "sprite");
		if (lua_isnumber(L, -1)) {
			char buffer[64];
			safesnprintf(buffer, sizeof(buffer), "%d", static_cast<int32>(lua_tointeger(L, -1)));
			sprite_str = buffer;
		} else if (lua_isstring(L, -1)) {
			sprite_str = lua_tostring(L, -1);
		} else {
			sprite_str = "-1";  // Default to invisible if not specified
		}
		lua_pop(L, 1);

		if (name.empty() || items.empty()) {
			ShowError("npc_parseluafile: 'shops[%zu]' missing required fields (name/items).\n", i);
			lua_pop(L, 1);
			continue;
		}

		std::string placement;
		const std::string map = table_get_string(L, -1, "map", "-");
		if (map == "-") {
			placement = "-";
		} else {
			const int32 x = table_get_int(L, -1, "x");
			const int32 y = table_get_int(L, -1, "y");
			const int32 dir = table_get_int(L, -1, "dir");
			char buffer[NPC_LUA_FIELD_BUFFER];
			safesnprintf(buffer, sizeof(buffer), "%s,%d,%d,%d", map.c_str(), x, y, dir);
			placement = buffer;
		}

		std::ostringstream line;
		line << placement << "\t" << type << "\t" << name << "\t" << sprite_str << "," << items;
		ShowDebug("npc_lua: shop line: %s\n", line.str().c_str());
		append_with_newline(out, line.str());
		lua_pop(L, 1);
	}

	lua_pop(L, 1);
}

void append_typed_duplicates(lua_State* L, int table_index, std::string& out, const char* filepath) {
	lua_getfield(L, table_index, "duplicates");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return;
	}

	size_t count = lua_rawlen(L, -1);
	for (size_t i = 1; i <= count; ++i) {
		lua_rawgeti(L, -1, static_cast<lua_Integer>(i));
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			continue;
		}

		const std::string raw = table_get_string(L, -1, "raw");
		if (!raw.empty()) {
			append_with_newline(out, raw);
			lua_pop(L, 1);
			continue;
		}

		const std::string source = table_get_string(L, -1, "source");
		const std::string name = table_get_string(L, -1, "name");
		const std::string sprite = table_get_string(L, -1, "view");
		if (source.empty() || name.empty() || sprite.empty()) {
			ShowError("npc_parseluafile: '%s' duplicates[%zu] missing required fields (source/name/view).\n",
			          filepath ? filepath : "<unknown>", i);
			lua_pop(L, 1);
			continue;
		}

		std::string placement;
		const std::string map = table_get_string(L, -1, "map", "-");
		if (map == "-") {
			placement = "-";
		} else {
			const int32 x = table_get_int(L, -1, "x");
			const int32 y = table_get_int(L, -1, "y");
			const int32 dir = table_get_int(L, -1, "dir");
			char buffer[NPC_LUA_FIELD_BUFFER];
			safesnprintf(buffer, sizeof(buffer), "%s,%d,%d,%d", map.c_str(), x, y, dir);
			placement = buffer;
		}

		std::ostringstream line;
		line << placement << "\tduplicate(" << source << ")\t" << name << "\t" << sprite;
		ShowDebug("npc_lua: duplicate line: %s\n", line.str().c_str());
		append_with_newline(out, line.str());
		lua_pop(L, 1);
	}

	lua_pop(L, 1);
}

void append_typed_scripts(lua_State* L, int table_index, std::string& out, const char* filepath) {
	lua_getfield(L, table_index, "scripts");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return;
	}

	size_t count = lua_rawlen(L, -1);
	for (size_t i = 1; i <= count; ++i) {
		lua_rawgeti(L, -1, static_cast<lua_Integer>(i));
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			continue;
		}

		const std::string raw = table_get_string(L, -1, "raw");
		if (!raw.empty()) {
			append_with_newline(out, raw);
			lua_pop(L, 1);
			continue;
		}

		const std::string body = table_get_string(L, -1, "body");
		const bool has_main = table_has_function(L, -1, "main");
		std::vector<std::string> event_labels;
		std::vector<std::string> local_labels;
		collect_table_function_keys(L, -1, "events", event_labels);
		collect_table_function_keys(L, -1, "labels", local_labels);
		const bool has_lua_body = has_main || !event_labels.empty() || !local_labels.empty();
		const std::string name = table_get_string(L, -1, "name");
		if (name.empty() || (body.empty() && !has_lua_body)) {
			ShowError("npc_parseluafile: '%s' scripts[%zu] missing required fields (name/body|main/events/labels).\n",
			          filepath ? filepath : "<unknown>", i);
			lua_pop(L, 1);
			continue;
		}
		if (body.empty() && has_lua_body) {
			lua_pop(L, 1);
			continue;
		}

		std::string placement;
		const std::string map = table_get_string(L, -1, "map", "-");
		if (map == "-") {
			placement = "-";
		} else {
			const int32 x = table_get_int(L, -1, "x");
			const int32 y = table_get_int(L, -1, "y");
			const int32 dir = table_get_int(L, -1, "dir");
			char buffer[NPC_LUA_FIELD_BUFFER];
			safesnprintf(buffer, sizeof(buffer), "%s,%d,%d,%d", map.c_str(), x, y, dir);
			placement = buffer;
		}

		std::string type = table_get_string(L, -1, "type", "script");
		const std::string state = table_get_string(L, -1, "state");
		if (!state.empty()) {
			type += "(" + state + ")";
		}

		bool has_trigger_x = false;
		bool has_trigger_y = false;
		const int32 sprite = table_get_int(L, -1, "sprite", -1);
		const int32 trigger_x = table_get_int(L, -1, "trigger_x", 0, &has_trigger_x);
		const int32 trigger_y = table_get_int(L, -1, "trigger_y", 0, &has_trigger_y);

		std::ostringstream header;
		header << placement << "\t" << type << "\t" << name << "\t" << sprite;
		if (has_trigger_x && has_trigger_y) {
			header << "," << trigger_x << "," << trigger_y;
		}
		header << ",{";

		append_with_newline(out, header.str());
		if (!body.empty()) {
			append_with_newline(out, body);
		}
		append_with_newline(out, "}");
		lua_pop(L, 1);
	}

	lua_pop(L, 1);
}

void append_typed_functions(lua_State* L, int table_index, std::string& out, const char* filepath) {
	lua_getfield(L, table_index, "functions");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return;
	}

	size_t count = lua_rawlen(L, -1);
	for (size_t i = 1; i <= count; ++i) {
		lua_rawgeti(L, -1, static_cast<lua_Integer>(i));
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			continue;
		}

		const std::string raw = table_get_string(L, -1, "raw");
		if (!raw.empty()) {
			append_with_newline(out, raw);
			lua_pop(L, 1);
			continue;
		}

		const std::string body = table_get_string(L, -1, "body");
		const bool has_run = table_has_function(L, -1, "run");
		std::vector<std::string> local_labels;
		collect_table_function_keys(L, -1, "labels", local_labels);
		const bool has_lua_body = has_run || !local_labels.empty();

		const std::string name = table_get_string(L, -1, "name");
		const bool is_empty_placeholder = name.empty() && body.empty() && !has_lua_body;
		if (is_empty_placeholder) {
			lua_pop(L, 1);
			continue;
		}
		if (name.empty() || (body.empty() && !has_lua_body)) {
			ShowError("npc_parseluafile: '%s' functions[%zu] missing required fields (name/body|run/labels).\n",
			          filepath ? filepath : "<unknown>", i);
			lua_pop(L, 1);
			continue;
		}
		if (body.empty() && has_lua_body) {
			lua_pop(L, 1);
			continue;
		}

		append_with_newline(out, "function\tscript\t" + name + "\t{");
		if (!body.empty()) {
			append_with_newline(out, body);
		}
		append_with_newline(out, "}");
		lua_pop(L, 1);
	}

	lua_pop(L, 1);
}

bool write_temp_script_file(const std::string& dsl, std::string& out_path) {
#if defined(_WIN32)
	char path[L_tmpnam_s];
	if (tmpnam_s(path, sizeof(path)) != 0) {
		ShowError("npc_parseluafile: failed to allocate temporary path.\n");
		return false;
	}

	FILE* fp = fopen(path, "wb");
	if (fp == nullptr) {
		ShowError("npc_parseluafile: unable to create temporary file '%s' - %s.\n", path, strerror(errno));
		return false;
	}
#else
	std::string pattern = "/tmp/rathena_npc_lua_XXXXXX";
	std::vector<char> path(pattern.begin(), pattern.end());
	path.push_back('\0');
	int fd = mkstemp(path.data());
	if (fd < 0) {
		ShowError("npc_parseluafile: unable to create temporary file from pattern '%s' - %s.\n", pattern.c_str(), strerror(errno));
		return false;
	}

	FILE* fp = fdopen(fd, "wb");
	if (fp == nullptr) {
		ShowError("npc_parseluafile: unable to open temporary file descriptor for '%s' - %s.\n", path.data(), strerror(errno));
		close(fd);
		return false;
	}
#endif

	size_t written = fwrite(dsl.data(), 1, dsl.size(), fp);
	fclose(fp);

	if (written != dsl.size()) {
#if defined(_WIN32)
		ShowError("npc_parseluafile: failed to write all data to temporary file '%s'.\n", path);
		remove(path);
#else
		ShowError("npc_parseluafile: failed to write all data to temporary file '%s'.\n", path.data());
		remove(path.data());
#endif
		return false;
	}

#if defined(_WIN32)
	out_path.assign(path);
#else
	out_path.assign(path.data());
#endif
	return true;
}

} // namespace
#endif

int32 npc_parseluafile(const char* filepath) {
#if defined(HAVE_LUA) || defined(WITH_LUA)
	if (filepath == nullptr || filepath[0] == '\0') {
		ShowError("npc_parseluafile: invalid filepath.\n");
		return 0;
	}

	if (!lua_engine_load_file(filepath)) {
		return 0;
	}

	lua_State* L = lua_engine_get_state();
	if (L == nullptr || !lua_engine_push_loaded_table()) {
		ShowError("npc_parseluafile: failed to access loaded Lua table for '%s'.\n", filepath);
		return 0;
	}

	std::vector<LuaNpcDef> lua_npcs;
	std::vector<LuaFunctionDef> lua_functions;
	collect_lua_script_bindings(L, -1, lua_npcs);
	collect_lua_function_bindings(L, -1, lua_functions);

	for (LuaNpcDef& def : lua_npcs) {
		def.npc_id = npc_lua_register_script(def, filepath);
		if (def.npc_id <= 0) {
			ShowWarning("npc_parseluafile: unable to create Lua NPC '%s' from '%s'.\n",
			            def.exname.c_str(), filepath);
			continue;
		}
		if (!lua_engine_register_npc(def)) {
			ShowWarning("npc_parseluafile: failed to register Lua runtime for npc '%s' in '%s'.\n",
			            def.exname.c_str(), filepath);
		}
	}

	for (const LuaFunctionDef& def : lua_functions) {
		if (!lua_engine_register_function(def)) {
			ShowWarning("npc_parseluafile: failed to register Lua function '%s' in '%s'.\n",
			            def.name.c_str(), filepath);
		}
	}

	std::string dsl;
	append_with_newline(dsl, table_get_string(L, -1, "dsl"));
	append_raw_lines(L, -1, "raw_lines", dsl);
	append_typed_warps(L, -1, dsl);
	append_typed_monsters(L, -1, dsl);
	append_typed_mapflags(L, -1, dsl);
	append_typed_shops(L, -1, dsl);
	append_typed_scripts(L, -1, dsl, filepath);
	append_typed_duplicates(L, -1, dsl, filepath);
	append_typed_functions(L, -1, dsl, filepath);

	lua_pop(L, 1);

	int32 result = 1;
	if (!dsl.empty()) {
		std::string tmp_path;
		if (!write_temp_script_file(dsl, tmp_path)) {
			result = 0;
		} else {
			result = npc_parsesrcfile(tmp_path.c_str());
			remove(tmp_path.c_str());
		}
	} else if (lua_npcs.empty() && lua_functions.empty()) {
		ShowWarning("npc_parseluafile: '%s' produced no script entries.\n", filepath);
	}

	if (result != 0) {
		s_mapiterator* iter = mapit_geteachnpc();
		for (block_list* bl = mapit_next(iter); bl != nullptr; bl = mapit_next(iter)) {
			npc_data* nd = BL_CAST(BL_NPC, bl);
			if (nd == nullptr || nd->subtype != NPCTYPE_SCRIPT || nd->src_id <= 0 || nd->path == nullptr) {
				continue;
			}
			if (strcasecmp(nd->path, filepath) != 0) {
				continue;
			}
			lua_engine_clone_npc(nd->src_id, nd->id, nd->exname);
		}
		mapit_free(iter);
	}

	release_lua_npc_bindings(L, lua_npcs);
	release_lua_function_bindings(L, lua_functions);
	return result;
#else
	ShowError("npc_parseluafile: Lua support is disabled at build time for '%s'.\n", filepath ? filepath : "<null>");
	return 0;
#endif
}
