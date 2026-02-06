# Lua 引擎修复：补齐 compare 内置

时间：见文件名
范围：`src/map/lua_engine.cpp`

## 问题
- `errors.txt` 中高频报错：`attempt to call a number value (global 'compare')`
- 根因：Lua 环境中未注册 `compare` 内置，命中变量默认值回退（数值 0），调用时触发 runtime error。

## 修改
1. 新增 `lua_builtin_compare(lua_State* L)`
   - 语义与 DSL `compare` 对齐：大小写不敏感子串匹配。
   - 返回 `1/0`（整型）以兼容原脚本布尔分支。
2. 在 `lua_register_builtins()` 中注册 `compare`。

## 验证
- `make -C src/map -j4 map-server` 编译通过。

## 后续
- 让用户重跑 `./map-server --run-once 2> errors.txt`，观察该类错误是否显著下降，再继续处理下一类高频错误。
