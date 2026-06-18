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
- P1：A（左）、D（右）、S（下蹲）、W（跳跃）、J（轻拳）、K（轻腿）、U（重拳）、L（重腿）、I（超重击）
- P2：Left（左）、Right（右）、Down（下蹲）、Up（跳跃）、小键盘1（轻拳）、小键盘2（轻腿）、小键盘3（重拳）、小键盘0（重腿）、小键盘5（超重击）
- 16ms 定时器驱动移动（约 60fps）

**移动系统**
- 移动步长 0.008，身体碰撞距离 0.15 单位
- 移动时自动切换 forward/backward 动画
- 镜头约束：两角色保持在 0.95 单位内

**跳跃系统**
- 直跳（站立按 W/Up）：playJump()
- 斜跳（行走中按 W/Up）：playDiagonalJump(forward/backward)
- 跳跃期间移动定时器停止
- 斜跳可自由飞越对手，落地后身体碰撞生效

**攻击系统**
- 五段攻击：轻拳、轻腿、重拳、重腿、超重击
- 攻击判定框（hitbox）+ 受击判定框（hurtbox）碰撞检测
- 攻击有效帧范围配置（attackFrames）
- 同时攻击时双方都能命中
- 攻击命中后播放 3 帧特效动画（1.png→2.png→3.png），定位于防御者脖子处

**受击系统**
- 受击动画：Yagami 3 段（hurt1/hurt2/hurt3），Orochi 1 段
- 击退效果（knockback）：渐进式，分散到受击动画每帧
- 击退距离：轻拳/轻腿 30，重拳 80，重腿 120

**下蹲系统**
- P1 按 S / P2 按 Down 进入下蹲
- Orochi：显示 Crouch.png（CrouchAttack 帧0 resize），冻结不循环
- Yagami：显示 Crouch.png（原始 384×512 resize 251×335），循环播放
- 蹲防机制：下蹲时任何攻击不触发受击动画、无击退、无伤害
- 下蹲时禁止移动、跳跃、攻击

**朝向系统**
- updateFacing() 根据双方 posXRatio 自动翻转
- 跳跃中静默更新（不触发信号），落地后正式刷新

**渲染层**
- 背景：AnimatedImage (Monaco.gif) + 镜头视差
- 角色：Item + Image，精灵表视口裁剪 + Scale 翻转
- HUD：头像 + 名字 + 血条 + 能量条 + 倒计时 (60s)
- 调试判定框可视化（showDebugHitbox，默认关闭）

### 2.4 CharacterModel — 角色动画状态机

15 种状态：Waiting → Opening → Stand ↔ Forward/Backward，Stand ↕ Jump/DiagonalJump，Stand → LightPunch/LightKick/HeavyPunch/HeavyKick/HeavyStrike → Stand，Stand → Crouch ↔ CrouchAttack，任意状态 → Hurt → Stand

关键 Q_PROPERTY：currentFrame、sourcePath、frameWidth/Height、totalFrames、posXRatio、positionY、facingLeft、visualScale、animOffsetX、state、attacking、attackActive、hitboxCenterX/Y、hitboxRadius、hurtboxCenterX/Y

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

**碰撞检测**（checkCollision）：
- 攻击框：hitboxX/Y + hitboxRadius
- 受击框：hurtboxX/Y + hurtboxW/H
- 双方同时命中判定

**渐进击退**（applyKnockback + onTick）：
- 击退距离分散到受击动画每帧
- 每帧移动 step = remaining / frames

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

从 characters.json 解析 AnimParams 结构体：path、cols、fw/fh、interval、loop、divFrame、jumpHeight、jumpDistance、visualScale、feetBottom、feetMargin、offsetX、attackPointX/Y、attackRadius、attackFrames、knockbackDistance。

## 3. 操作说明

| 操作 | P1 | P2 |
|------|-----|-----|
| 左移 | A | Left |
| 右移 | D | Right |
| 下蹲 | S | Down |
| 直跳 | W（站立时） | Up（站立时） |
| 前跳 | W（向前行走中） | Up（向前行走中） |
| 后跳 | W（向后行走中） | Up（向后行走中） |
| 轻拳 | J | 小键盘1 |
| 轻腿 | K | 小键盘2 |
| 重拳 | U | 小键盘3 |
| 重腿 | L | 小键盘0 |
| 超重击 | I | 小键盘5 |

## 4. 资源清单

**已接入角色**

| 角色 | CID | 精灵表 |
|------|-----|--------|
| 大蛇 | Orochi | Stand/Forward/Backward/Jump/DiagonalJump/opening/LightPunch/LightKick/HeavyPunch/HeavyKick/HeavyStrike/Hurt/Crouch/CrouchAttack |
| 八神庵 | Yagami | Stand/Forward/Backward/Jump/DiagonalJump/opening/LightPunch/LightKick/HeavyPunch/HeavyKick/HeavyStrike/Hurt1_std/Hurt2_std/Hurt3_std/Crouch |

**未接入角色**（有头像/立绘，缺精灵表）：草薙京、库拉、不知火舞、K

**战斗背景**（5个，已全部接入）：Monaco（当前使用）、AmusementPark、Bali、Gyeongbokgung、OrochiShermie

**已有未使用的精灵表**：JumpAttack、StandBlock、AirBlock、Dodge、DashForward、DashBackward、PowerUp、SuperMove、Knockdown、Yagami/CrouchAttack

**攻击特效资源**：Attack/1.png (15×21)、Attack/2.png (27×23)、Attack/3.png (32×31)，用于轻拳命中特效

## 5. 已知限制

1. 仅 2 个角色可选（Orochi/Yagami），其余 4 个角色选入后会报错
2. 无伤害数值系统（攻击只触发动画，不扣血）
3. 无胜负判定、无回合系统
4. Online Two-Player 未实现
5. 战斗背景硬编码为 Monaco.gif
6. 蹲姿攻击（CrouchAttack）未实现（蹲姿中按轻拳仅蹲防，不触发蹲攻击）
7. 攻击特效仅一套通用帧图，不同攻击类型未使用不同特效
