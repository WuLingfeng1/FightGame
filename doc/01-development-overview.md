# FightGame 总体开发思路

## 1. 项目概述

**FightGame** 是一款仿写拳皇97（KOF '97）的本地双人格斗游戏。

- **技术栈**：Qt 6.11 / QML + C++23
- **构建系统**：CMake 4.2.3 + Ninja
- **窗口尺寸**：900 x 640
- **许可证**：AGPLv3
- **作者**：Linfeng Wu

## 2. 技术架构

采用 **QML 渲染层 + C++ 逻辑层** 的双层架构，通过 Qt 的 Q_PROPERTY + 信号槽机制桥接。

**QML 渲染层**（Main.qml / SelectScreen.qml / FightScreen.qml）
- 精灵表视口裁剪 + Scale 翻转
- HUD（血条/能量条/倒计时）
- 键盘输入捕获
- 背景视差滚动

↕ Q_PROPERTY + 信号

**C++ 逻辑层**（CharacterModel / FightDirector / CharacterConfig）
- 动画状态机（7状态循环）
- 战斗流程管理（开场→战斗）
- 跳跃抛物线计算
- 镜头跟随算法
- JSON 配置解析

### 核心设计模式

| 模式 | 应用位置 | 说明 |
|------|---------|------|
| 状态机 | CharacterModel::State | 7种状态驱动动画流转 |
| 观察者 | Qt 信号槽 | C++ 属性变化 → QML 绑定更新 → 渲染 |
| 导演模式 | FightDirector | 统一管理战斗流程、开场顺序、镜头系统 |
| 数据驱动 | characters.json | 角色动画参数全部 JSON 化，可配置 |

## 3. 分阶段开发路线

### 已完成阶段

| 阶段 | 版本 | 内容 |
|------|------|------|
| 1 | v0.1.0 | 主菜单 + 选人界面 + 战斗界面框架 |
| 2 | v0.1.1 | 界面串联 + 开场动画 + 窗口适配 |
| 3 | v0.1.2 ~ v0.1.3 | 角色动画系统（精灵表状态机 + 朝向系统） |
| 4 | v0.1.4 | 镜头跟随机制 |
| 5 | v0.1.5 ~ v0.1.6 | 移动/跳跃/斜跳 + Bug修复 |

### 待完成阶段

| 阶段 | 内容 | 依赖 |
|------|------|------|
| 6 | 攻击系统（轻拳/重拳/轻踢/重踢/必杀技） | 阶段5 |
| 7 | 碰撞检测 + 判定框（hitbox/hurtbox） | 阶段6 |
| 8 | 伤害计算 + 血量系统 | 阶段7 |
| 9 | 能量系统 + 超必杀 | 阶段8 |
| 10 | 防御/闪避/受身系统 | 阶段7 |
| 11 | 胜负判定 + 回合系统 | 阶段8 |
| 12 | 在线对战（网络同步） | 阶段11 |

## 4. 数据驱动架构

所有角色动画参数通过 `config/characters.json` 配置：

```json
{
    "角色ID": {
        "facingLeft": true/false,
        "posX": 初始位置比例,
        "opening": { "cols", "fw", "fh", "interval", "path" },
        "stand":   { "cols", "fw", "fh", "interval", "path", "feetBottom", "feetMargin" },
        "forward": { "cols", "fw", "fh", "interval", "path", "loop" },
        "backward":{ ... },
        "jump":    { ..., "jumpHeight", "visualScale" },
        "diagonalJump": { ..., "jumpHeight", "jumpDistance", "divFrame", "visualScale" }
    }
}
```

新角色接入只需：准备精灵表图片 → 添加 JSON 配置 → 在 SelectScreen.qml 中注册角色条目。

## 5. 扩展规划

### 待接入角色

已有头像和立绘但缺少精灵表和 JSON 配置：

| 角色 | CID | 状态 |
|------|-----|------|
| 草薙京 | Kusanagi | 待制作精灵表 |
| 库拉 | Kula | 待制作精灵表 |
| 不知火舞 | Shiranui | 待制作精灵表 |
| K | Kdash | 待制作精灵表 |

### 已有角色精灵表（未全部接入）

Orochi 和 Yagami 的精灵表目录中已有大量未使用的动作资源：

- 攻击：LightPunch、HeavyPunch、LightKick、HeavyKick、CrouchAttack、JumpAttack
- 防御：StandBlock、AirBlock、Dodge
- 特殊：DashForward、DashBackward、PowerUp、SuperMove、Knockdown
- 受击：Hurt1~5

这些资源可直接用于后续攻击/防御/受身系统的开发。

## 6. 辅助工具

`tools/` 目录下包含 4 个 Python 精灵表处理脚本：

| 脚本 | 功能 |
|------|------|
| refine_jump.py | 跳跃精灵表尺寸标准化（对齐到 251x335） |
| refine_all_jumps.py | 批量处理所有跳跃精灵表（内容感知缩放） |
| resize_forward.py | 前进动画尺寸调整 |
| resize_backward.py | 后退动画尺寸调整 |
