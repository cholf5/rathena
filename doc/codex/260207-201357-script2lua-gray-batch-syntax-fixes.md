# script2lua 灰度批次：语法修复与重转换

## 本次改动
- 在 `src/tool/script2lua.cpp` 修复 DSL `end;` 转换规则：由 `return` 改为 `do return end`，避免 Lua 中“return 不是块尾语句”导致的语法错误。
- 增强 `else if (...)` 的条件尾部拼接：支持 `) || (...) {` / `) && (...) {` 形态，正确并入同一个 `elseif` 条件表达式。
- 增强 `if/else if/else` 的行内块头处理：支持 `if (...) { stmt...` 风格，避免把 `{` 当普通语句输出。

## 灰度重转文件
- 基于最新 `errors.txt` 中 `failed to load` 列表，批量重转了 guild/jobs/quests/warps 下的 30+ 个 `.txt -> .lua` 文件（覆盖写入）。

## 验证结果
- 单文件验证：`npc/other/monster_race.lua` 可通过 `loadfile` 语法检查。
- 批次语法验证后，仍有若干脚本存在语法问题，典型包括：
  - `npc/guild/agit_main.lua`
  - `npc/guild2/agit_main_se.lua`
  - `npc/quests/guildrelay.lua`
  - `npc/quests/ninja_quests.lua`
  - `npc/quests/quests_13_2.lua`
  - `npc/quests/quests_moscovia.lua`
  - `npc/quests/quests_nameless.lua`
  - `npc/quests/the_sign_quest.lua`

## 备注
- 运行 `./map-server --run-once` 目前被数据库 TCP 连接问题阻断（`127.0.0.1:3306`），因此本轮以离线 Lua 语法检查为主。
