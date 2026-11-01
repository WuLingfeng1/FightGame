# Fightgame

基于 C++ 和 Qt Quick 的双人格斗游戏，包含本地对战、联机房间，以及草薙京、大蛇和八神的动作与技能。

## 构建

依赖 CMake 3.22 或更高版本、支持 C++23 的编译器，以及 Qt 6.9 或更高版本。Qt 组件包括 Quick、Network、Widgets、Multimedia；启用测试时还需要 Test。

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Qt 未在默认搜索路径时，可通过 `CMAKE_PREFIX_PATH` 指定本机 Qt 的安装位置。

## 回归测试

```sh
cmake -S . -B build -DFIGHTGAME_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## 目录

| 路径 | 用途 |
| --- | --- |
| `*.cpp`、`*.h` | 角色模型、战斗流程、联机和音频逻辑 |
| `*.qml`、`FighterGeometry.js` | 界面、角色显示和技能效果 |
| `config/` | 动作参数和默认键位 |
| `images/character/` | 原有角色帧图和新增图集 |
| `images/character/<角色>/Imported/` | 按动作命名的导入素材，避免覆盖原有同名资源 |
| `resources/` | 音乐和角色语音 |
| `tests/` | 动作、碰撞、技能与衔接测试 |
| `doc/` | 项目文档 |

草薙京保持原有素材配置。备用导入素材不会自动启用，实际动作由角色配置和资源列表决定。
