// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
//
// STATUS NOTE (2026-02):
// `script2lua` is currently in limited-maintenance mode and is intentionally
// not the primary migration path. Prefer manual "golden" Lua rewrites for
// production gameplay scripts under `lua/gold/`.
// Use this tool only for exploratory/assisted conversion output, then review
// and rewrite manually before enabling in live script configs.

#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace {

enum class EntryKind {
	Warp,
	Monster,
	Mapflag,
	Shop,
	Duplicate,
	Script,
	Function,
};

struct UnsupportedItem {
	size_t line = 0;
	std::string reason;
};

struct LuaEntry {
	EntryKind kind{};
	size_t line = 0;
	size_t body_line = 0;
	std::string w1;
	std::string w2;
	std::string w3;
	std::string w4;
	std::string body;
};

struct LuaCodeBlock {
	std::string name;
	size_t line = 0;
	std::vector<std::pair<size_t, std::string>> source_lines;
	std::vector<std::string> lua_lines;
};

struct BodyConversion {
	std::vector<LuaCodeBlock> events;
	std::vector<LuaCodeBlock> labels;
	LuaCodeBlock main;
	std::vector<UnsupportedItem> unsupported;
};

struct RenderResult {
	std::string output;
	std::vector<UnsupportedItem> unsupported;
};

struct FileResult {
	fs::path source;
	fs::path target;
	std::string status;
	size_t line_count = 0;
	std::vector<UnsupportedItem> unsupported;
	std::string message;
};

struct ParseResult {
	std::vector<LuaEntry> entries;
	std::vector<UnsupportedItem> unsupported;
};

struct Options {
	fs::path input;
	fs::path output;
	fs::path report;
	bool recursive = false;
	bool overwrite = false;
	bool strict = false;
	bool quiet = false;
};

void print_usage() {
	std::cout
		<< "Usage: script2lua <input> <output> [options]\n"
		<< "Status: exploratory converter only (manual rewrite required for production)\n"
		<< "\n"
		<< "Options:\n"
		<< "  --report <path>   Write migration report to this file\n"
		<< "  --recursive       Recursively scan input directory for .txt files\n"
		<< "  --overwrite       Overwrite existing output files\n"
		<< "  --strict          Exit with failure when unsupported syntax is found\n"
		<< "  --quiet           Reduce stdout output\n";
}

bool starts_with_dash(const std::string& value) {
	return !value.empty() && value[0] == '-';
}

bool parse_options(int argc, char* argv[], Options& options, std::string& error) {
	if (argc < 3) {
		error = "missing required arguments";
		return false;
	}

	options.input = argv[1];
	options.output = argv[2];

	for (int i = 3; i < argc; ++i) {
		const std::string arg = argv[i];
		if (arg == "--recursive") {
			options.recursive = true;
		} else if (arg == "--overwrite") {
			options.overwrite = true;
		} else if (arg == "--strict") {
			options.strict = true;
		} else if (arg == "--quiet") {
			options.quiet = true;
		} else if (arg == "--report") {
			if (i + 1 >= argc || starts_with_dash(argv[i + 1])) {
				error = "--report requires a file path";
				return false;
			}
			options.report = argv[++i];
		} else {
			error = "unknown argument: " + arg;
			return false;
		}
	}

	return true;
}

std::string read_file(const fs::path& path) {
	std::ifstream input(path, std::ios::binary);
	if (!input.is_open()) {
		return "";
	}
	std::ostringstream ss;
	ss << input.rdbuf();
	return ss.str();
}

bool write_file(const fs::path& path, const std::string& data) {
	if (path.has_parent_path()) {
		std::error_code ec;
		fs::create_directories(path.parent_path(), ec);
		if (ec) {
			return false;
		}
	}

	std::ofstream output(path, std::ios::binary | std::ios::trunc);
	if (!output.is_open()) {
		return false;
	}
	output.write(data.data(), static_cast<std::streamsize>(data.size()));
	return output.good();
}

size_t count_lines(const std::string& input) {
	if (input.empty()) {
		return 0;
	}
	size_t lines = 1;
	for (char c : input) {
		if (c == '\n') {
			++lines;
		}
	}
	return lines;
}

std::string to_lower(std::string text) {
	std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});
	return text;
}

std::string trim_copy(const std::string& value) {
	size_t start = 0;
	while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
		++start;
	}
	size_t end = value.size();
	while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
		--end;
	}
	return value.substr(start, end - start);
}

std::vector<std::string> split_lines_preserve(const std::string& text) {
	std::vector<std::string> lines;
	std::string current;
	for (char c : text) {
		if (c == '\n') {
			lines.push_back(current);
			current.clear();
		} else {
			current.push_back(c);
		}
	}
	if (!current.empty() || (!text.empty() && text.back() == '\n')) {
		lines.push_back(current);
	}
	return lines;
}

std::vector<std::string> split_tabs(const std::string& line) {
	std::vector<std::string> parts;
	size_t start = 0;
	for (;;) {
		size_t pos = line.find('\t', start);
		if (pos == std::string::npos) {
			parts.push_back(line.substr(start));
			break;
		}
		parts.push_back(line.substr(start, pos - start));
		start = pos + 1;
	}
	return parts;
}

std::string join_from(const std::vector<std::string>& parts, size_t index) {
	if (index >= parts.size()) {
		return "";
	}
	std::string out = parts[index];
	for (size_t i = index + 1; i < parts.size(); ++i) {
		out += "\t";
		out += parts[i];
	}
	return out;
}

