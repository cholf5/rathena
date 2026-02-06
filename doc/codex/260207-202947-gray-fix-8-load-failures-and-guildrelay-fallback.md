# 灰度修复：8 个 Lua 加载失败文件（本轮）

## 本轮处理
- 读取最新 `errors.txt`，定位 8 个 `lua_engine_load_file: failed to load` 文件。
- 在 `src/tool/script2lua.cpp` 增加/修正语法转换规则：
  - 支持前置自增/自减：`++var` / `--var`。
  - 将参数 `-` 转换为 `nil`（用于 `menu(...,-)` 等）。
  - 当 `switch/case/default` 泄漏到普通语句转换路径时，输出注释而不是非法 Lua 语法。
  - 重写三元表达式转换：先处理括号内层，再处理外层，降低 `?:` 在拼接场景下的错误改写。
  - 过滤残留的独立 `{` / `}` 行。

## 文件结果
- 已通过离线 `loadfile` 语法检查（7 个）：
  - `npc/guild/agit_main.lua`
  - `npc/guild2/agit_main_se.lua`
  - `npc/quests/ninja_quests.lua`
  - `npc/quests/quests_13_2.lua`
  - `npc/quests/quests_moscovia.lua`
  - `npc/quests/quests_nameless.lua`
  - `npc/quests/the_sign_quest.lua`
- `npc/quests/guildrelay.lua` 仍存在复杂块结构转换问题（多段重复 if/else 嵌套），暂未完全稳定。

## 灰度回退
- 按灰度策略，先回退 `guildrelay` 的加载入口到 DSL，避免阻塞整体启动：
  - `npc/scripts_athena.conf`:
    - `npc: npc/quests/guildrelay.lua` -> `npc: npc/quests/guildrelay.txt`

## 备注
- 本轮未执行 `./map-server`（按约定由你执行并提供 `errors.txt`）。
