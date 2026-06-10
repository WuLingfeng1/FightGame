# FightGame
仿写拳皇97


代码结构
FightGame/
├── Main.qml              ← 已完成（开始界面）
├── SelectScreen.qml      ← 已完成（选人界面）
├── FightScreen.qml        ← 第1步：纯战斗 UI（血条/计时器/站位）
├── FighterModel.h/.cpp    ← 第2步：角色数据（血量/位置/动画）
├── GameEngine.h/.cpp      ← 第3步：游戏循环 + 碰撞 + 输入路由
├── BattleImageProvider    ← 第4步：逐帧喂图给 QML
└── FightScreen.qml        ← 第5步：接入真实角色动画和交互
