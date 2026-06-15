# FightGame 已完成部分开发文档

## 1. 系统架构

Main.qml (主菜单) → SelectScreen.qml (选人界面) → FightScreen.qml (战斗界面)

FightScreen 持有 FightDirector 实例，FightDirector 管理两个 CharacterModel，CharacterConfig 从 characters.json 加载配置。

## 2. 模块详解

### 2.1 Main.qml — 主菜单

- ApplicationWindow，900x640，StackView 路由
- 背景图 + 标题 "FIGHT GAME"
- 三个按钮：Local Two-Player（跳转选人）、Online Two-Player（未实现）、EXIT

### 2.2 SelectScreen.qml — 角色选择界面

- 选人状态机：currentTurn = 1（P1选）→ 2（P2选）→ 0（锁定）
- 上半：P1 portrait + VS + P2 portrait
- 下半：6 个头像网格 + CONFIRM / FIGHT / BACK
- 边框颜色：锁定=金边、P1预览=暗红、P2预览=深蓝、默认=灰
- 可选角色：草薙京、库拉、大蛇、八神庵、不知火舞、K

### 2.3 FightScreen.qml — 战斗主界面

**输入系统**
- P1：A（左）、D（右）、W（跳跃）、J（轻拳）、K（轻腿）、U（重拳）、L（重腿）
- P2：Left（左）、Right（右）、Up（跳跃）、小键盘1（轻拳）、小键盘2（轻腿）、小键盘3（重拳）、小键盘0（重腿）
- 16ms 定时器驱动移动（约 60fps）

**移动系统**
- 移动步长 0.008，边界约束：不能超过对手 +0.95
- 移动时自动切换 forward/backward 动画

**跳跃系统**
- 直跳（站立按 W/Up）：playJump()
- 斜跳（行走中按 W/Up）：playDiagonalJump(forward/backward)
- 跳跃期间移动定时器停止

**朝向系统**
- updateFacing() 根据双方 posXRatio 自动翻转
- 跳跃中静默更新（不触发信号），落地后正式刷新

**渲染层**
- 背景：AnimatedImage (Monaco.gif) + 镜头视差
- 角色：Item + Image，精灵表视口裁剪 + Scale 翻转
- HUD：头像 + 名字 + 血条 + 能量条 + 倒计时 (60s)

### 2.4 CharacterModel — 角色动画状态机

10 种状态：Waiting → Opening → Stand ↔ Forward/Backward，Stand ↕ Jump/DiagonalJump，Stand → LightPunch/LightKick/HeavyPunch/HeavyKick → Stand

关键 Q_PROPERTY：currentFrame、sourcePath、frameWidth/Height、totalFrames、posXRatio、positionY、facingLeft、visualScale、animOffsetX

**跳跃抛物线**（setPosY）：
```
t = localFrame / (totalFrames - 1)
arcOffset = -jumpHeight * 4 * t * (1 - t)
posY = groundY + arcOffset
```

**斜跳水平位移**（onTick）：
```
xSign = initialFacingLeft ? -1 : 1
xDelta = xSign * (isForward ? 1 : -1)
posX = startXRatio + xDelta * jumpDistance * t
```

### 2.5 FightDirector — 战斗导演

战斗流程：Opening_P1 → Opening_P2 → Fighting

**镜头算法**（updateCamera）：
```
target = (p1 + p2) / 2 - 0.5
target = clamp(target, maxPos - 0.95, minPos - 0.05)
target = clamp(target, 0, stageWidth - 1)
diff = target - currentOffset
factor = |diff| < 0.015 ? 0.2 : 0.7
offset += diff * factor
```

### 2.6 CharacterConfig — 配置加载器

从 characters.json 解析 AnimParams 结构体：path、cols、fw/fh、interval、loop、divFrame、jumpHeight、jumpDistance、visualScale、feetBottom、feetMargin、offsetX。

## 3. 操作说明

| 操作 | P1 | P2 |
|------|-----|-----|
| 左移 | A | Left |
| 右移 | D | Right |
| 直跳 | W（站立时） | Up（站立时） |
| 前跳 | W（向前行走中） | Up（向前行走中） |
| 后跳 | W（向后行走中） | Up（向后行走中） |
| 轻拳 | J | 小键盘1 |
| 轻腿 | K | 小键盘2 |
| 重拳 | U | 小键盘3 |
| 重腿 | L | 小键盘0 |

## 4. 资源清单

**已接入角色**

| 角色 | CID | 精灵表 |
|------|-----|--------|
| 大蛇 | Orochi | Stand/Forward/Backward/Jump/DiagonalJump/opening/LightPunch/LightKick/HeavyPunch/HeavyKick |
| 八神庵 | Yagami | Stand/Forward/Backward/Jump/DiagonalJump/opening/LightPunch/LightKick/HeavyPunch/HeavyKick |

**未接入角色**（有头像/立绘，缺精灵表）：草薙京、库拉、不知火舞、K

**战斗背景**（5个，已全部接入）：Monaco（当前使用）、AmusementPark、Bali、Gyeongbokgung、OrochiShermie

**已有未使用的精灵表**：CrouchAttack、JumpAttack、StandBlock、AirBlock、Dodge、DashForward、DashBackward、PowerUp、SuperMove、Knockdown、Hurt1~5

## 5. 已知限制

1. 仅 2 个角色可选（Orochi/Yagami），其余 4 个角色选入后会报错
2. 无碰撞检测、无伤害系统
3. 无胜负判定、无回合系统
4. Online Two-Player 未实现
5. 战斗背景硬编码为 Monaco.gif
