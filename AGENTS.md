# FightGame - Agent 指令

## 项目概述
Qt6/QML 格斗游戏（拳皇97风格）。C++ 后端驱动动画状态机，QML 负责渲染和输入。

## 构建与运行
```bash
# 配置（首次或修改 CMakeLists.txt 后）
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# 构建
cmake --build build

# 运行
./build/appFightGame
```

依赖：Qt 6.9+、CMake 4.2+、Ninja、支持模块的 C++23 编译器。

## 架构

### 入口
- `main.cpp` → 通过 `engine.loadFromModule("FightGame", "Main")` 加载 `Main.qml`

### QML 界面（StackView 导航）
- `Main.qml` → 开始菜单，跳转到 SelectScreen
- `SelectScreen.qml` → 选人界面（P1/P2），跳转到 FightScreen
- `FightScreen.qml` → 战斗渲染，接收角色 ID 作为属性

### C++ 后端（QML_ELEMENT 类型）
- `FightDirector` → 战斗阶段控制器（Opening_P1 → Opening_P2 → Fighting），拥有两个 CharacterModel，管理镜头
- `CharacterModel` → 精灵表动画状态机（Waiting → Opening → Stand ↔ Forward/Backward/Jump/Attack）
- `CharacterConfig` / `CharacterConfigLoader` → 解析 `config/characters.json`

### 角色数据
`config/characters.json` 定义每个角色的动画参数：
- 精灵表路径（`qrc:/images/...`）、帧尺寸（`fw`/`fh`）、帧数（`cols`）、间隔（毫秒）
- 位置偏移（`feetBottom`、`feetMargin`）、跳跃物理（`jumpHeight`、`jumpDistance`）
- 每个动画是水平精灵条，帧从左到右推进

### 按键映射
- P1：A/D（移动）、W（跳跃）、J（轻拳）
- P2：方向键（移动）、Up（跳跃）、小键盘 1（轻拳）

## 精灵工具（`tools/`）
Python 脚本（需要 `Pillow`）用于缩放精灵表以匹配站立帧比例：
```bash
pip install Pillow
python tools/refine_all_jumps.py   # 批量：所有跳跃/斜跳缩放
python tools/resize_forward.py     # 单个动画类型
```
脚本从 `config/characters.json` 读取帧数。运行前务必备份原图——脚本会原地覆盖 PNG。

## 测试
无测试套件。验证方式：构建、运行、目视检查动画。

## QML LSP
`CMakeLists.txt` 设置了 `QT_QML_GENERATE_QMLLS_INI ON`；生成的 `.qmlls.ini` 启用 QML 语言服务器。该文件已被 gitignore。

## 关键约定
- 所有图片通过 Qt 资源系统嵌入（`qrc:/`）；新图片必须在 `CMakeLists.txt` 的 `qt_add_resources` 中添加
- 角色动画在 JSON 中定义；**但** `SelectScreen.qml` 有硬编码角色列表（20-27 行）——新角色也需在那里添加
- QML 通过 `import FightGame` 访问通过 `QML_ELEMENT` 注册的 C++ 类型
- 镜头跟随两名格斗者中点；舞台宽度为 3.0 归一化单位
- `posXRatio` 是 0.0–1.0 归一化水平位置；实际像素 X = `posXRatio * root.width`

## 常见陷阱
- 精灵表在同一动画内必须保持一致的帧尺寸，否则会导致渲染异常
- `feetBottom`/`feetMargin` 是角色特定的，以现有角色配置为起点复制
- 跳跃动画使用 `divFrame` 将精灵表分为前跳/后跳两段（斜跳用）
- 构建使用 C++ 模块（`CMAKE_CXX_MODULE_STD ON`），确保编译器支持 `import std`
- `FightScreen.qml` 通过 `Qt.createComponent("qrc:/qt/qml/FightGame/FightScreen.qml")` 创建组件——路径必须与 QML 模块 URI 匹配
