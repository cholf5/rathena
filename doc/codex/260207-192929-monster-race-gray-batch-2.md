# Monster Race 灰度第二批（script2lua）

时间：自动生成（见文件名）
范围：`src/tool/script2lua.cpp`、`npc/other/monster_race.lua`

## 本次修改
1. 修复链式赋值转换：`a = b = v` 现在会生成 `b = v` 与 `a = v`，不再生成 `a = b`。
2. 修复递归输入污染：目录扫描时忽略 `*.report.txt`，避免 `xxx.lua.report.lua` 连锁产物继续被当作输入。
3. 重新转换 `npc/other/monster_race.txt -> npc/other/monster_race.lua`。

## 验证
1. 构建：`make -C src/tool script2lua -j4` 通过。
2. 单文件转换：`./script2lua npc/other/monster_race.txt npc/other/monster_race.lua --overwrite --report /tmp/monster_race.report.txt` 通过，unsupported=0。
3. Lua 语法：`luac -p npc/other/monster_race.lua` 通过。
4. 目录过滤回归：`--recursive` 模式下 `*.report.txt` 不再进入输入集合。

## 说明
- `map-server --run-once` 在当前执行环境仍报 `127.0.0.1:3306` 连接失败，因此本批以“离线可验证项”为主。