bool starts_with(const std::string& value, const std::string& prefix) {
	return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

bool ends_with(const std::string& value, const std::string& suffix) {
	return value.size() >= suffix.size()
		&& value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool is_script_token(const std::string& token) {
	const std::string lower = to_lower(trim_copy(token));
	return lower == "script" || starts_with(lower, "script(");
}

bool is_function_header(const std::string& w1, const std::string& w2) {
	return to_lower(trim_copy(w1)) == "function" && to_lower(trim_copy(w2)) == "script";
}

bool is_warp_token(const std::string& token) {
	const std::string lower = to_lower(trim_copy(token));
	return starts_with(lower, "warp");
}

bool is_monster_token(const std::string& token) {
	const std::string lower = to_lower(trim_copy(token));
	return lower == "monster" || lower == "boss_monster";
}

bool is_mapflag_token(const std::string& token) {
	return to_lower(trim_copy(token)) == "mapflag";
}

bool is_shop_token(const std::string& token) {
	const std::string lower = to_lower(trim_copy(token));
	return lower == "shop" || lower == "cashshop" || lower == "itemshop" || lower == "pointshop" || lower == "marketshop";
}

bool is_duplicate_token(const std::string& token) {
	const std::string lower = to_lower(trim_copy(token));
	return starts_with(lower, "duplicate(");
}

bool extract_script_block(const std::vector<std::string>& lines, size_t start_line, size_t open_pos, std::string& body, size_t& end_line, std::string& error) {
	bool in_double_quote = false;
	bool in_line_comment = false;
	bool in_block_comment = false;
	bool escaped = false;
	int32_t depth = 1;

	body.clear();
	for (size_t line_idx = start_line; line_idx < lines.size(); ++line_idx) {
		const std::string& line = lines[line_idx];
		size_t begin = (line_idx == start_line) ? open_pos + 1 : 0;

		for (size_t i = begin; i < line.size(); ++i) {
			const char c = line[i];
			const char next = (i + 1 < line.size()) ? line[i + 1] : '\0';

			if (in_line_comment) {
				body.push_back(c);
				continue;
			}
			if (in_block_comment) {
				body.push_back(c);
				if (c == '*' && next == '/') {
					body.push_back(next);
					++i;
					in_block_comment = false;
				}
				continue;
			}
			if (in_double_quote) {
				body.push_back(c);
				if (escaped) {
					escaped = false;
				} else if (c == '\\') {
					escaped = true;
				} else if (c == '"') {
					in_double_quote = false;
				}
				continue;
			}

			if (c == '/' && next == '/') {
				body.push_back(c);
				body.push_back(next);
				++i;
				in_line_comment = true;
				continue;
			}
			if (c == '/' && next == '*') {
				body.push_back(c);
				body.push_back(next);
				++i;
				in_block_comment = true;
				continue;
			}
			if (c == '"') {
				body.push_back(c);
				in_double_quote = true;
				escaped = false;
				continue;
			}
			if (c == '{') {
				++depth;
				body.push_back(c);
				continue;
			}
			if (c == '}') {
				--depth;
				if (depth == 0) {
					end_line = line_idx;
					return true;
				}
				body.push_back(c);
				continue;
			}
			body.push_back(c);
		}

		if (line_idx + 1 < lines.size()) {
			body.push_back('\n');
		}
		in_line_comment = false;
	}

	error = "unterminated script/function body";
	return false;
}

ParseResult parse_script_text(const std::string& input_text) {
	ParseResult result;
	const std::vector<std::string> lines = split_lines_preserve(input_text);
	bool in_block_comment = false;

	for (size_t i = 0; i < lines.size(); ++i) {
		std::string line = lines[i];
		if (!line.empty() && line.back() == '\r') {
			line.pop_back();
		}

		const std::string trimmed = trim_copy(line);
		if (in_block_comment) {
			if (trimmed.find("*/") != std::string::npos) {
				in_block_comment = false;
			}
			continue;
		}

		if (trimmed.empty()) {
			continue;
		}
		if (starts_with(trimmed, "//")) {
			continue;
		}
		if (starts_with(trimmed, "/*")) {
			if (trimmed.find("*/") == std::string::npos) {
				in_block_comment = true;
			}
			continue;
		}

		std::vector<std::string> fields = split_tabs(line);
		if (fields.size() < 2) {
			result.unsupported.push_back({i + 1, "line has fewer than 2 tab-separated fields"});
			continue;
		}

		const std::string w1 = trim_copy(fields[0]);
		const std::string w2 = trim_copy(fields[1]);
		const bool block_entry = (is_script_token(w2) || is_function_header(w1, w2));

		if (block_entry) {
			size_t open_pos = line.find('{');
			if (open_pos == std::string::npos) {
				result.unsupported.push_back({i + 1, "script/function line is missing '{'"});
				continue;
			}

			const std::string header = line.substr(0, open_pos);
			std::vector<std::string> header_fields = split_tabs(header);
			while (header_fields.size() < 4) {
				header_fields.push_back("");
			}

			LuaEntry entry;
			entry.kind = is_function_header(trim_copy(header_fields[0]), trim_copy(header_fields[1])) ? EntryKind::Function : EntryKind::Script;
			entry.line = i + 1;
			entry.body_line = i + 1;
			entry.w1 = trim_copy(header_fields[0]);
			entry.w2 = trim_copy(header_fields[1]);
			entry.w3 = trim_copy(header_fields[2]);
			entry.w4 = trim_copy(join_from(header_fields, 3));

			std::string error;
			size_t end_line = i;
			if (!extract_script_block(lines, i, open_pos, entry.body, end_line, error)) {
				result.unsupported.push_back({i + 1, error});
				continue;
			}

			if (entry.kind == EntryKind::Function && entry.w3.empty()) {
				result.unsupported.push_back({i + 1, "function definition missing name"});
				i = end_line;
				continue;
			}
			if (entry.kind == EntryKind::Script && (entry.w1.empty() || entry.w2.empty() || entry.w3.empty())) {
				result.unsupported.push_back({i + 1, "script definition missing required header fields"});
				i = end_line;
				continue;
			}

			result.entries.push_back(std::move(entry));
			i = end_line;
			continue;
		}

		if (fields.size() < 3) {
			result.unsupported.push_back({i + 1, "line has fewer than 3 tab-separated fields"});
			continue;
		}

		LuaEntry entry;
		entry.line = i + 1;
		entry.w1 = trim_copy(fields[0]);
		entry.w2 = trim_copy(fields[1]);
		entry.w3 = trim_copy(fields[2]);
		entry.w4 = trim_copy(join_from(fields, 3));

		if (is_warp_token(entry.w2)) {
			if (entry.w4.empty()) {
				result.unsupported.push_back({i + 1, "warp line missing 4th field"});
				continue;
			}
			entry.kind = EntryKind::Warp;
		} else if (is_monster_token(entry.w2)) {
			if (entry.w4.empty()) {
				result.unsupported.push_back({i + 1, "monster line missing 4th field"});
				continue;
			}
			entry.kind = EntryKind::Monster;
		} else if (is_mapflag_token(entry.w2)) {
			entry.kind = EntryKind::Mapflag;
		} else if (is_shop_token(entry.w2)) {
			if (entry.w4.empty()) {
				result.unsupported.push_back({i + 1, "shop line missing 4th field"});
				continue;
			}
			entry.kind = EntryKind::Shop;
		} else if (is_duplicate_token(entry.w2)) {
			if (entry.w4.empty()) {
				result.unsupported.push_back({i + 1, "duplicate line missing 4th field"});
				continue;
			}
			entry.kind = EntryKind::Duplicate;
		} else {
			result.unsupported.push_back({i + 1, "unsupported top-level token: " + entry.w2});
			continue;
		}

		result.entries.push_back(std::move(entry));
	}

	return result;
}

std::string to_portable_path(const fs::path& path) {
	return path.generic_string();
}

std::string lua_quote_string(const std::string& input) {
	std::ostringstream out;
	for (unsigned char c : input) {
		switch (c) {
			case '\\': out << "\\\\"; break;
			case '"': out << "\\\""; break;
			case '\n': out << "\\n"; break;
			case '\r': out << "\\r"; break;
			case '\t': out << "\\t"; break;
			default:
				if (c < 0x20 || c == 0x7f) {
					const char* hex = "0123456789abcdef";
					out << "\\x" << hex[(c >> 4) & 0x0f] << hex[c & 0x0f];
				} else {
					out << static_cast<char>(c);
				}
				break;
		}
	}
	return out.str();
}

void emit_lua_string(std::ostringstream& out, const std::string& value) {
	out << '"' << lua_quote_string(value) << '"';
}

bool starts_with_word(const std::string& text, const std::string& word) {
	if (!starts_with(text, word)) {
		return false;
	}
	if (text.size() == word.size()) {
		return true;
	}
	const unsigned char next = static_cast<unsigned char>(text[word.size()]);
	return std::isspace(next) != 0 || next == '(' || next == ':' || next == '{';
}

bool is_identifier_start(unsigned char c) {
	return std::isalpha(c) != 0 || c == '_';
}

bool is_identifier_char(unsigned char c) {
	return std::isalnum(c) != 0 || c == '_';
}

bool is_prefixed_variable_start(const std::string& input, size_t index) {
	if (index >= input.size()) {
		return false;
	}

	const unsigned char c = static_cast<unsigned char>(input[index]);
	const unsigned char next = (index + 1 < input.size()) ? static_cast<unsigned char>(input[index + 1]) : 0;
	if (c == '.') {
		return next == '@' || next == '$' || next == '#' || next == '\'' || is_identifier_start(next);
	}
	if (c == '$' || c == '@' || c == '#' || c == '\'') {
		return std::isalnum(next) != 0 || next == '_' || next == '$' || next == '@' || next == '#';
	}
	return false;
}

bool is_variable_token_char(unsigned char c) {
	return std::isalnum(c) != 0 || c == '_' || c == '$' || c == '@' || c == '#' || c == '.' || c == '\'';
}

std::string lua_quote_literal(const std::string& input) {
	return "\"" + lua_quote_string(input) + "\"";
}

std::vector<std::string> split_args_top_level(const std::string& input) {
	std::vector<std::string> args;
	std::string current;
	int depth = 0;
	bool in_double_quote = false;
	bool escaped = false;

	for (size_t i = 0; i < input.size(); ++i) {
		const char c = input[i];
		if (in_double_quote) {
			current.push_back(c);
			if (escaped) {
				escaped = false;
			} else if (c == '\\') {
				escaped = true;
			} else if (c == '"') {
				in_double_quote = false;
			}
			continue;
		}

		if (c == '"') {
			current.push_back(c);
			in_double_quote = true;
			escaped = false;
			continue;
		}

		if (c == '(' || c == '[' || c == '{') {
			++depth;
			current.push_back(c);
			continue;
		}
		if ((c == ')' || c == ']' || c == '}') && depth > 0) {
			--depth;
			current.push_back(c);
			continue;
		}

		if (c == ',' && depth == 0) {
			args.push_back(trim_copy(current));
			current.clear();
			continue;
		}

		current.push_back(c);
	}

	if (!current.empty() || !args.empty()) {
		args.push_back(trim_copy(current));
	}
	return args;
}

void split_line_comment(const std::string& input, std::string& code, std::string& comment) {
	bool in_double_quote = false;
	bool escaped = false;

	for (size_t i = 0; i + 1 < input.size(); ++i) {
		const char c = input[i];
		const char next = input[i + 1];
		if (in_double_quote) {
			if (escaped) {
				escaped = false;
			} else if (c == '\\') {
				escaped = true;
			} else if (c == '"') {
				in_double_quote = false;
			}
			continue;
		}

		if (c == '"') {
			in_double_quote = true;
			escaped = false;
			continue;
		}

		if (c == '/' && next == '/') {
			code = input.substr(0, i);
			comment = input.substr(i + 2);
			return;
		}
	}

	code = input;
	comment.clear();
}

size_t find_matching_parenthesis(const std::string& input, size_t open_pos) {
	if (open_pos >= input.size() || input[open_pos] != '(') {
		return std::string::npos;
	}

	int depth = 0;
	bool in_double_quote = false;
	bool escaped = false;
	for (size_t i = open_pos; i < input.size(); ++i) {
		const char c = input[i];
		if (in_double_quote) {
			if (escaped) {
				escaped = false;
			} else if (c == '\\') {
				escaped = true;
			} else if (c == '"') {
				in_double_quote = false;
			}
			continue;
		}

		if (c == '"') {
			in_double_quote = true;
			escaped = false;
			continue;
		}

		if (c == '(') {
			++depth;
		} else if (c == ')') {
			--depth;
			if (depth == 0) {
				return i;
			}
		}
	}
	return std::string::npos;
}

bool parse_keyword_condition(const std::string& input, const std::string& keyword, std::string& condition, std::string& tail) {
	const std::string trimmed = trim_copy(input);
	if (!starts_with_word(trimmed, keyword)) {
		return false;
	}

	size_t open_pos = trimmed.find('(', keyword.size());
	if (open_pos == std::string::npos) {
		return false;
	}
	size_t close_pos = find_matching_parenthesis(trimmed, open_pos);
	if (close_pos == std::string::npos || close_pos <= open_pos) {
		return false;
	}

	condition = trim_copy(trimmed.substr(open_pos + 1, close_pos - open_pos - 1));
	tail = trim_copy(trimmed.substr(close_pos + 1));
	return true;
}

bool parse_for_clause(const std::string& input, std::string& init, std::string& condition, std::string& iter, std::string& tail) {
	const std::string trimmed = trim_copy(input);
	if (!starts_with_word(trimmed, "for")) {
		return false;
	}

	const size_t open_pos = trimmed.find('(', 3);
	if (open_pos == std::string::npos) {
		return false;
	}
	const size_t close_pos = find_matching_parenthesis(trimmed, open_pos);
	if (close_pos == std::string::npos || close_pos <= open_pos) {
		return false;
	}

	std::string inside = trimmed.substr(open_pos + 1, close_pos - open_pos - 1);
	std::vector<std::string> parts;
	std::string current;
	bool in_double_quote = false;
	bool escaped = false;
	int depth = 0;
	for (size_t i = 0; i < inside.size(); ++i) {
		const char c = inside[i];
		if (in_double_quote) {
			current.push_back(c);
			if (escaped) {
				escaped = false;
			} else if (c == '\\') {
				escaped = true;
			} else if (c == '"') {
				in_double_quote = false;
			}
			continue;
		}
		if (c == '"') {
			current.push_back(c);
			in_double_quote = true;
			escaped = false;
			continue;
		}
		if (c == '(' || c == '[' || c == '{') {
			++depth;
			current.push_back(c);
			continue;
		}
		if ((c == ')' || c == ']' || c == '}') && depth > 0) {
			--depth;
			current.push_back(c);
			continue;
		}
		if (c == ';' && depth == 0) {
			parts.push_back(trim_copy(current));
			current.clear();
			continue;
		}
		current.push_back(c);
	}
	parts.push_back(trim_copy(current));
	if (parts.size() != 3) {
		return false;
	}

	init = trim_copy(parts[0]);
	condition = trim_copy(parts[1]);
	iter = trim_copy(parts[2]);
	tail = trim_copy(trimmed.substr(close_pos + 1));
	return true;
}

bool parse_local_function_declaration(const std::string& input, std::string& name, std::string& tail) {
	const std::string trimmed = trim_copy(input);
	if (!starts_with_word(trimmed, "function")) {
		return false;
	}
	const size_t keyword_len = 8;
	size_t i = keyword_len;
	while (i < trimmed.size() && std::isspace(static_cast<unsigned char>(trimmed[i])) != 0) {
		++i;
	}
	if (i >= trimmed.size()) {
		return false;
	}
	size_t start = i;
	while (i < trimmed.size() && is_identifier_char(static_cast<unsigned char>(trimmed[i]))) {
		++i;
	}
	if (i == start) {
		return false;
	}
	name = trimmed.substr(start, i - start);
	tail = trim_copy(trimmed.substr(i));
	return true;
}

bool parse_label_declaration(const std::string& input, std::string& label, std::string& tail) {
	const std::string trimmed = trim_copy(input);
	if (trimmed.empty()) {
		return false;
	}

	size_t colon_pos = std::string::npos;
	bool in_double_quote = false;
	bool escaped = false;
	for (size_t i = 0; i < trimmed.size(); ++i) {
		const char c = trimmed[i];
		if (in_double_quote) {
			if (escaped) {
				escaped = false;
			} else if (c == '\\') {
				escaped = true;
			} else if (c == '"') {
				in_double_quote = false;
			}
			continue;
		}
		if (c == '"') {
			in_double_quote = true;
			escaped = false;
			continue;
		}
		if (c == ':') {
			colon_pos = i;
			break;
		}
	}

	if (colon_pos == std::string::npos) {
		return false;
	}

	label = trim_copy(trimmed.substr(0, colon_pos));
	tail = trim_copy(trimmed.substr(colon_pos + 1));
	if (label.empty()) {
		return false;
	}

	for (unsigned char c : label) {
		if (!is_identifier_char(c)) {
			return false;
		}
	}

	const std::string lower = to_lower(label);
	if (lower == "default" || lower == "case") {
		return false;
	}

	return true;
}

bool is_bare_symbol_token(const std::string& input) {
	if (input.empty()) {
		return false;
	}
	for (unsigned char c : input) {
		if (!(std::isalnum(c) != 0 || c == '_' || c == '$' || c == '@' || c == '#' || c == '.' || c == '\'')) {
			return false;
		}
	}
	return true;
}

bool is_numeric_literal(const std::string& input) {
	if (input.empty()) {
		return false;
	}
	size_t i = 0;
	if (input[0] == '-' || input[0] == '+') {
		i = 1;
	}
	bool has_digit = false;
	bool has_dot = false;
	for (; i < input.size(); ++i) {
		const unsigned char c = static_cast<unsigned char>(input[i]);
		if (std::isdigit(c) != 0) {
			has_digit = true;
			continue;
		}
		if (c == '.' && !has_dot) {
			has_dot = true;
			continue;
		}
		return false;
	}
	return has_digit;
}

bool find_top_level_ternary(const std::string& input, size_t& question_pos, size_t& colon_pos) {
	question_pos = std::string::npos;
	colon_pos = std::string::npos;

	bool in_double_quote = false;
	bool escaped = false;
	int depth = 0;
	int ternary_depth = 0;

	for (size_t i = 0; i < input.size(); ++i) {
		const char c = input[i];
		if (in_double_quote) {
			if (escaped) {
				escaped = false;
			} else if (c == '\\') {
				escaped = true;
			} else if (c == '"') {
				in_double_quote = false;
			}
			continue;
		}

		if (c == '"') {
			in_double_quote = true;
			escaped = false;
			continue;
		}

		if (c == '(' || c == '[' || c == '{') {
			++depth;
			continue;
		}
		if ((c == ')' || c == ']' || c == '}') && depth > 0) {
			--depth;
			continue;
		}
		if (c == '?' && depth == 0) {
			if (ternary_depth == 0 && question_pos == std::string::npos) {
				question_pos = i;
			}
			++ternary_depth;
			continue;
		}
		if (c == ':' && depth == 0 && ternary_depth > 0) {
			--ternary_depth;
			if (ternary_depth == 0 && question_pos != std::string::npos) {
				colon_pos = i;
				return true;
			}
		}
	}

	return false;
}

std::string rewrite_ternary_expression(const std::string& input) {
	std::string text = trim_copy(input);

	// First, normalize ternary operators inside innermost parenthesized chunks.
	bool changed = true;
	while (changed) {
		changed = false;
		std::vector<size_t> stack;
		for (size_t i = 0; i < text.size(); ++i) {
			const char c = text[i];
			if (c == '(') {
				stack.push_back(i);
				continue;
			}
			if (c == ')' && !stack.empty()) {
				const size_t open = stack.back();
				stack.pop_back();
				const std::string inner = text.substr(open + 1, i - open - 1);
				size_t q = std::string::npos;
				size_t col = std::string::npos;
				if (find_top_level_ternary(inner, q, col)) {
					const std::string cond = rewrite_ternary_expression(inner.substr(0, q));
					const std::string on_true = rewrite_ternary_expression(inner.substr(q + 1, col - q - 1));
					const std::string on_false = rewrite_ternary_expression(inner.substr(col + 1));
					const std::string repl = "((" + cond + ") and (" + on_true + ") or (" + on_false + "))";
					text.replace(open + 1, i - open - 1, repl);
					changed = true;
					break;
				}
			}
		}
	}

	// Then handle top-level ternary if present.
	size_t question_pos = std::string::npos;
	size_t colon_pos = std::string::npos;
	if (find_top_level_ternary(text, question_pos, colon_pos)) {
		const std::string condition = rewrite_ternary_expression(text.substr(0, question_pos));
		const std::string when_true = rewrite_ternary_expression(text.substr(question_pos + 1, colon_pos - question_pos - 1));
		const std::string when_false = rewrite_ternary_expression(text.substr(colon_pos + 1));
		return "((" + condition + ") and (" + when_true + ") or (" + when_false + "))";
	}

	return text;
}

std::string convert_expression_to_lua(const std::string& input) {
	const std::string ternary_rewritten = rewrite_ternary_expression(input);

	std::string out;
	bool in_double_quote = false;
	bool escaped = false;

	for (size_t i = 0; i < ternary_rewritten.size(); ++i) {
		const char c = ternary_rewritten[i];
		const char next = (i + 1 < ternary_rewritten.size()) ? ternary_rewritten[i + 1] : '\0';

		if (in_double_quote) {
			out.push_back(c);
			if (escaped) {
				escaped = false;
			} else if (c == '\\') {
				escaped = true;
			} else if (c == '"') {
				in_double_quote = false;
			}
			continue;
		}

		if (c == '"') {
			out.push_back(c);
			in_double_quote = true;
			escaped = false;
			continue;
		}

		if (c == '&' && next == '&') {
			out.append(" and ");
			++i;
			continue;
		}
		if (c == '|' && next == '|') {
			out.append(" or ");
			++i;
			continue;
		}
		if (c == '!' && next == '=') {
			out.append("~=");
			++i;
			continue;
		}
		if (c == '!' && next != '=') {
			out.append(" not ");
			continue;
		}

		if (is_identifier_start(static_cast<unsigned char>(c))) {
			size_t end = i + 1;
			while (end < ternary_rewritten.size() && is_identifier_char(static_cast<unsigned char>(ternary_rewritten[end]))) {
				++end;
			}
			if (end < ternary_rewritten.size() && ternary_rewritten[end] == '$') {
				++end;
				const std::string token = ternary_rewritten.substr(i, end - i);
				out.append("_ENV[");
				out.append(lua_quote_literal(token));
				out.push_back(']');
				i = end - 1;
				continue;
			}
		}

		if (is_prefixed_variable_start(ternary_rewritten, i)) {
			size_t end = i + 1;
			while (end < ternary_rewritten.size() && is_variable_token_char(static_cast<unsigned char>(ternary_rewritten[end]))) {
				++end;
			}
			const std::string token = ternary_rewritten.substr(i, end - i);
			out.append("_ENV[");
			out.append(lua_quote_literal(token));
			out.push_back(']');
			i = end - 1;
			continue;
		}
		if (std::isdigit(static_cast<unsigned char>(c)) != 0) {
			size_t end = i + 1;
			while (end < ternary_rewritten.size() && is_identifier_char(static_cast<unsigned char>(ternary_rewritten[end]))) {
				++end;
			}
			if (end > i + 1) {
				const std::string token = ternary_rewritten.substr(i, end - i);
				if (token.find('_') != std::string::npos && !is_numeric_literal(token)) {
					out.append("_ENV[");
					out.append(lua_quote_literal(token));
					out.push_back(']');
					i = end - 1;
					continue;
				}
			}
		}

		out.push_back(c);
	}

	std::string converted = trim_copy(out);
	{
		std::string normalized;
		normalized.reserve(converted.size());
		bool in_double_quote_norm = false;
		bool escaped_norm = false;
		for (size_t i = 0; i < converted.size(); ++i) {
			const char c = converted[i];
			if (in_double_quote_norm) {
				normalized.push_back(c);
				if (escaped_norm) {
					escaped_norm = false;
				} else if (c == '\\') {
					escaped_norm = true;
				} else if (c == '"') {
					in_double_quote_norm = false;
				}
				continue;
			}
			if (c == '"') {
				normalized.push_back(c);
				in_double_quote_norm = true;
				escaped_norm = false;
				continue;
			}
			if (c == '+') {
				size_t l = i;
				while (l > 0 && std::isspace(static_cast<unsigned char>(converted[l - 1])) != 0) {
					--l;
				}
				size_t r = i + 1;
				while (r < converted.size() && std::isspace(static_cast<unsigned char>(converted[r])) != 0) {
					++r;
				}
				const bool left_is_quote = (l > 0 && converted[l - 1] == '"');
				const bool right_is_quote = (r < converted.size() && converted[r] == '"');
				bool left_is_string_var = false;
				bool right_is_string_var = false;
				if (!left_is_quote && l > 0) {
					size_t k = l;
					while (k > 0 && std::isspace(static_cast<unsigned char>(converted[k - 1])) == 0) {
						--k;
					}
					const std::string left_token = converted.substr(k, l - k);
					left_is_string_var = left_token.find("_ENV[\"") != std::string::npos && left_token.find("$\"]") != std::string::npos;
				}
				if (!right_is_quote && r < converted.size()) {
					size_t k = r;
					while (k < converted.size() && std::isspace(static_cast<unsigned char>(converted[k])) == 0) {
						++k;
					}
					const std::string right_token = converted.substr(r, k - r);
					right_is_string_var = right_token.find("_ENV[\"") != std::string::npos && right_token.find("$\"]") != std::string::npos;
				}
				if (left_is_quote || right_is_quote || left_is_string_var || right_is_string_var) {
					normalized.append(" .. ");
					continue;
				}
			}
			normalized.push_back(c);
		}
		converted = trim_copy(normalized);
	}
	size_t question = std::string::npos;
	size_t colon = std::string::npos;
	bool in_double_quote_scan = false;
	bool escaped_scan = false;
	for (size_t i = 0; i < converted.size(); ++i) {
		const char c = converted[i];
		if (in_double_quote_scan) {
			if (escaped_scan) {
				escaped_scan = false;
			} else if (c == '\\') {
				escaped_scan = true;
			} else if (c == '"') {
				in_double_quote_scan = false;
			}
			continue;
		}
		if (c == '"') {
			in_double_quote_scan = true;
			escaped_scan = false;
			continue;
		}
		if (c == '?' && question == std::string::npos) {
			question = i;
			continue;
		}
		if (c == ':' && question != std::string::npos) {
			colon = i;
			break;
		}
	}

	if (question != std::string::npos && colon != std::string::npos && question < colon) {
		const std::string cond = trim_copy(converted.substr(0, question));
		const std::string on_true = trim_copy(converted.substr(question + 1, colon - question - 1));
		const std::string on_false = trim_copy(converted.substr(colon + 1));
		return "((" + cond + ") and (" + on_true + ") or (" + on_false + "))";
	}

	return converted;
}

size_t find_assignment_operator(const std::string& input) {
	bool in_double_quote = false;
	bool escaped = false;
	int depth = 0;
	for (size_t i = 0; i < input.size(); ++i) {
		const char c = input[i];
		const char prev = (i > 0) ? input[i - 1] : '\0';
		const char next = (i + 1 < input.size()) ? input[i + 1] : '\0';

		if (in_double_quote) {
			if (escaped) {
				escaped = false;
			} else if (c == '\\') {
				escaped = true;
			} else if (c == '"') {
				in_double_quote = false;
			}
			continue;
		}

		if (c == '"') {
			in_double_quote = true;
			escaped = false;
			continue;
		}

		if (c == '(' || c == '[' || c == '{') {
			++depth;
			continue;
		}
		if ((c == ')' || c == ']' || c == '}') && depth > 0) {
			--depth;
			continue;
		}

		if (depth == 0 && c == '=' && prev != '=' && prev != '!' && prev != '<' && prev != '>' && next != '=') {
			return i;
		}
	}
	return std::string::npos;
}

bool command_first_arg_needs_string(const std::string& command_lower) {
	return command_lower == "set"
		|| command_lower == "setr"
		|| command_lower == "setd"
		|| command_lower == "setarray"
		|| command_lower == "copyarray"
		|| command_lower == "cleararray"
		|| command_lower == "input"
		|| command_lower == "callsub"
		|| command_lower == "goto";
}

bool is_quoted_string_literal(const std::string& arg) {
	return arg.size() >= 2 && arg.front() == '"' && arg.back() == '"';
}

bool is_lua_keyword_command(const std::string& command_lower) {
	return command_lower == "goto" || command_lower == "function";
}

std::string convert_single_statement_to_lua(const std::string& input) {
	std::string statement = trim_copy(input);
	if (statement.empty()) {
		return "";
	}

	if (statement.back() == ';') {
		statement.pop_back();
		statement = trim_copy(statement);
	}

	if (statement.empty()) {
		return "";
	}

	const std::string lower = to_lower(statement);
	if (lower == "end") {
		// DSL `end;` means terminate current script execution immediately.
		// Use a scoped return so generated Lua remains valid even when
		// additional translated statements appear afterwards.
		return "do return end";
	}
	if (lower == "break") {
		return "break";
	}
	if (lower == "continue") {
		return "-- continue";
	}
	if (lower == "return") {
		return "return";
	}
	if (starts_with_word(lower, "for")) {
		return "-- unsupported for-loop: " + statement;
	}
	if (starts_with_word(lower, "switch")
		|| starts_with_word(lower, "case")
		|| starts_with_word(lower, "default")) {
		// Control-flow tokens should be handled by block conversion. If one
		// leaks here, keep syntax valid instead of emitting malformed Lua.
		return "-- unsupported control-flow statement: " + statement;
	}

	if (statement.size() > 2 && starts_with(statement, "++")) {
		const std::string target = trim_copy(statement.substr(2));
		const std::string expr = convert_expression_to_lua(target);
		return expr + " = " + expr + " + 1";
	}
	if (statement.size() > 2 && starts_with(statement, "--")) {
		const std::string target = trim_copy(statement.substr(2));
		const std::string expr = convert_expression_to_lua(target);
		return expr + " = " + expr + " - 1";
	}

	if (statement.size() > 2 && statement.compare(statement.size() - 2, 2, "++") == 0) {
		const std::string target = trim_copy(statement.substr(0, statement.size() - 2));
		const std::string expr = convert_expression_to_lua(target);
		return expr + " = " + expr + " + 1";
	}
	if (statement.size() > 2 && statement.compare(statement.size() - 2, 2, "--") == 0) {
		const std::string target = trim_copy(statement.substr(0, statement.size() - 2));
		const std::string expr = convert_expression_to_lua(target);
		return expr + " = " + expr + " - 1";
	}

	const size_t assign_pos = find_assignment_operator(statement);
	if (assign_pos != std::string::npos) {
		const std::string lhs = trim_copy(statement.substr(0, assign_pos));
		const std::string rhs = trim_copy(statement.substr(assign_pos + 1));
		if (!lhs.empty()) {
			const char op = lhs.back();
			if (op == '+' || op == '-' || op == '*' || op == '/' || op == '%') {
				const std::string target = trim_copy(lhs.substr(0, lhs.size() - 1));
				const std::string expr = convert_expression_to_lua(target);
				return expr + " = " + expr + " " + std::string(1, op) + " " + convert_expression_to_lua(rhs);
			}
		}
		return convert_expression_to_lua(lhs) + " = " + convert_expression_to_lua(rhs);
	}

	size_t name_end = 0;
	if (!is_identifier_start(static_cast<unsigned char>(statement[0]))) {
		return convert_expression_to_lua(statement);
	}
	while (name_end < statement.size()) {
		const unsigned char c = static_cast<unsigned char>(statement[name_end]);
		if (!is_identifier_char(c)) {
			break;
		}
		++name_end;
	}

	if (name_end == 0) {
		return convert_expression_to_lua(statement);
	}

	const std::string command = statement.substr(0, name_end);
	const std::string command_lower = to_lower(command);
	std::string rest = trim_copy(statement.substr(name_end));

	if (rest.empty()) {
		return command + "()";
	}

	std::vector<std::string> args;
	if (rest.front() == '(' && rest.back() == ')' && find_matching_parenthesis(rest, 0) == rest.size() - 1) {
		args = split_args_top_level(rest.substr(1, rest.size() - 2));
	} else {
		args = split_args_top_level(rest);
	}

	std::vector<std::string> converted;
	converted.reserve(args.size());
	for (size_t i = 0; i < args.size(); ++i) {
		const std::string arg = trim_copy(args[i]);
		if (arg.empty()) {
			converted.emplace_back("nil");
			continue;
		}
		if (arg == "-") {
			converted.emplace_back("nil");
			continue;
		}
		if ((command_lower == "setarray" && i == 0)
			|| (command_lower == "copyarray" && (i == 0 || i == 1))
			|| (command_lower == "cleararray" && i == 0)
			|| (command_lower == "getmapxy" && i < 3)) {
			if (is_quoted_string_literal(arg)) {
				converted.push_back(arg);
			} else {
				converted.push_back(lua_quote_literal(arg));
			}
			continue;
		}

		if (i == 0 && command_first_arg_needs_string(command_lower)
			&& is_bare_symbol_token(arg) && !is_numeric_literal(arg)) {
			converted.push_back(lua_quote_literal(arg));
			continue;
		}

		converted.push_back(convert_expression_to_lua(arg));
	}

	std::ostringstream out;
	if (is_lua_keyword_command(command_lower)) {
		out << "_ENV[" << lua_quote_literal(command) << "](";
	} else {
		out << command << "(";
	}
	for (size_t i = 0; i < converted.size(); ++i) {
		if (i > 0) {
			out << ", ";
		}
		out << converted[i];
	}
	out << ")";
	return out.str();
}

std::vector<std::string> convert_statement_to_lua_lines(const std::string& input) {
	std::vector<std::string> converted;
	std::vector<std::string> statements;

	std::string current;
	bool in_double_quote = false;
	bool escaped = false;
	int depth = 0;
	for (size_t i = 0; i < input.size(); ++i) {
		const char c = input[i];
		if (in_double_quote) {
			current.push_back(c);
			if (escaped) {
				escaped = false;
			} else if (c == '\\') {
				escaped = true;
			} else if (c == '"') {
				in_double_quote = false;
			}
			continue;
		}

		if (c == '"') {
			current.push_back(c);
			in_double_quote = true;
			escaped = false;
			continue;
		}

		if (c == '(' || c == '[' || c == '{') {
			++depth;
			current.push_back(c);
			continue;
		}
		if ((c == ')' || c == ']' || c == '}') && depth > 0) {
			--depth;
			current.push_back(c);
			continue;
		}

		if (c == ';' && depth == 0) {
			const std::string stmt = trim_copy(current);
			if (!stmt.empty()) {
				statements.push_back(stmt);
			}
			current.clear();
			continue;
		}

		current.push_back(c);
	}

	const std::string tail = trim_copy(current);
	if (!tail.empty()) {
		statements.push_back(tail);
	}

	for (size_t i = 0; i < statements.size();) {
		const std::string stmt = trim_copy(statements[i]);
		const std::string stmt_lower = to_lower(stmt);
		if (starts_with_word(stmt_lower, "if")) {
			std::string condition;
			std::string tail;
			if (parse_keyword_condition(stmt, "if", condition, tail) && !tail.empty() && tail != "{") {
				converted.push_back("if " + convert_expression_to_lua(condition) + " then");
				const std::string then_line = convert_single_statement_to_lua(tail);
				if (!then_line.empty()) {
					converted.push_back(then_line);
				}
				if (i + 1 < statements.size()) {
					const std::string next = trim_copy(statements[i + 1]);
					const std::string next_lower = to_lower(next);
					if (starts_with_word(next_lower, "else")) {
						converted.push_back("else");
						const std::string else_body = trim_copy(next.substr(4));
						const std::string else_line = convert_single_statement_to_lua(else_body);
						if (!else_line.empty()) {
							converted.push_back(else_line);
						}
						if (i + 2 < statements.size() && to_lower(trim_copy(statements[i + 2])) == "end") {
							converted.push_back("end");
							i += 3;
							continue;
						}
						converted.push_back("end");
						i += 2;
						continue;
					}
					if (to_lower(next) == "end") {
						converted.push_back("end");
						i += 2;
						continue;
					}
				}
				converted.push_back("end");
				++i;
				continue;
			}
		}
		if (i + 2 < statements.size()) {
			const std::string next = trim_copy(statements[i + 1]);
			const std::string next_lower = to_lower(next);
			const std::string third = trim_copy(statements[i + 2]);
			const std::string third_lower = to_lower(third);
			if (starts_with_word(stmt_lower, "if")
				&& starts_with_word(next_lower, "else")
				&& third_lower == "end") {
				std::string condition;
				std::string tail;
				if (parse_keyword_condition(stmt, "if", condition, tail) && !tail.empty()) {
					converted.push_back("if " + convert_expression_to_lua(condition) + " then");
					const std::string then_line = convert_single_statement_to_lua(tail);
					if (!then_line.empty()) {
						converted.push_back(then_line);
					}
					converted.push_back("else");
					const std::string else_body = trim_copy(next.substr(4));
					const std::string else_line = convert_single_statement_to_lua(else_body);
					if (!else_line.empty()) {
						converted.push_back(else_line);
					}
					converted.push_back("end");
					i += 3;
					continue;
				}
			}
		}

		const std::string line = convert_single_statement_to_lua(stmt);
		const size_t assign_pos = find_assignment_operator(stmt);
		if (assign_pos != std::string::npos) {
			const std::string lhs = trim_copy(stmt.substr(0, assign_pos));
			const std::string rhs = trim_copy(stmt.substr(assign_pos + 1));
			const size_t rhs_assign_pos = find_assignment_operator(rhs);
			if (!lhs.empty() && rhs_assign_pos != std::string::npos) {
				const std::string rhs_lhs = trim_copy(rhs.substr(0, rhs_assign_pos));
				const std::string rhs_rhs = trim_copy(rhs.substr(rhs_assign_pos + 1));
				if (!rhs_lhs.empty() && !rhs_rhs.empty()) {
					converted.push_back(convert_expression_to_lua(rhs_lhs) + " = " + convert_expression_to_lua(rhs_rhs));
					converted.push_back(convert_expression_to_lua(lhs) + " = " + convert_expression_to_lua(rhs_rhs));
					++i;
					continue;
				}
			}
		}
		if (!line.empty()) {
			converted.push_back(line);
		}
		++i;
	}

	return converted;
}

std::string next_significant_code(const std::vector<std::pair<size_t, std::string>>& lines, size_t start) {
	for (size_t i = start; i < lines.size(); ++i) {
		std::string code;
		std::string comment;
		split_line_comment(lines[i].second, code, comment);
		const std::string trimmed = trim_copy(code);
		if (!trimmed.empty()) {
			return trimmed;
		}
	}
	return "";
}

bool is_else_clause(const std::string& input) {
	const std::string trimmed = trim_copy(input);
	if (starts_with_word(trimmed, "else")) {
		return true;
	}
	if (!trimmed.empty() && trimmed[0] == '}') {
		return starts_with_word(trim_copy(trimmed.substr(1)), "else");
	}
	return false;
}

bool is_direct_else_clause(const std::string& input) {
	const std::string trimmed = trim_copy(input);
	return starts_with_word(trimmed, "else");
}

bool should_buffer_multiline_statement(const std::string& statement) {
	const std::string lower = to_lower(statement);
	if (statement.empty()) {
		return false;
	}
	if (statement.find(';') != std::string::npos) {
		return false;
	}
	if (statement[0] == '}') {
		return false;
	}
	if (statement == "{") {
		return false;
	}
	if (starts_with(lower, "default:")) {
		return false;
	}
	if (starts_with(lower, "case ") || starts_with(lower, "case\t")) {
		return false;
	}
	if (starts_with_word(lower, "if")
		|| starts_with_word(lower, "else")
		|| starts_with_word(lower, "while")
		|| starts_with_word(lower, "switch")
		|| starts_with_word(lower, "case")
		|| starts_with_word(lower, "default")
		|| starts_with_word(lower, "for")
		|| starts_with_word(lower, "function")) {
		return false;
	}
	return true;
}

bool ends_with_binary_operator(const std::string& statement) {
	const std::string t = trim_copy(statement);
	if (t.size() < 2) {
		return false;
	}
	return t.compare(t.size() - 2, 2, "||") == 0 || t.compare(t.size() - 2, 2, "&&") == 0;
}

void convert_block_to_lua(LuaCodeBlock& block, std::vector<UnsupportedItem>& unsupported) {
	enum class FrameKind {
		If,
		While,
		For,
		PendingFor,
		Switch,
		InlineIf,
		PendingIf,
		Function,
	};

	struct Frame {
		FrameKind kind{};
		int switch_id = 0;
		bool switch_seen_case = false;
		bool switch_emitted_default = false;
		std::string for_iter;
	};

	std::vector<Frame> stack;
	int switch_counter = 0;
	bool multiline_setarray = false;
	std::string setarray_buffer;
	size_t setarray_line = 0;
	bool multiline_statement = false;
	std::string statement_buffer;
	size_t statement_line = 0;
	bool append_setarray_args = false;
	size_t append_setarray_index = 0;
	bool in_block_comment = false;
	bool multiline_operator = false;
	std::string operator_buffer;
	size_t operator_line = 0;
	std::vector<std::string> local_functions;

	auto close_frame = [&](size_t line) {
		if (stack.empty()) {
			unsupported.push_back({line, "unexpected closing brace"});
			return;
		}
		const Frame frame = stack.back();
		stack.pop_back();
		if (frame.kind == FrameKind::For && !frame.for_iter.empty()) {
			const std::vector<std::string> iter_lines = convert_statement_to_lua_lines(frame.for_iter);
			for (const std::string& iter_line : iter_lines) {
				if (!iter_line.empty()) {
					block.lua_lines.push_back(iter_line);
				}
			}
		}
		block.lua_lines.push_back("end");
	};

	auto append_statement = [&](const std::string& statement_text) {
		auto has_loop_frame = [&]() -> bool {
			for (const Frame& frame : stack) {
				if (frame.kind == FrameKind::While || frame.kind == FrameKind::For) {
					return true;
				}
			}
			return false;
		};

		std::string normalized = statement_text;
		const size_t assign_pos = find_assignment_operator(normalized);
		if (assign_pos != std::string::npos) {
			const std::string lhs = trim_copy(normalized.substr(0, assign_pos));
			std::string rhs = trim_copy(normalized.substr(assign_pos + 1));
			if (!rhs.empty() && rhs.back() == ';') {
				rhs.pop_back();
				rhs = trim_copy(rhs);
			}
			for (const std::string& fn : local_functions) {
				if (rhs == fn) {
					normalized = lhs + " = " + fn + "()";
					break;
				}
			}
		}

		const std::vector<std::string> lines = convert_statement_to_lua_lines(normalized);
		for (const std::string& line : lines) {
			std::string adjusted = line;
			if (adjusted == "break" && !has_loop_frame()) {
				adjusted = "-- break";
			}
			if (adjusted == "goto __continue" && !has_loop_frame()) {
				adjusted = "-- continue";
			}
			block.lua_lines.push_back(adjusted);
		}
	};

	auto append_setarray_value = [&](const std::string& value) {
		if (append_setarray_index >= block.lua_lines.size()) {
			return;
		}
		std::string& line = block.lua_lines[append_setarray_index];
		const std::string expression = convert_expression_to_lua(value);
		const size_t nil_marker = line.rfind(", nil)");
		if (nil_marker != std::string::npos) {
			line.replace(nil_marker, 6, ", " + expression + ")");
			return;
		}
		if (!line.empty() && line.back() == ')') {
			line.pop_back();
			line += ", " + expression + ")";
		}
	};
	auto append_maybe_braced = [&](const std::string& text) {
		std::string value = trim_copy(text);
		if (value.size() >= 2 && value.front() == '{' && value.back() == '}') {
			value = trim_copy(value.substr(1, value.size() - 2));
		}
		if (!value.empty()) {
			append_statement(value);
		}
	};
	auto is_incomplete_control_header = [&](const std::string& value) -> bool {
		const std::string lowered = to_lower(value);
		std::string condition;
		std::string tail;
		if (starts_with_word(lowered, "if")) {
			return !parse_keyword_condition(value, "if", condition, tail);
		}
		if (starts_with_word(lowered, "while")) {
			return !parse_keyword_condition(value, "while", condition, tail);
		}
		if (starts_with_word(lowered, "switch")) {
			return !parse_keyword_condition(value, "switch", condition, tail);
		}
		return false;
	};

	for (size_t i = 0; i < block.source_lines.size(); ++i) {
		const size_t line_no = block.source_lines[i].first;
		std::string raw = block.source_lines[i].second;
		if (!raw.empty() && raw.back() == '\r') {
			raw.pop_back();
		}
		std::string filtered;
		filtered.reserve(raw.size());
		for (size_t j = 0; j < raw.size(); ++j) {
			const char c = raw[j];
			const char next = (j + 1 < raw.size()) ? raw[j + 1] : '\0';
			if (in_block_comment) {
				if (c == '*' && next == '/') {
					in_block_comment = false;
					++j;
				}
				continue;
			}
			if (c == '/' && next == '*') {
				in_block_comment = true;
				++j;
				continue;
			}
			filtered.push_back(c);
		}

		std::string code;
		std::string comment;
		split_line_comment(filtered, code, comment);
		std::string statement = trim_copy(code);

		if (append_setarray_args) {
			if (statement.empty()) {
				continue;
			}

			std::string value = statement;
			const bool ends_setarray = value.find(';') != std::string::npos;
			if (!value.empty() && value.back() == ';') {
				value.pop_back();
				value = trim_copy(value);
			}
			if (!value.empty() && value.back() == ',') {
				value.pop_back();
				value = trim_copy(value);
			}
			if (!value.empty()) {
				append_setarray_value(value);
			}
			if (ends_setarray) {
				append_setarray_args = false;
			}
			continue;
		}

		if (multiline_setarray) {
			if (!statement.empty()) {
				if (!setarray_buffer.empty()) {
					setarray_buffer.push_back(' ');
				}
				setarray_buffer += statement;
			}
			if (statement.find(';') != std::string::npos) {
				statement = setarray_buffer;
				setarray_buffer.clear();
				multiline_setarray = false;
			} else {
				continue;
			}
		} else if (starts_with_word(statement, "setarray") && statement.find(';') == std::string::npos) {
			multiline_setarray = true;
			setarray_buffer = statement;
			setarray_line = line_no;
			continue;
		}
		if (multiline_operator) {
			if (!statement.empty()) {
				if (!operator_buffer.empty()) {
					operator_buffer.push_back(' ');
				}
				operator_buffer += statement;
			}
			if (ends_with_binary_operator(statement)) {
				continue;
			}
			statement = trim_copy(operator_buffer);
			operator_buffer.clear();
			multiline_operator = false;
		} else if (ends_with_binary_operator(statement)) {
			multiline_operator = true;
			operator_buffer = statement;
			operator_line = line_no;
			continue;
		}
		if (multiline_statement) {
			if (!statement.empty()) {
				if (!statement_buffer.empty()) {
					statement_buffer.push_back(' ');
				}
				statement_buffer += statement;
			}
			if (statement.find(';') != std::string::npos) {
				statement = statement_buffer;
				statement_buffer.clear();
				multiline_statement = false;
			} else {
				continue;
			}
		} else if (should_buffer_multiline_statement(statement)) {
			multiline_statement = true;
			statement_buffer = statement;
			statement_line = line_no;
			continue;
		}
		if (!multiline_statement && statement.find(';') == std::string::npos && is_incomplete_control_header(statement)) {
			multiline_statement = true;
			statement_buffer = statement;
			statement_line = line_no;
			continue;
		}
		if (statement.size() >= 2 && statement.front() == '{' && statement.back() == '}') {
			const std::string inner = trim_copy(statement.substr(1, statement.size() - 2));
			if (!inner.empty()) {
				append_statement(inner);
			}
			if (!comment.empty()) {
				block.lua_lines.push_back("-- " + trim_copy(comment));
			}
			continue;
		}

		if (!stack.empty() && stack.back().kind == FrameKind::InlineIf && !is_direct_else_clause(statement)) {
			stack.pop_back();
			block.lua_lines.push_back("end");
		}

		bool joined_else = false;
		if (!statement.empty() && statement[0] == '}') {
			const std::string tail = trim_copy(statement.substr(1));
			if (starts_with_word(tail, "else")) {
				statement = tail;
				joined_else = true;
			}
		}

		if (!joined_else) {
			while (!statement.empty() && statement[0] == '}') {
				const std::string remainder = trim_copy(statement.substr(1));
				if (remainder.empty() && !stack.empty()
					&& (stack.back().kind == FrameKind::If
						|| stack.back().kind == FrameKind::InlineIf)) {
					const std::string next = next_significant_code(block.source_lines, i + 1);
					if (is_direct_else_clause(next)) {
						statement.clear();
						break;
					}
				}
				close_frame(line_no);
				statement = remainder;
			}
		}

		if (statement.empty()) {
			if (!comment.empty()) {
				block.lua_lines.push_back("-- " + trim_copy(comment));
			}
			continue;
		}
		if (statement == "{") {
			if (!stack.empty() && stack.back().kind == FrameKind::PendingIf) {
				stack.back().kind = FrameKind::If;
			}
			if (!comment.empty()) {
				block.lua_lines.push_back("-- " + trim_copy(comment));
			}
			continue;
		}

		if (!stack.empty() && stack.back().kind == FrameKind::PendingIf && !is_direct_else_clause(statement)) {
			if (!statement.empty() && statement.front() == '{') {
				stack.back().kind = FrameKind::If;
				if (!comment.empty()) {
					block.lua_lines.push_back("-- " + trim_copy(comment));
				}
				continue;
			}
			const std::string pending_lower = to_lower(statement);
			if (starts_with_word(pending_lower, "case") || starts_with_word(pending_lower, "default")) {
				stack.pop_back();
				block.lua_lines.push_back("end");
			} else {
				stack.pop_back();
				if (starts_with_word(statement, "for")) {
					unsupported.push_back({line_no, "for loop conversion is not supported yet"});
					block.lua_lines.push_back("-- unsupported for-loop: " + statement);
				} else {
					append_statement(statement);
				}
				const std::string next = next_significant_code(block.source_lines, i + 1);
				if (is_direct_else_clause(next)) {
					stack.push_back({FrameKind::InlineIf, 0});
				} else {
					block.lua_lines.push_back("end");
				}
				if (!comment.empty()) {
					block.lua_lines.push_back("-- " + trim_copy(comment));
				}
				continue;
			}
		}
		if (!stack.empty() && stack.back().kind == FrameKind::PendingFor) {
			if (!statement.empty() && statement.front() == '{') {
				stack.back().kind = FrameKind::For;
				if (!comment.empty()) {
					block.lua_lines.push_back("-- " + trim_copy(comment));
				}
				continue;
			}
			append_statement(statement);
			if (!stack.back().for_iter.empty()) {
				const std::vector<std::string> iter_lines = convert_statement_to_lua_lines(stack.back().for_iter);
				for (const std::string& iter_line : iter_lines) {
					if (!iter_line.empty()) {
						block.lua_lines.push_back(iter_line);
					}
				}
			}
			stack.pop_back();
			block.lua_lines.push_back("end");
			if (!comment.empty()) {
				block.lua_lines.push_back("-- " + trim_copy(comment));
			}
			continue;
		}
		const std::string lower_statement = to_lower(statement);
		if (starts_with(lower_statement, "default:")) {
			const std::string after_default = trim_copy(statement.substr(8));
			if (!after_default.empty() &&
			    (starts_with(to_lower(after_default), "case ") || starts_with(to_lower(after_default), "case\t") || starts_with_word(to_lower(after_default), "case"))) {
				statement = after_default;
			}
		}

		std::string condition;
		std::string tail;
		const std::string lowered = to_lower(statement);
		if (starts_with_word(lowered, "if") && parse_keyword_condition(statement, "if", condition, tail)) {
			const std::string tail_lower = to_lower(tail);
			if (starts_with(tail, "&&") || starts_with(tail, "||")
				|| starts_with_word(tail_lower, "and") || starts_with_word(tail_lower, "or")) {
				std::string extended_tail = tail;
				bool has_open_brace = false;
				if (!extended_tail.empty() && extended_tail.back() == '{') {
					has_open_brace = true;
					extended_tail = trim_copy(extended_tail.substr(0, extended_tail.size() - 1));
				}
				condition = trim_copy(condition + " " + extended_tail);
				tail = has_open_brace ? "{" : "";
			}
			block.lua_lines.push_back("if " + convert_expression_to_lua(condition) + " then");
			if (tail == "{") {
				stack.push_back({FrameKind::If, 0});
			} else if (tail.empty()) {
				stack.push_back({FrameKind::PendingIf, 0});
			} else if (!tail.empty() && tail.front() == '{' && tail.back() == '}') {
				append_maybe_braced(tail);
				const std::string next = next_significant_code(block.source_lines, i + 1);
				if (is_direct_else_clause(next)) {
					stack.push_back({FrameKind::InlineIf, 0});
				} else {
					block.lua_lines.push_back("end");
				}
			} else if (!tail.empty() && tail.front() == '{') {
				stack.push_back({FrameKind::If, 0});
				const std::string inline_body = trim_copy(tail.substr(1));
				if (!inline_body.empty()) {
					append_statement(inline_body);
				}
			} else {
				append_maybe_braced(tail);
				const std::string next = next_significant_code(block.source_lines, i + 1);
				if (is_direct_else_clause(next)) {
					stack.push_back({FrameKind::InlineIf, 0});
				} else {
					block.lua_lines.push_back("end");
				}
			}
		} else if (starts_with_word(lowered, "else")) {
			std::string rest = trim_copy(statement.substr(4));
			if (starts_with_word(to_lower(rest), "if")) {
				if (stack.empty() || (stack.back().kind != FrameKind::If && stack.back().kind != FrameKind::InlineIf && stack.back().kind != FrameKind::PendingIf)) {
					unsupported.push_back({line_no, "else-if without matching if"});
					continue;
				}
				if (!parse_keyword_condition(rest, "if", condition, tail)) {
					unsupported.push_back({line_no, "invalid else-if condition"});
					continue;
				}
				const std::string tail_lower = to_lower(tail);
				if (starts_with(tail, "&&") || starts_with(tail, "||")
					|| starts_with_word(tail_lower, "and") || starts_with_word(tail_lower, "or")) {
					std::string extended_tail = tail;
					bool has_open_brace = false;
					if (!extended_tail.empty() && extended_tail.back() == '{') {
						has_open_brace = true;
						extended_tail = trim_copy(extended_tail.substr(0, extended_tail.size() - 1));
					}
					condition = trim_copy(condition + " " + extended_tail);
					tail = has_open_brace ? "{" : "";
				}
				block.lua_lines.push_back("elseif " + convert_expression_to_lua(condition) + " then");
				if (tail == "{") {
					stack.back().kind = FrameKind::If;
				} else if (tail.empty()) {
					stack.back().kind = FrameKind::PendingIf;
				} else if (!tail.empty() && tail.front() == '{' && tail.back() == '}') {
					append_maybe_braced(tail);
					const std::string next = next_significant_code(block.source_lines, i + 1);
					if (!is_direct_else_clause(next)) {
						stack.pop_back();
						block.lua_lines.push_back("end");
					} else {
						stack.back().kind = FrameKind::InlineIf;
					}
				} else if (!tail.empty() && tail.front() == '{') {
					stack.back().kind = FrameKind::If;
					const std::string inline_body = trim_copy(tail.substr(1));
					if (!inline_body.empty()) {
						append_statement(inline_body);
					}
				} else {
					append_maybe_braced(tail);
					const std::string next = next_significant_code(block.source_lines, i + 1);
					if (!is_direct_else_clause(next)) {
						stack.pop_back();
						block.lua_lines.push_back("end");
					} else {
						stack.back().kind = FrameKind::InlineIf;
					}
				}
			} else {
				if (stack.empty() || (stack.back().kind != FrameKind::If && stack.back().kind != FrameKind::InlineIf && stack.back().kind != FrameKind::PendingIf)) {
					unsupported.push_back({line_no, "else without matching if"});
					continue;
				}
				block.lua_lines.push_back("else");
				if (rest == "{") {
					stack.back().kind = FrameKind::If;
				} else if (rest.empty()) {
					stack.back().kind = FrameKind::PendingIf;
				} else if (!rest.empty() && rest.front() == '{' && rest.back() == '}') {
					append_maybe_braced(rest);
					stack.pop_back();
					block.lua_lines.push_back("end");
				} else if (!rest.empty() && rest.front() == '{') {
					stack.back().kind = FrameKind::If;
					const std::string inline_body = trim_copy(rest.substr(1));
					if (!inline_body.empty()) {
						append_statement(inline_body);
					}
				} else if (!rest.empty()) {
					append_maybe_braced(rest);
					stack.pop_back();
					block.lua_lines.push_back("end");
				}
			}
		} else if (starts_with_word(lowered, "while") && parse_keyword_condition(statement, "while", condition, tail)) {
			if (tail == "{") {
				block.lua_lines.push_back("while " + convert_expression_to_lua(condition) + " do");
				stack.push_back({FrameKind::While, 0});
			} else {
				block.lua_lines.push_back("while " + convert_expression_to_lua(condition) + " do");
				append_maybe_braced(tail);
				block.lua_lines.push_back("end");
			}
		} else if (starts_with_word(lowered, "for")) {
			std::string init_expr;
			std::string cond_expr;
			std::string iter_expr;
			std::string for_tail;
			if (!parse_for_clause(statement, init_expr, cond_expr, iter_expr, for_tail)) {
				unsupported.push_back({line_no, "for loop conversion is not supported yet"});
				continue;
			}
			if (!init_expr.empty()) {
				append_statement(init_expr);
			}
			const std::string lua_cond = cond_expr.empty() ? "true" : convert_expression_to_lua(cond_expr);
			block.lua_lines.push_back("while " + lua_cond + " do");
			if (for_tail == "{") {
				Frame frame;
				frame.kind = FrameKind::For;
				frame.for_iter = iter_expr;
				stack.push_back(std::move(frame));
			} else if (for_tail.empty()) {
				Frame frame;
				frame.kind = FrameKind::PendingFor;
				frame.for_iter = iter_expr;
				stack.push_back(std::move(frame));
			} else {
				append_maybe_braced(for_tail);
				if (!iter_expr.empty()) {
					append_statement(iter_expr);
				}
				block.lua_lines.push_back("end");
			}
		} else if (starts_with_word(lowered, "function")) {
			std::string func_name;
			std::string func_tail;
			if (!parse_local_function_declaration(statement, func_name, func_tail)) {
				if (statement.find('{') == std::string::npos && statement.find(';') != std::string::npos) {
					continue;
				}
				unsupported.push_back({line_no, "unsupported local function declaration"});
				continue;
			}
			if (func_tail.empty() || func_tail == ";") {
				local_functions.push_back(func_name);
				continue;
			}
			if (func_tail != "{") {
				unsupported.push_back({line_no, "unsupported local function declaration"});
				continue;
			}
			block.lua_lines.push_back("local function " + func_name + "()");
			local_functions.push_back(func_name);
			Frame frame;
			frame.kind = FrameKind::Function;
			stack.push_back(std::move(frame));
		} else if (starts_with_word(lowered, "switch") && parse_keyword_condition(statement, "switch", condition, tail)) {
			bool has_open_brace = (tail == "{");
			if (!has_open_brace && tail.empty()) {
				has_open_brace = (trim_copy(next_significant_code(block.source_lines, i + 1)) == "{");
			}
			if (!has_open_brace) {
				unsupported.push_back({line_no, "switch without opening brace"});
				continue;
			}
			++switch_counter;
			block.lua_lines.push_back("local __switch" + std::to_string(switch_counter) + " = " + convert_expression_to_lua(condition));
			block.lua_lines.push_back("if false then");
			Frame frame;
			frame.kind = FrameKind::Switch;
			frame.switch_id = switch_counter;
			stack.push_back(frame);
		} else if (starts_with(lowered, "case ") || starts_with(lowered, "case\t") || starts_with_word(lowered, "case")) {
			if (stack.empty() || stack.back().kind != FrameKind::Switch) {
				unsupported.push_back({line_no, "case outside switch"});
				continue;
			}
			if (stack.back().switch_emitted_default) {
				unsupported.push_back({line_no, "case after default is not supported"});
				continue;
			}
			size_t colon = statement.find(':');
			if (colon == std::string::npos) {
				unsupported.push_back({line_no, "case is missing ':'"});
				continue;
			}
			const std::string expr = trim_copy(statement.substr(4, colon - 4));
			block.lua_lines.push_back("elseif __switch" + std::to_string(stack.back().switch_id) + " == " + convert_expression_to_lua(expr) + " then");
			stack.back().switch_seen_case = true;
			const std::string rest = trim_copy(statement.substr(colon + 1));
			append_maybe_braced(rest);
		} else if (starts_with(lowered, "default:") || starts_with_word(lowered, "default")) {
			if (stack.empty() || stack.back().kind != FrameKind::Switch) {
				unsupported.push_back({line_no, "default outside switch"});
				continue;
			}
			size_t colon = statement.find(':');
			if (colon == std::string::npos) {
				unsupported.push_back({line_no, "default is missing ':'"});
				continue;
			}
			const std::string rest = trim_copy(statement.substr(colon + 1));
			if (!stack.back().switch_seen_case && rest.empty()) {
				continue;
			}
			block.lua_lines.push_back("else");
			stack.back().switch_emitted_default = true;
			append_maybe_braced(rest);
		} else if (starts_with_word(lowered, "for")) {
			unsupported.push_back({line_no, "for loop conversion is not supported yet"});
		} else {
			append_statement(statement);
		}

		if (starts_with_word(statement, "setarray") && statement.find(';') == std::string::npos && !block.lua_lines.empty()) {
			append_setarray_args = true;
			append_setarray_index = block.lua_lines.size() - 1;
		}

		if (!comment.empty()) {
			block.lua_lines.push_back("-- " + trim_copy(comment));
		}
	}

	while (!stack.empty()) {
		const Frame frame = stack.back();
		stack.pop_back();
		if (frame.kind != FrameKind::InlineIf) {
			unsupported.push_back({block.line, "unclosed block automatically terminated"});
		}
		block.lua_lines.push_back("end");
	}

	if (multiline_setarray) {
		unsupported.push_back({setarray_line, "unterminated setarray statement"});
	}
	if (multiline_statement) {
		unsupported.push_back({statement_line, "unterminated multiline statement"});
	}
	if (multiline_operator) {
		unsupported.push_back({operator_line, "unterminated multiline operator expression"});
	}

	// Fix malformed switch conversion shape: "if false then" -> "else" -> "elseif ..."
	// produced by legacy default/case order patterns.
	for (size_t i = 0; i + 1 < block.lua_lines.size();) {
		const std::string current = trim_copy(block.lua_lines[i]);
		const std::string next = trim_copy(block.lua_lines[i + 1]);
		if (current == "else" && starts_with(next, "elseif __switch")) {
			size_t prev = i;
			while (prev > 0) {
				const std::string prev_line = trim_copy(block.lua_lines[prev - 1]);
				if (!prev_line.empty()) {
					if (prev_line == "if false then") {
						block.lua_lines.erase(block.lua_lines.begin() + static_cast<std::ptrdiff_t>(i));
						continue;
					}
					break;
				}
				--prev;
			}
		}
		++i;
	}

	std::vector<std::string> normalized_lines;
	normalized_lines.reserve(block.lua_lines.size());
	for (const std::string& raw_line : block.lua_lines) {
		const std::string trimmed = trim_copy(raw_line);
		if (trimmed == "{" || trimmed == "}") {
			continue;
		}
		if (starts_with(trimmed, "else(") && trimmed.size() >= 6 && trimmed.back() == ')') {
			const std::string inner = trim_copy(trimmed.substr(5, trimmed.size() - 6));
			normalized_lines.push_back("else");
			if (!inner.empty() && inner != "{") {
				const std::vector<std::string> inner_lines = convert_statement_to_lua_lines(inner);
				if (inner_lines.empty()) {
					normalized_lines.push_back(inner);
				} else {
					normalized_lines.insert(normalized_lines.end(), inner_lines.begin(), inner_lines.end());
				}
			}
			continue;
		}
		normalized_lines.push_back(raw_line);
	}

	// Resolve duplicated else/elseif on the same if-chain by inserting a
	// closing `end` before the later else-branch.
	struct LuaControlFrame {
		bool is_if = false;
		bool else_seen = false;
	};
	std::vector<LuaControlFrame> control_frames;
	std::vector<std::string> flow_fixed;
	flow_fixed.reserve(normalized_lines.size() + 8);

	for (const std::string& raw_line : normalized_lines) {
		const std::string line = trim_copy(raw_line);
		const std::string lower = to_lower(line);

		if (line == "end") {
			if (!control_frames.empty()) {
				control_frames.pop_back();
			}
			flow_fixed.push_back(raw_line);
			continue;
		}

		const bool is_else = (line == "else");
		const bool is_elseif = starts_with_word(lower, "elseif");
		if (is_else || is_elseif) {
			while (!control_frames.empty() && control_frames.back().is_if && control_frames.back().else_seen) {
				flow_fixed.push_back("end");
				control_frames.pop_back();
			}
			if (control_frames.empty() || !control_frames.back().is_if) {
				unsupported.push_back({block.line, "else/elseif without matching if in generated lua"});
			} else if (is_else) {
				control_frames.back().else_seen = true;
			}
			flow_fixed.push_back(raw_line);
			continue;
		}

		if (starts_with_word(lower, "if") && ends_with(lower, "then")) {
			control_frames.push_back({true, false});
		} else if (starts_with_word(lower, "for") && ends_with(lower, "do")) {
			control_frames.push_back({false, false});
		} else if (starts_with_word(lower, "while") && ends_with(lower, "do")) {
			control_frames.push_back({false, false});
		} else if (starts_with_word(lower, "repeat")) {
			control_frames.push_back({false, false});
		} else if (starts_with_word(lower, "function")) {
			control_frames.push_back({false, false});
		} else if (starts_with_word(lower, "local function")) {
			control_frames.push_back({false, false});
		}

		flow_fixed.push_back(raw_line);
	}

	block.lua_lines.swap(flow_fixed);
}

BodyConversion convert_entry_body(const LuaEntry& entry) {
	BodyConversion conversion;
	conversion.main.name = "__main";
	conversion.main.line = entry.body_line;
	std::vector<std::pair<size_t, std::string>> shared_local_functions;

	LuaCodeBlock* current = &conversion.main;
	const std::vector<std::string> lines = split_lines_preserve(entry.body);
	for (size_t i = 0; i < lines.size(); ++i) {
		std::string line = lines[i];
		if (!line.empty() && line.back() == '\r') {
			line.pop_back();
		}

		std::string func_name;
		std::string func_tail;
		if (parse_local_function_declaration(line, func_name, func_tail) && func_tail == "{") {
			int depth = 0;
			size_t j = i;
			for (; j < lines.size(); ++j) {
				std::string fn_line = lines[j];
				if (!fn_line.empty() && fn_line.back() == '\r') {
					fn_line.pop_back();
				}
				shared_local_functions.push_back({entry.body_line + j + 1, fn_line});
				for (char c : fn_line) {
					if (c == '{') {
						++depth;
					} else if (c == '}') {
						--depth;
					}
				}
				if (depth == 0 && j > i) {
					break;
				}
			}
			i = j;
			continue;
		}

		std::string label;
		std::string tail;
		if (parse_label_declaration(line, label, tail)) {
			LuaCodeBlock block;
			block.name = label;
			block.line = entry.body_line + i + 1;
			if (starts_with(to_lower(label), "on")) {
				conversion.events.push_back(std::move(block));
				current = &conversion.events.back();
			} else {
				conversion.labels.push_back(std::move(block));
				current = &conversion.labels.back();
			}
			if (!tail.empty()) {
				current->source_lines.push_back({entry.body_line + i + 1, tail});
			}
			continue;
		}

		current->source_lines.push_back({entry.body_line + i + 1, line});
	}

	if (!shared_local_functions.empty()) {
		conversion.main.source_lines.insert(conversion.main.source_lines.begin(), shared_local_functions.begin(), shared_local_functions.end());
		for (LuaCodeBlock& block : conversion.events) {
			block.source_lines.insert(block.source_lines.begin(), shared_local_functions.begin(), shared_local_functions.end());
		}
		for (LuaCodeBlock& block : conversion.labels) {
			block.source_lines.insert(block.source_lines.begin(), shared_local_functions.begin(), shared_local_functions.end());
		}
	}

	convert_block_to_lua(conversion.main, conversion.unsupported);
	for (LuaCodeBlock& block : conversion.events) {
		convert_block_to_lua(block, conversion.unsupported);
	}
	for (LuaCodeBlock& block : conversion.labels) {
		convert_block_to_lua(block, conversion.unsupported);
	}

	return conversion;
}

void emit_simple_entry(std::ostringstream& out, const LuaEntry& entry) {
	out << "\t\t{\n";
	out << "\t\t\tline = " << entry.line << ",\n";
	out << "\t\t\tw1 = "; emit_lua_string(out, entry.w1); out << ",\n";
	out << "\t\t\tw2 = "; emit_lua_string(out, entry.w2); out << ",\n";
	out << "\t\t\tw3 = "; emit_lua_string(out, entry.w3); out << ",\n";
	if (!entry.w4.empty()) {
		out << "\t\t\tw4 = "; emit_lua_string(out, entry.w4); out << ",\n";
	}
	out << "\t\t},\n";
}

void emit_function_body(std::ostringstream& out, const LuaCodeBlock& block, const std::string& indent, bool varargs = false) {
	out << "function(ctx";
	if (varargs) {
		out << ", ...";
	}
	out << ")\n";
	out << indent << "\tlocal _ENV = ctx:env()\n";
	if (block.lua_lines.empty()) {
		out << indent << "\treturn\n";
	} else {
		for (const std::string& line : block.lua_lines) {
			out << indent << "\t" << line << "\n";
		}
	}
	out << indent << "end";
}

void emit_script_entry(std::ostringstream& out, const LuaEntry& entry, const BodyConversion& conversion) {
	out << "\t\t{\n";
	out << "\t\t\tline = " << entry.line << ",\n";
	out << "\t\t\tw1 = "; emit_lua_string(out, entry.w1); out << ",\n";
	out << "\t\t\tw2 = "; emit_lua_string(out, entry.w2); out << ",\n";
	out << "\t\t\tw3 = "; emit_lua_string(out, entry.w3); out << ",\n";
	out << "\t\t\tw4 = "; emit_lua_string(out, entry.w4); out << ",\n";

	out << "\t\t\tmain = ";
	emit_function_body(out, conversion.main, "\t\t\t");
	out << ",\n";

	out << "\t\t\tevents = {\n";
	for (const LuaCodeBlock& block : conversion.events) {
		out << "\t\t\t\t[";
		emit_lua_string(out, block.name);
		out << "] = ";
		emit_function_body(out, block, "\t\t\t\t");
		out << ",\n";
	}
	out << "\t\t\t},\n";

	out << "\t\t\tlabels = {\n";
	for (const LuaCodeBlock& block : conversion.labels) {
		out << "\t\t\t\t[";
		emit_lua_string(out, block.name);
		out << "] = ";
		emit_function_body(out, block, "\t\t\t\t");
		out << ",\n";
	}
	out << "\t\t\t},\n";
	out << "\t\t},\n";
}

void emit_function_entry(std::ostringstream& out, const LuaEntry& entry, const BodyConversion& conversion) {
	out << "\t\t{\n";
	out << "\t\t\tline = " << entry.line << ",\n";
	out << "\t\t\tw1 = "; emit_lua_string(out, entry.w1); out << ",\n";
	out << "\t\t\tw2 = "; emit_lua_string(out, entry.w2); out << ",\n";
	out << "\t\t\tw3 = "; emit_lua_string(out, entry.w3); out << ",\n";
	if (!entry.w4.empty()) {
		out << "\t\t\tw4 = "; emit_lua_string(out, entry.w4); out << ",\n";
	}

	out << "\t\t\trun = ";
	emit_function_body(out, conversion.main, "\t\t\t", true);
	out << ",\n";

	out << "\t\t\tlabels = {\n";
	for (const LuaCodeBlock& block : conversion.labels) {
		out << "\t\t\t\t[";
		emit_lua_string(out, block.name);
		out << "] = ";
		emit_function_body(out, block, "\t\t\t\t");
		out << ",\n";
	}
	out << "\t\t\t},\n";
	out << "\t\t},\n";
}

RenderResult make_lua_output(const fs::path& source, const ParseResult& parsed) {
	std::vector<LuaEntry> warps;
	std::vector<LuaEntry> monsters;
	std::vector<LuaEntry> mapflags;
	std::vector<LuaEntry> shops;
	std::vector<LuaEntry> duplicates;
	std::vector<LuaEntry> scripts;
	std::vector<LuaEntry> functions;

	for (const LuaEntry& entry : parsed.entries) {
		switch (entry.kind) {
			case EntryKind::Warp: warps.push_back(entry); break;
			case EntryKind::Monster: monsters.push_back(entry); break;
			case EntryKind::Mapflag: mapflags.push_back(entry); break;
			case EntryKind::Shop: shops.push_back(entry); break;
			case EntryKind::Duplicate: duplicates.push_back(entry); break;
			case EntryKind::Script: scripts.push_back(entry); break;
			case EntryKind::Function: functions.push_back(entry); break;
		}
	}

	RenderResult rendered;
	std::ostringstream out;
	out << "-- Generated by script2lua.\n";
	out << "-- Source: " << to_portable_path(source) << "\n";
	out << "return {\n";

	out << "\twarps = {\n";
	for (const LuaEntry& entry : warps) {
		emit_simple_entry(out, entry);
	}
	out << "\t},\n";

	out << "\tmonsters = {\n";
	for (const LuaEntry& entry : monsters) {
		emit_simple_entry(out, entry);
	}
	out << "\t},\n";

	out << "\tmapflags = {\n";
	for (const LuaEntry& entry : mapflags) {
		emit_simple_entry(out, entry);
	}
	out << "\t},\n";

	out << "\tshops = {\n";
	for (const LuaEntry& entry : shops) {
		emit_simple_entry(out, entry);
	}
	out << "\t},\n";

	out << "\tduplicates = {\n";
	for (const LuaEntry& entry : duplicates) {
		emit_simple_entry(out, entry);
	}
	out << "\t},\n";

	out << "\tscripts = {\n";
	for (const LuaEntry& entry : scripts) {
		BodyConversion conversion = convert_entry_body(entry);
		rendered.unsupported.insert(rendered.unsupported.end(), conversion.unsupported.begin(), conversion.unsupported.end());
		emit_script_entry(out, entry, conversion);
	}
	out << "\t},\n";

	out << "\tfunctions = {\n";
	for (const LuaEntry& entry : functions) {
		BodyConversion conversion = convert_entry_body(entry);
		rendered.unsupported.insert(rendered.unsupported.end(), conversion.unsupported.begin(), conversion.unsupported.end());
		emit_function_entry(out, entry, conversion);
	}
	out << "\t},\n";

	out << "}\n";
	rendered.output = out.str();
	return rendered;
}

std::vector<fs::path> collect_input_files(const Options& options, std::string& error) {
	std::vector<fs::path> files;

	std::error_code ec;
	if (!fs::exists(options.input, ec) || ec) {
		error = "input path does not exist";
		return files;
	}

	if (fs::is_regular_file(options.input, ec)) {
		if (!ec) {
			files.push_back(options.input);
		}
		return files;
	}

	if (!fs::is_directory(options.input, ec) || ec) {
		error = "input path is neither a file nor a directory";
		return files;
	}

	if (options.recursive) {
		for (const auto& entry : fs::recursive_directory_iterator(options.input)) {
			if (!entry.is_regular_file()) {
				continue;
			}
			const fs::path& path = entry.path();
			const std::string portable = to_portable_path(path);
			if (path.extension() == ".txt" && !ends_with(portable, ".report.txt")) {
				files.push_back(entry.path());
			}
		}
	} else {
		for (const auto& entry : fs::directory_iterator(options.input)) {
			if (!entry.is_regular_file()) {
				continue;
			}
			const fs::path& path = entry.path();
			const std::string portable = to_portable_path(path);
			if (path.extension() == ".txt" && !ends_with(portable, ".report.txt")) {
				files.push_back(entry.path());
			}
		}
	}

	std::sort(files.begin(), files.end());
	return files;
}

fs::path resolve_output_path(const Options& options, const fs::path& source, bool input_is_file) {
	if (input_is_file) {
		if (options.output.has_extension() && !fs::is_directory(options.output)) {
			return options.output;
		}
		fs::path target = options.output / source.filename();
		target.replace_extension(".lua");
		return target;
	}

	const fs::path relative = fs::relative(source, options.input);
	fs::path target = options.output / relative;
	target.replace_extension(".lua");
	return target;
}

std::string now_string_utc() {
	using clock = std::chrono::system_clock;
	const auto now = clock::now();
	const std::time_t tt = clock::to_time_t(now);

#if defined(_WIN32)
	std::tm tm;
	gmtime_s(&tm, &tt);
#else
	std::tm tm;
	gmtime_r(&tt, &tm);
#endif

	char buf[64];
	std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S UTC", &tm);
	return buf;
}

bool write_report(const fs::path& path, const std::vector<FileResult>& results) {
	size_t converted = 0;
	size_t skipped = 0;
	size_t failed = 0;
	size_t unsupported = 0;

	for (const FileResult& item : results) {
		if (item.status == "converted") {
			++converted;
		} else if (item.status == "skipped") {
			++skipped;
		} else if (item.status == "failed") {
			++failed;
		}
		unsupported += item.unsupported.size();
	}

	std::ostringstream report;
	report << "script2lua migration report\n";
	report << "generated_at: " << now_string_utc() << "\n";
	report << "total_files: " << results.size() << "\n";
	report << "converted: " << converted << "\n";
	report << "skipped: " << skipped << "\n";
	report << "failed: " << failed << "\n";
	report << "unsupported_items: " << unsupported << "\n";
	report << "\n";

	for (const FileResult& item : results) {
		report << "[" << item.status << "] "
		       << to_portable_path(item.source)
		       << " -> "
		       << to_portable_path(item.target)
		       << " (lines=" << item.line_count << ")\n";
		if (!item.message.empty()) {
			report << "  note: " << item.message << "\n";
		}
		if (item.unsupported.empty()) {
			report << "  unsupported: none\n";
		} else {
			for (const UnsupportedItem& issue : item.unsupported) {
				report << "  unsupported: line=" << issue.line << " reason=" << issue.reason << "\n";
			}
		}
	}

	return write_file(path, report.str());
}

} // namespace

int main(int argc, char* argv[]) {
	Options options;
	std::string error;
	if (!parse_options(argc, argv, options, error)) {
		std::cerr << "script2lua: " << error << "\n";
		print_usage();
		return 2;
	}

	const bool input_is_file = fs::is_regular_file(options.input);
	std::vector<fs::path> files = collect_input_files(options, error);
	if (!error.empty()) {
		std::cerr << "script2lua: " << error << "\n";
		return 2;
	}

	if (files.empty()) {
		std::cerr << "script2lua: no input files found\n";
		return 1;
	}

	std::vector<FileResult> results;
	results.reserve(files.size());

	for (const fs::path& source : files) {
		FileResult result;
		result.source = source;
		result.target = resolve_output_path(options, source, input_is_file);

		if (!options.overwrite && fs::exists(result.target)) {
			result.status = "skipped";
			result.message = "output exists (use --overwrite to replace)";
			results.push_back(result);
			if (!options.quiet) {
				std::cout << "skip: " << to_portable_path(result.source) << " -> " << to_portable_path(result.target) << "\n";
			}
			continue;
		}

		const std::string input = read_file(source);
		if (input.empty() && fs::file_size(source) > 0) {
			result.status = "failed";
			result.message = "unable to read source file";
			results.push_back(result);
			std::cerr << "error: cannot read " << to_portable_path(source) << "\n";
			continue;
		}

		result.line_count = count_lines(input);
		ParseResult parsed = parse_script_text(input);
		result.unsupported = parsed.unsupported;

		RenderResult rendered = make_lua_output(source, parsed);
		result.unsupported.insert(result.unsupported.end(), rendered.unsupported.begin(), rendered.unsupported.end());

		if (!write_file(result.target, rendered.output)) {
			result.status = "failed";
			result.message = "unable to write target file";
			results.push_back(result);
			std::cerr << "error: cannot write " << to_portable_path(result.target) << "\n";
			continue;
		}

		result.status = "converted";
		if (!result.unsupported.empty()) {
			result.message = "converted with unsupported entries";
		}
		results.push_back(result);
		if (!options.quiet) {
			std::cout << "ok: " << to_portable_path(result.source) << " -> " << to_portable_path(result.target) << "\n";
		}
	}

	fs::path report_path = options.report;
	if (report_path.empty()) {
		if (input_is_file) {
			report_path = resolve_output_path(options, files.front(), true);
			report_path += ".report.txt";
		} else {
			report_path = options.output / "script2lua_report.txt";
		}
	}

	if (!write_report(report_path, results)) {
		std::cerr << "script2lua: failed to write report to " << to_portable_path(report_path) << "\n";
		return 1;
	}

	size_t failed = 0;
	size_t unsupported = 0;
	for (const FileResult& item : results) {
		if (item.status == "failed") {
			++failed;
		}
		unsupported += item.unsupported.size();
	}

	if (!options.quiet) {
		std::cout << "report: " << to_portable_path(report_path) << "\n";
	}

	if (failed > 0) {
		return 1;
	}

	if (options.strict && unsupported > 0) {
		return 1;
	}

	return 0;
}
