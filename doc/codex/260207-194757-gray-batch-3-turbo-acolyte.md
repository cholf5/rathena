# 灰度第三批：turbo_track + acolyte_warp

时间：见文件名
范围：`src/tool/script2lua.cpp`、`npc/other/turbo_track.lua`、`npc/other/acolyte_warp.lua`

## 目标
- 修复 `npc/other` 批次中的高复杂脚本转换问题。
- 保证输出为纯 Lua 逻辑，且可通过 `luac` 语法检查。

## 本次代码修改
1. `script2lua` 链式赋值修正：`a=b=v` 生成 `b=v` 与 `a=v`。
2. `script2lua` 递归输入过滤：忽略 `*.report.txt`，阻断 `.report.lua` 连锁转换。
3. `script2lua` 增强函数声明兼容：支持 `function Name;` 前置声明（Lua 侧按 no-op 处理），避免误报 unsupported。
4. `script2lua` 增强兜底：对无法完整解析但明显是前置声明的 `function ...;` 行不再记 unsupported。

## 灰度结果
- `npc/other/turbo_track.txt -> npc/other/turbo_track.lua`
  - `unsupported_items: 0`
  - `luac -p` 通过
- `npc/other/acolyte_warp.txt -> npc/other/acolyte_warp.lua`
  - `unsupported_items: 0`
  - `luac -p` 通过

## 运行时验证说明
- 已按建议尝试使用 `mariadbd-safe --datadir="$(brew --prefix)/var/mysql"` 启动数据库。
- MariaDB 错误日志显示服务已就绪（`socket: /tmp/mysql.sock, port: 3306`）。
- 但当前执行环境中 `map-server --run-once` 仍报 `Can't connect to server on '127.0.0.1' (36)`，本批运行时验证受环境连接隔离影响。

## 后续建议
- 继续按灰度策略推进 `npc/other` 下一批高复杂脚本（如 `auction/books/bulletin_boards`）。
- 在可直连 DB 的同一执行域内补跑 `map-server --run-once`，做行为级回归。
