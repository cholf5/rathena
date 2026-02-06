# Lua 运行时错误诊断增强

## 目的
当前 `errors.txt` 中大量报错形如 `table: 0x...`，缺少脚本行号，无法高效定位。

## 改动
- 文件：`src/map/lua_engine.cpp`
- 新增：
  - `lua_value_debug_string(...)`：将 Lua 错误对象统一序列化为可读字符串，并附带类型标注。
  - `lua_traceback_string(...)`：基于 `luaL_traceback` 输出调用栈。
- 调整 `lua_resume_instance_internal(...)` 的错误分支：
  - 统一使用上述函数输出 `runtime error`，并保留错误对象弹栈。

## 预期效果
- `errors.txt` 将从 `table: 0x...` 变为包含 traceback 的可定位错误信息（包含 Lua 文件/行）。
- 便于下一轮集中修补缺失 builtin / 行为不兼容点。

## 验证
- 本地已重新编译 `map-server` 通过。
- 未执行 `./map-server`（按约定由你执行并提供 `errors.txt`）。
