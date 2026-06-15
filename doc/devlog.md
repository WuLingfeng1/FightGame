# 开发日志

## 2026-06-15

### 攻击动作系统完善

#### 新增功能
- **Yagami 四段攻击**：将 StandAttack.png 拆分为独立精灵表，实现轻拳(J)、轻腿(K)、重拳(U)、重腿(L)
- **Orochi 三段攻击**：为 LightKick、HeavyPunch、HeavyKick 精灵表缩放至 251×335 并配置参数
- **P2 按键绑定**：小键盘 1/2/3/0 对应轻拳/轻腿/重拳/重腿
- **朝向翻转补偿**：animOffsetX 根据 facingLeft 自动取反，消除换位时的 X 轴跳变

#### Bug 修复
- **轻拳脚尖上移**：setPosY() 添加 visualScale 补偿公式 `positionY += (fh - fb) * (vs - 1)`
- **轻拳水平偏移**：为 Yagami/Orochi 的 lightPunch 添加 offsetX 参数
- **Orochi 轻拳精灵尺寸错误**：LightPunch.png 从 384×512 缩放至 251×335
- **动画衔接卡顿**：裁剪 LightKick(13→10帧) 和 HeavyPunch(16→13帧) 的恢复帧

#### 代码改动
- `characterconfig.h/cpp`：AnimParams 新增 offsetX 字段，CharacterConfig 新增 lightKick/heavyPunch/heavyKick
- `charactermodel.h/cpp`：State 枚举新增 LightKick/HeavyPunch/HeavyKick，新增对应 play 函数
- `FightScreen.qml`：P1/P2 按键绑定扩展，animOffsetX 朝向补偿
- `config/characters.json`：两个角色的攻击参数配置

#### 精灵表变更
- Yagami：StandAttack.png → 4 个独立文件 (StandAttack_LightPunch/LightKick/HeavyPunch/HeavyKick.png)
- Orochi：LightKick/HeavyPunch/HeavyKick.png 缩放至 251×335
