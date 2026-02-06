# Lua runtime error 栈处理修复

## 问题
运行时报错只显示 `<invalid stack index>` + `stack traceback:`，没有有效上下文。

## 根因
在 `lua_resume_instance_internal` 里，`lua_resume` 返回后无论状态都会先 `lua_pop(result_count)`，可能在错误分支提前把错误对象弹栈，导致后续读取错误对象时栈为空。

## 修复
- 文件：`src/map/lua_engine.cpp`
- 变更：
  - 仅在 `LUA_OK` / `LUA_YIELD` 分支弹出 `result_count`。
  - 错误分支增加空栈保护：若 `lua_gettop(co) <= 0`，直接输出 `lua_resume status=<code> with empty stack`。
  - `lua_value_debug_string` 的空栈提示从 `<invalid stack index>` 改为 `<empty error stack>`。

## 预期
下一次 `errors.txt` 会出现更可定位的 runtime 错误（错误对象或至少状态码），便于继续补 builtin 兼容。

## 验证
- 已重新编译 `map-server` 通过。
- 未执行 `./map-server`（按约定由用户执行）。
