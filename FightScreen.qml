// Module
// File: FightScreen.qml   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-10 14:56:24
// Description:
//     对战主界面, 纯渲染层, 动画逻辑由 C++ CharacterModel / FightDirector 驱动
// Change Log:
//     [v0.1.1]     2026-06-11 22:43:30   Added character intro animations
//     [v0.1.2]     2026-06-12 13:54:47   分离渲染与功能, 增加forward动作, 微调单帧尺寸
//     [v0.1.3]     2026-06-12 16:58:12   修改了角色朝向的BUG,补充了角色朝向的状态判定,分别加入了两个角色的backward动作
//     [v0.1.4]     2026-06-12 17:45:15   实现了镜头跟随机制,修复了移动打断开场动画的Bug
//     [v0.1.5]     2026-06-15 02:22:38   实现了跳跃功能,斜跳功能,修改了跳跃高度
//     [v0.1.6]     2026-06-15 13:28:32   修复了两名角色同时起跳越过对方时产生的镜头晃动和闪屏
//     [v0.1.7]     2026-06-15 21:06:43   角色同时新增轻拳,轻腿,重拳,重腿这些攻击动作
//     [v0.1.8]     2026-06-17 09:51:57   新增碰撞检测功能,同时也加入了受击动作,实现了攻击响应击退效果
//     [v0.1.9]     2026-06-18 17:34:33   实现下蹲系统（含蹲防无敌机制）和攻击视觉特效系统
//     [v0.2.0]     2026-06-21 20:49:28   实现了闪避功能,改善了一下攻击和受击的机制,下蹲防御时新增受击击退效果
//     [v0.2.1]     2026-06-22 09:33:20   实现伤害计算+血量系统+三局两胜回合制
//     [v0.2.2]     2026-06-22 10:23:02   修复回合重置开场动画/第三回合无法移动/镜头抖动/跳跃卡死
//     [v0.2.3]     2026-06-22 11:12:32   新增站立防御系统(StandBlock)+75%减伤机制
import QtQuick
import QtQuick.Controls
import FightGame

Item {
    id: root
    focus: true
    activeFocusOnTab: true

    // 场景属性, 由 SelectScreen 传入
    property var    stackViewRef: null
    property string p1Name:     ""
    property string p1Avatar:   ""
    property string p1Portrait: ""
    property string p1CharId:   ""
    property string p2Name:     ""
    property string p2Avatar:   ""
    property string p2Portrait: ""
    property string p2CharId:   ""

    property bool isOnline: false
    property bool isHost: false
    property var networkMgr: null

    function sendInput(action, pressed) {
        if (!isOnline || !networkMgr) return
        var msg = {"type": "input", "action": action}
        if (pressed !== undefined) msg.pressed = pressed
        networkMgr.sendMessage(msg)
    }

    function handleRemoteInput(action, pressed) {
        if (isHost) {
            // 远程处理 P2 输入, 先同步状态标志防止指令丢失
            p2Jumping = (director.p2Model.state === 5 || director.p2Model.state === 6)
            var p2state = director.p2Model.state
            p2Attacking = (p2state >= 7 && p2state <= 12) || p2state === 14 || p2state === 15
            p2Blocking = (p2state === 16)
            p2Crouching = (p2state === 13)
            switch (action) {
            case "move_right": pressed ? startMoveRight2() : stopMoveRight2(); break
            case "move_left":  pressed ? startMoveLeft2()  : stopMoveLeft2();  break
            case "jump":       if (!p2Jumping && !p2Blocking) startJump2(); break
            case "crouch":     pressed ? startCrouch2() : stopCrouch2(); break
            case "light_punch":
                if (!p2Jumping && !p2Blocking) {
                    p2Attacking = true
                    stopMove2Timers()
                    director.p2Model.playLightPunch()
                }
                break
            case "light_kick":
                if (!p2Jumping && !p2Blocking) {
                    p2Attacking = true
                    stopMove2Timers()
                    director.p2Model.playLightKick()
                }
                break
            case "heavy_punch":
                if (!p2Jumping && !p2Blocking) {
                    p2Attacking = true
                    stopMove2Timers()
                    director.p2Model.playHeavyPunch()
                }
                break
            case "heavy_kick":
                if (!p2Jumping && !p2Blocking) {
                    p2Attacking = true
                    stopMove2Timers()
                    director.p2Model.playHeavyKick()
                }
                break
            case "heavy_strike":
                if (!p2Jumping && !p2Blocking) {
                    p2Attacking = true
                    stopMove2Timers()
                    director.p2Model.playHeavyStrike()
                }
                break
            case "block":
                if (pressed) {
                    p2Blocking = true
                    if (!p2Jumping && !p2Attacking && !p2Crouching) {
                        director.p2Model.playStandBlock()
                        moveTimer2.stop(); isMoving2 = false
                    }
                } else {
                    p2Blocking = false
                    if (director.p2Model.state === 16) director.p2Model.releaseStandBlock()
                }
                break
            case "dodge_forward":  startDodge2(true);  break
            case "dodge_backward": startDodge2(false); break
            }
        } else {
            // 远程处理 P1 输入 (客机本地是P2, P1是远程)
            p1Jumping = (director.p1Model.state === 5 || director.p1Model.state === 6)
            var p1state = director.p1Model.state
            p1Attacking = (p1state >= 7 && p1state <= 12) || p1state === 14 || p1state === 15
            p1Blocking = (p1state === 16)
            p1Crouching = (p1state === 13)
            switch (action) {
            case "move_right": pressed ? startMoveRight() : stopMoveRight(); break
            case "move_left":  pressed ? startMoveLeft()  : stopMoveLeft();  break
            case "jump":       if (!p1Jumping && !p1Blocking) startJump(); break
            case "crouch":     pressed ? startCrouch1() : stopCrouch1(); break
            case "light_punch":
                if (!p1Jumping && !p1Blocking) {
                    p1Attacking = true
                    stopMove1Timers()
                    director.p1Model.playLightPunch()
                }
                break
            case "light_kick":
                if (!p1Jumping && !p1Blocking) {
                    p1Attacking = true
                    stopMove1Timers()
                    director.p1Model.playLightKick()
                }
                break
            case "heavy_punch":
                if (!p1Jumping && !p1Blocking) {
                    p1Attacking = true
                    stopMove1Timers()
                    director.p1Model.playHeavyPunch()
                }
                break
            case "heavy_kick":
                if (!p1Jumping && !p1Blocking) {
                    p1Attacking = true
                    stopMove1Timers()
                    director.p1Model.playHeavyKick()
                }
                break
            case "heavy_strike":
                if (!p1Jumping && !p1Blocking) {
                    p1Attacking = true
                    stopMove1Timers()
                    director.p1Model.playHeavyStrike()
                }
                break
            case "block":
                if (pressed) {
                    p1Blocking = true
                    if (!p1Jumping && !p1Attacking && !p1Crouching) {
                        director.p1Model.playStandBlock()
                        moveTimer.stop(); isMoving = false
                    }
                } else {
                    p1Blocking = false
                    if (director.p1Model.state === 16) director.p1Model.releaseStandBlock()
                }
                break
            case "dodge_forward":  startDodge1(true);  break
            case "dodge_backward": startDodge1(false); break
            }
        }
    }

    function stopMove1Timers() {
        moveTimer.moveRight = false
        moveTimer.moveLeft = false
        moveTimer.stop()
        isMoving = false
        p1CurrentAnim = ""
    }

    function stopMove2Timers() {
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = false
        moveTimer2.stop()
        isMoving2 = false
        p2CurrentAnim = ""
    }

    // 战斗导演与通用常量
    FightDirector {
        id: director
        rootHeight: root.height
        onHitDetected: function(attacker, damage) {
            function playHurtByAttackType(targetModel, attackerState) {
                if (attackerState === 7 || attackerState === 8) {
                    targetModel.playHurt1()
                } else if (attackerState === 9) {
                    targetModel.playHurt3()
                } else if (attackerState === 10) {
                    targetModel.playHurt2()
                } else {
                    targetModel.playHurt()
                }
            }
            
            if (attacker === 1) {
                p2Health = Math.max(0, p2Health - damage)
                if (!director.p2Model.isBlocking()) {
                    playHurtByAttackType(director.p2Model, director.p1Model.state)
                }
                if (director.p1Model.state >= 7 && director.p1Model.state <= 11) attackVfx.play(director.p1Model, director.p2Model)
            } else {
                p1Health = Math.max(0, p1Health - damage)
                if (!director.p1Model.isBlocking()) {
                    playHurtByAttackType(director.p1Model, director.p2Model.state)
                }
                if (director.p2Model.state >= 7 && director.p2Model.state <= 11) attackVfx.play(director.p2Model, director.p1Model)
            }
            checkRoundEnd()

            if (isOnline && isHost && networkMgr) {
                networkMgr.sendMessage({
                    "type": "hit",
                    "attacker": attacker,
                    "attackerState": attacker === 1 ? director.p1Model.state : director.p2Model.state,
                    "damage": damage,
                    "p1Health": p1Health,
                    "p2Health": p2Health,
                    "defenderBlocking": attacker === 1 ? director.p2Model.isBlocking() : director.p1Model.isBlocking()
                })
            }
        }
    }

    // 碰撞检测定时器, 每33ms检测一次(约30fps)
    // 联机模式下只在主机端运行碰撞检测, 客机接收主机的命中消息
    Timer {
        id: collisionTimer
        interval: 33
        repeat: true
        running: director.phase === FightDirector.Fighting && !resettingRound && (!isOnline || isHost)
        onTriggered: {
            director.checkCollision()
        }
    }

    // 倒计时定时器
    Timer {
        id: countdownTimer
        interval: 1000
        repeat: true
        running: director.phase === FightDirector.Fighting && !roundEnding && !resettingRound
        onTriggered: {
            if (timerSeconds > 0) {
                timerSeconds--
            }
            if (timerSeconds <= 0) {
                checkRoundEnd()
            }
        }
    }

    // 联机状态同步定时器: 主机每100ms向客机同步完整游戏状态
    Timer {
        id: syncTimer
        interval: 100
        repeat: true
        running: isOnline && isHost && director.phase === FightDirector.Fighting && !resettingRound
        onTriggered: {
            if (networkMgr) {
                networkMgr.sendMessage({
                    "type": "sync",
                    "p1Health": p1Health,
                    "p2Health": p2Health,
                    "p1x": director.p1Model.posXRatio,
                    "p2x": director.p2Model.posXRatio,
                    "timerSeconds": timerSeconds,
                    "p1Wins": p1Wins,
                    "p2Wins": p2Wins,
                    "currentRound": currentRound,
                    "roundEnding": roundEnding
                })
            }
        }
    }

    property real fitScale: Math.min(root.width / 900, root.height / 640)
    property real moveStep: 0.008
    property real bodyCollisionDist: 0.15  // 身体碰撞最小距离

    // P1 移动定时器, 每 16ms 更新一次位置
    Timer {
        id: moveTimer
        interval: 16
        repeat: true
        running: false
        property bool moveRight: false
        property bool moveLeft: false
        onTriggered: {
            var p1 = director.p1Model.posXRatio
            var p2 = director.p2Model.posXRatio
            if (moveRight) {
                var maxPos = (p1 < p2) ? p2 - bodyCollisionDist : p2 + 0.95
                maxPos = Math.min(maxPos, director.maxCameraOffset + 1.0)
                director.p1Model.posXRatio = Math.min(p1 + moveStep, maxPos)
            }
            if (moveLeft) {
                var minPos = (p1 > p2) ? p2 + bodyCollisionDist : p2 - 0.95
                minPos = Math.max(minPos, 0)
                director.p1Model.posXRatio = Math.max(p1 - moveStep, minPos)
            }
            director.updateCamera()
            updateFacing()
            var desired = ""
            if (moveRight) desired = director.p1Model.facingLeft ? "backward" : "forward"
            if (moveLeft)  desired = director.p1Model.facingLeft ? "forward" : "backward"
            if (desired !== "" && desired !== p1CurrentAnim) {
                p1CurrentAnim = desired
                if (desired === "forward") director.p1Model.playForward()
                else director.p1Model.playBackward()
            }
        }
    }

    // P1 移动状态
    property bool isMoving: false
    property bool p1Jumping: false
    property bool p1Attacking: false
    property bool p1Crouching: false
    property bool moveLeftPressed: false
    property bool moveRightPressed: false
    property string p1CurrentAnim: ""

    // 组合键缓冲: 等待第二个按键
    property bool p1WaitingCombo: false
    property bool p2WaitingCombo: false
    property bool p1ComboKeyJ: true   // 记录首发键: true=J, false=K
    property bool p2ComboKeyJ: true

    Timer {
        id: p1ComboTimer
        interval: 100
        onTriggered: {
            if (p1WaitingCombo) {
                p1WaitingCombo = false
                if (p1ComboKeyJ) {
                    startAttack1()
                    sendInput("light_punch")
                } else {
                    startAttackLightKick1()
                    sendInput("light_kick")
                }
            }
        }
    }
    Timer {
        id: p2ComboTimer
        interval: 100
        onTriggered: {
                if (p2WaitingCombo) {
                    p2WaitingCombo = false
                    if (p2ComboKeyJ) {
                        startAttack2()
                        sendInput("light_punch")
                    } else {
                        startAttackLightKick2()
                        sendInput("light_kick")
                    }
            }
        }
    }

    // P1 闪避方向判断: 根据角色朝向决定前后
    function getP1DodgeForward() {
        var fwd = !director.p1Model.facingLeft
        if (moveRightPressed && !moveLeftPressed) {
            return fwd       // 右键 → 朝对手方向 = 前闪
        } else if (moveLeftPressed && !moveRightPressed) {
            return !fwd      // 左键 → 背离对手 = 后闪
        }
        return true  // 默认前闪
    }

    // P2 闪避方向判断
    function getP2DodgeForward() {
        var fwd = !director.p2Model.facingLeft
        if (moveRight2Pressed && !moveLeft2Pressed) {
            return fwd
        } else if (moveLeft2Pressed && !moveRight2Pressed) {
            return !fwd
        }
        return true
    }

    // P1 向右移动
    function startMoveRight() {
        moveRightPressed = true
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16) return
        updateFacing()
        moveTimer.moveLeft = false
        moveTimer.moveRight = true
        if (!isMoving) {
            isMoving = true
            if (director.p1Model.facingLeft) {
                director.p1Model.playBackward()
                p1CurrentAnim = "backward"
            } else {
                director.p1Model.playForward()
                p1CurrentAnim = "forward"
            }
            moveTimer.start()
        }
    }
    // P1 向左移动
    function startMoveLeft() {
        moveLeftPressed = true
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16) return
        updateFacing()
        moveTimer.moveRight = false
        moveTimer.moveLeft = true
        if (!isMoving) {
            isMoving = true
            if (director.p1Model.facingLeft) {
                director.p1Model.playForward()
                p1CurrentAnim = "forward"
            } else {
                director.p1Model.playBackward()
                p1CurrentAnim = "backward"
            }
            moveTimer.start()
        }
    }
    function stopMoveRight() {
        moveRightPressed = false
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16) return
        updateFacing()
        if (moveLeftPressed) {
            moveTimer.moveRight = false
            moveTimer.moveLeft = true
            if (director.p1Model.facingLeft) {
                director.p1Model.playForward()
                p1CurrentAnim = "forward"
            } else {
                director.p1Model.playBackward()
                p1CurrentAnim = "backward"
            }
        } else {
            moveTimer.stop()
            moveTimer.moveRight = false
            moveTimer.moveLeft = false
            isMoving = false
            p1CurrentAnim = ""
            director.p1Model.playStand()
        }
    }
    function stopMoveLeft() {
        moveLeftPressed = false
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16) return
        updateFacing()
        if (moveRightPressed) {
            moveTimer.moveLeft = false
            moveTimer.moveRight = true
            if (director.p1Model.facingLeft) {
                director.p1Model.playBackward()
                p1CurrentAnim = "backward"
            } else {
                director.p1Model.playForward()
                p1CurrentAnim = "forward"
            }
        } else {
            moveTimer.stop()
            moveTimer.moveRight = false
            moveTimer.moveLeft = false
            isMoving = false
            p1CurrentAnim = ""
            director.p1Model.playStand()
        }
    }

    // P1 跳跃: 行走中按W触发对角跳(前跳/后跳), 站立时按W触发直跳
    function startJump() {
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16) return
        p1Jumping = true
        var wasMoving = isMoving
        var animBeforeJump = p1CurrentAnim
        moveTimer.moveRight = false
        moveTimer.moveLeft = false
        moveTimer.stop()
        isMoving = false
        p1CurrentAnim = ""
        if (wasMoving && (animBeforeJump === "forward" || animBeforeJump === "backward")) {
            director.p1Model.playDiagonalJump(animBeforeJump === "forward")
        } else {
            director.p1Model.playJump()
        }
    }

    // P1 轻拳攻击
    function startAttack1() {
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16) return
        p1Attacking = true
        moveTimer.moveRight = false
        moveTimer.moveLeft = false
        moveTimer.stop()
        isMoving = false
        p1CurrentAnim = ""
        director.p1Model.playLightPunch()
    }

    // P1 轻腿攻击
    function startAttackLightKick1() {
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16) return
        p1Attacking = true
        moveTimer.moveRight = false
        moveTimer.moveLeft = false
        moveTimer.stop()
        isMoving = false
        p1CurrentAnim = ""
        director.p1Model.playLightKick()
    }

    // P1 重拳攻击
    function startAttackHeavyPunch1() {
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16) return
        p1Attacking = true
        moveTimer.moveRight = false
        moveTimer.moveLeft = false
        moveTimer.stop()
        isMoving = false
        p1CurrentAnim = ""
        director.p1Model.playHeavyPunch()
    }

    // P1 重腿攻击
    function startAttackHeavyKick1() {
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16) return
        p1Attacking = true
        moveTimer.moveRight = false
        moveTimer.moveLeft = false
        moveTimer.stop()
        isMoving = false
        p1CurrentAnim = ""
        director.p1Model.playHeavyKick()
    }

    // P1 超重击攻击
    function startAttackHeavyStrike1() {
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16) return
        p1Attacking = true
        moveTimer.moveRight = false
        moveTimer.moveLeft = false
        moveTimer.stop()
        isMoving = false
        p1CurrentAnim = ""
        director.p1Model.playHeavyStrike()
    }

    // P1 闪避
    function startDodge1(forward) {
        if (p1Jumping) return
        p1Attacking = true
        moveTimer.moveRight = false
        moveTimer.moveLeft = false
        moveTimer.stop()
        isMoving = false
        p1CurrentAnim = ""
        director.p1Model.playDodge(forward)
    }

    // P1 下蹲
    function startCrouch1() {
        if (p1Jumping || p1Attacking || p1Blocking || director.p1Model.state === 16) return
        p1Crouching = true
        moveRightPressed = false
        moveLeftPressed = false
        moveTimer.moveRight = false
        moveTimer.moveLeft = false
        moveTimer.stop()
        isMoving = false
        p1CurrentAnim = ""
        director.p1Model.playCrouch()
    }

    // P1 退出下蹲
    function stopCrouch1() {
        p1Crouching = false
        director.p1Model.stopCrouch()
        // 恢复移动状态
        if (moveRightPressed && !moveLeftPressed) {
            startMoveRight()
        } else if (moveLeftPressed && !moveRightPressed) {
            startMoveLeft()
        }
    }

    // P2 移动定时器
    Timer {
        id: moveTimer2
        interval: 16
        repeat: true
        running: false
        property bool moveRight: false
        property bool moveLeft: false
        onTriggered: {
            var p2 = director.p2Model.posXRatio
            var p1 = director.p1Model.posXRatio
            if (moveRight) {
                var maxPos = (p2 < p1) ? p1 - bodyCollisionDist : p1 + 0.95
                maxPos = Math.min(maxPos, director.maxCameraOffset + 1.0)
                director.p2Model.posXRatio = Math.min(p2 + moveStep, maxPos)
            }
            if (moveLeft) {
                var minPos = (p2 > p1) ? p1 + bodyCollisionDist : p1 - 0.95
                minPos = Math.max(minPos, 0)
                director.p2Model.posXRatio = Math.max(p2 - moveStep, minPos)
            }
            director.updateCamera()
            updateFacing()
            var desired = ""
            if (moveRight) desired = director.p2Model.facingLeft ? "backward" : "forward"
            if (moveLeft)  desired = director.p2Model.facingLeft ? "forward" : "backward"
            if (desired !== "" && desired !== p2CurrentAnim) {
                p2CurrentAnim = desired
                if (desired === "forward") director.p2Model.playForward()
                else director.p2Model.playBackward()
            }
        }
    }

    // P2 移动状态
    property bool isMoving2: false
    property bool p2Jumping: false
    property bool p2Attacking: false
    property bool p2Crouching: false
    property bool moveLeft2Pressed: false
    property bool moveRight2Pressed: false
    property string p2CurrentAnim: ""

    // P2 向右移动
    function startMoveRight2() {
        moveRight2Pressed = true
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16) return
        updateFacing()
        moveTimer2.moveLeft = false
        moveTimer2.moveRight = true
        if (!isMoving2) {
            isMoving2 = true
            if (director.p2Model.facingLeft) {
                director.p2Model.playBackward()
                p2CurrentAnim = "backward"
            } else {
                director.p2Model.playForward()
                p2CurrentAnim = "forward"
            }
            moveTimer2.start()
        }
    }
    // P2 向左移动
    function startMoveLeft2() {
        moveLeft2Pressed = true
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16) return
        updateFacing()
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = true
        if (!isMoving2) {
            isMoving2 = true
            if (director.p2Model.facingLeft) {
                director.p2Model.playForward()
                p2CurrentAnim = "forward"
            } else {
                director.p2Model.playBackward()
                p2CurrentAnim = "backward"
            }
            moveTimer2.start()
        }
    }
    function stopMoveRight2() {
        moveRight2Pressed = false
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16) return
        updateFacing()
        if (moveLeft2Pressed) {
            moveTimer2.moveRight = false
            moveTimer2.moveLeft = true
            if (director.p2Model.facingLeft) {
                director.p2Model.playForward()
                p2CurrentAnim = "forward"
            } else {
                director.p2Model.playBackward()
                p2CurrentAnim = "backward"
            }
        } else {
            moveTimer2.stop()
            moveTimer2.moveRight = false
            moveTimer2.moveLeft = false
            isMoving2 = false
            p2CurrentAnim = ""
            director.p2Model.playStand()
        }
    }
    function stopMoveLeft2() {
        moveLeft2Pressed = false
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16) return
        updateFacing()
        if (moveRight2Pressed) {
            moveTimer2.moveLeft = false
            moveTimer2.moveRight = true
            if (director.p2Model.facingLeft) {
                director.p2Model.playBackward()
                p2CurrentAnim = "backward"
            } else {
                director.p2Model.playForward()
                p2CurrentAnim = "forward"
            }
        } else {
            moveTimer2.stop()
            moveTimer2.moveRight = false
            moveTimer2.moveLeft = false
            isMoving2 = false
            p2CurrentAnim = ""
            director.p2Model.playStand()
        }
    }

    // P2 跳跃: 行走中按↑触发对角跳(前跳/后跳), 站立时按↑触发直跳
    function startJump2() {
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16) return
        p2Jumping = true
        var wasMoving = isMoving2
        var animBeforeJump = p2CurrentAnim
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = false
        moveTimer2.stop()
        isMoving2 = false
        p2CurrentAnim = ""
        if (wasMoving && (animBeforeJump === "forward" || animBeforeJump === "backward")) {
            director.p2Model.playDiagonalJump(animBeforeJump === "forward")
        } else {
            director.p2Model.playJump()
        }
    }

    // P2 轻拳攻击
    function startAttack2() {
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16) return
        p2Attacking = true
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = false
        moveTimer2.stop()
        isMoving2 = false
        p2CurrentAnim = ""
        director.p2Model.playLightPunch()
    }

    // P2 轻腿攻击
    function startAttackLightKick2() {
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16) return
        p2Attacking = true
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = false
        moveTimer2.stop()
        isMoving2 = false
        p2CurrentAnim = ""
        director.p2Model.playLightKick()
    }

    // P2 重拳攻击
    function startAttackHeavyPunch2() {
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16) return
        p2Attacking = true
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = false
        moveTimer2.stop()
        isMoving2 = false
        p2CurrentAnim = ""
        director.p2Model.playHeavyPunch()
    }

    // P2 重腿攻击
    function startAttackHeavyKick2() {
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16) return
        p2Attacking = true
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = false
        moveTimer2.stop()
        isMoving2 = false
        p2CurrentAnim = ""
        director.p2Model.playHeavyKick()
    }

    // P2 超重击攻击
    function startAttackHeavyStrike2() {
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16) return
        p2Attacking = true
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = false
        moveTimer2.stop()
        isMoving2 = false
        p2CurrentAnim = ""
        director.p2Model.playHeavyStrike()
    }

    // P2 闪避
    function startDodge2(forward) {
        if (p2Jumping) return
        p2Attacking = true
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = false
        moveTimer2.stop()
        isMoving2 = false
        p2CurrentAnim = ""
        director.p2Model.playDodge(forward)
    }

    // P2 下蹲
    function startCrouch2() {
        if (p2Jumping || p2Attacking || p2Blocking || director.p2Model.state === 16) return
        p2Crouching = true
        moveRight2Pressed = false
        moveLeft2Pressed = false
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = false
        moveTimer2.stop()
        isMoving2 = false
        p2CurrentAnim = ""
        director.p2Model.playCrouch()
    }

    // P2 退出下蹲
    function stopCrouch2() {
        p2Crouching = false
        director.p2Model.stopCrouch()
        // 恢复移动状态
        if (moveRight2Pressed && !moveLeft2Pressed) {
            startMoveRight2()
        } else if (moveLeft2Pressed && !moveRight2Pressed) {
            startMoveLeft2()
        }
    }

    // 动态朝向: 始终面向对手
    function updateFacing() {
        director.p1Model.facingLeft = (director.p1Model.posXRatio > director.p2Model.posXRatio)
        director.p2Model.facingLeft = (director.p2Model.posXRatio > director.p1Model.posXRatio)
    }

    // 回合结束检查
    function checkRoundEnd() {
        if (roundEnding) return
        if (p1Health <= 0 || p2Health <= 0 || timerSeconds <= 0) {
            roundEnding = true
            if (p1Health <= 0 && p2Health > 0) {
                p2Wins++
            } else if (p2Health <= 0 && p1Health > 0) {
                p1Wins++
            } else if (timerSeconds <= 0) {
                if (p1Health > p2Health) p1Wins++
                else if (p2Health > p1Health) p2Wins++
            }
            
            if (isOnline && isHost && networkMgr) {
                networkMgr.sendMessage({
                    "type": "round_event",
                    "p1Wins": p1Wins,
                    "p2Wins": p2Wins,
                    "roundEnding": true
                })
            }

            if (p1Wins >= 2 || p2Wins >= 2) {
                matchResultTimer.start()
            } else {
                roundResetTimer.start()
            }
        }
    }

    // 回合重置
    function resetRound() {
        if (resettingRound || roundEnding === false) return
        resettingRound = true
        currentRound++
        p1Health = 100
        p2Health = 100
        timerSeconds = 60
        roundEnding = false
        director.resetForNewRound(p1CharId, p2CharId)
        resettingRound = false
    }

    // 回合重置定时器
    Timer {
        id: roundResetTimer
        interval: 2000
        onTriggered: resetRound()
    }

    // 比赛结果定时器
    Timer {
        id: matchResultTimer
        interval: 3000
        onTriggered: {
            if (isOnline && isHost && networkMgr) {
                networkMgr.sendMessage({"type": "match_end"})
            }
            if (stackViewRef) stackViewRef.pop()
        }
    }

    // 键盘输入, P1用A/D/W/J/K/U/L, P2用方向键/小键盘1-3/0
    property bool p1JPressed: false
    property bool p1KPressed: false
    property bool p2N1Pressed: false
    property bool p2N2Pressed: false

    Keys.onPressed: (event) => {
        if (event.isAutoRepeat || director.phase !== FightDirector.Fighting || resettingRound) return
        if (isOnline) {
            var isP1Key = (event.key === Qt.Key_D || event.key === Qt.Key_A || event.key === Qt.Key_W || event.key === Qt.Key_S || event.key === Qt.Key_J || event.key === Qt.Key_K || event.key === Qt.Key_U || event.key === Qt.Key_L || event.key === Qt.Key_I || event.key === Qt.Key_G)
            if (isHost && !isP1Key) return
            if (!isHost && isP1Key) return
        }
        switch (event.key) {
        case Qt.Key_D:      startMoveRight(); sendInput("move_right", true); break
        case Qt.Key_A:      startMoveLeft();  sendInput("move_left", true);  break
        case Qt.Key_S:      startCrouch1();   sendInput("crouch", true);     break
        case Qt.Key_W:      startJump();      sendInput("jump");             break
        case Qt.Key_J:
            p1JPressed = true
            if (p1WaitingCombo && p1KPressed && (moveRightPressed || moveLeftPressed)) {
                p1ComboTimer.stop()
                p1WaitingCombo = false
                var p1DodgeDir = getP1DodgeForward()
                startDodge1(p1DodgeDir)
                sendInput(p1DodgeDir ? "dodge_forward" : "dodge_backward")
            } else {
                p1ComboKeyJ = true
                p1WaitingCombo = true
                p1ComboTimer.restart()
            }
            break
        case Qt.Key_K:
            p1KPressed = true
            if (p1WaitingCombo && p1JPressed && (moveRightPressed || moveLeftPressed)) {
                p1ComboTimer.stop()
                p1WaitingCombo = false
                var p1DodgeDir2 = getP1DodgeForward()
                startDodge1(p1DodgeDir2)
                sendInput(p1DodgeDir2 ? "dodge_forward" : "dodge_backward")
            } else {
                p1ComboKeyJ = false
                p1WaitingCombo = true
                p1ComboTimer.restart()
            }
            break
        case Qt.Key_U:      startAttackHeavyPunch1();  sendInput("heavy_punch");  break
        case Qt.Key_L:      startAttackHeavyKick1();   sendInput("heavy_kick");   break
        case Qt.Key_I:      startAttackHeavyStrike1(); sendInput("heavy_strike"); break
        case Qt.Key_G:
            p1Blocking = true
            if (!p1Jumping && !p1Attacking && !p1Crouching) {
                director.p1Model.playStandBlock()
                moveTimer.stop()
                isMoving = false
            }
            sendInput("block", true)
            break
        case Qt.Key_Right:  startMoveRight2(); sendInput("move_right", true); break
        case Qt.Key_Left:   startMoveLeft2();  sendInput("move_left", true);  break
        case Qt.Key_Down:   startCrouch2();    sendInput("crouch", true);     break
        case Qt.Key_Up:     startJump2();      sendInput("jump");             break
        case Qt.Key_1:
            if (event.modifiers & Qt.KeypadModifier) {
                p2N1Pressed = true
                var dir1 = moveRight2Pressed || moveLeft2Pressed
                if (p2WaitingCombo && p2N2Pressed) {
                    p2ComboTimer.stop()
                    p2WaitingCombo = false
                    var p2DodgeDir = dir1 ? getP2DodgeForward() : true
                    startDodge2(p2DodgeDir)
                    sendInput(p2DodgeDir ? "dodge_forward" : "dodge_backward")
                } else {
                    p2ComboKeyJ = true
                    p2WaitingCombo = true
                    p2ComboTimer.restart()
                }
            }
            break
        case Qt.Key_2:
            if (event.modifiers & Qt.KeypadModifier) {
                p2N2Pressed = true
                var dir2 = moveRight2Pressed || moveLeft2Pressed
                if (p2WaitingCombo && p2N1Pressed) {
                    p2ComboTimer.stop()
                    p2WaitingCombo = false
                    var p2DodgeDir2 = dir2 ? getP2DodgeForward() : true
                    startDodge2(p2DodgeDir2)
                    sendInput(p2DodgeDir2 ? "dodge_forward" : "dodge_backward")
                } else {
                    p2ComboKeyJ = false
                    p2WaitingCombo = true
                    p2ComboTimer.restart()
                }
            }
            break
        case Qt.Key_3:      if (event.modifiers & Qt.KeypadModifier) { startAttackHeavyPunch2();  sendInput("heavy_punch");  } break
        case Qt.Key_0:      if (event.modifiers & Qt.KeypadModifier) { startAttackHeavyKick2();   sendInput("heavy_kick");   } break
        case Qt.Key_5:      if (event.modifiers & Qt.KeypadModifier) { startAttackHeavyStrike2(); sendInput("heavy_strike"); } break
        case Qt.Key_4:
            if (event.modifiers & Qt.KeypadModifier) {
                p2Blocking = true
                if (!p2Jumping && !p2Attacking && !p2Crouching) {
                    director.p2Model.playStandBlock()
                    moveTimer2.stop()
                    isMoving2 = false
                }
                sendInput("block", true)
            }
            break
        }
    }
    Keys.onReleased: (event) => {
        if (event.isAutoRepeat || director.phase !== FightDirector.Fighting || resettingRound) return
        if (isOnline) {
            var isP1Key = (event.key === Qt.Key_D || event.key === Qt.Key_A || event.key === Qt.Key_S || event.key === Qt.Key_J || event.key === Qt.Key_K || event.key === Qt.Key_G)
            if (isHost && !isP1Key) return
            if (!isHost && isP1Key) return
        }
        switch (event.key) {
        case Qt.Key_D:      stopMoveRight(); sendInput("move_right", false); break
        case Qt.Key_A:      stopMoveLeft();  sendInput("move_left", false);  break
        case Qt.Key_S:      stopCrouch1();   sendInput("crouch", false);     break
        case Qt.Key_J:      p1JPressed = false; break
        case Qt.Key_K:      p1KPressed = false; break
        case Qt.Key_Right:  stopMoveRight2(); sendInput("move_right", false); break
        case Qt.Key_Left:   stopMoveLeft2();  sendInput("move_left", false);  break
        case Qt.Key_Down:   stopCrouch2();    sendInput("crouch", false);     break
        case Qt.Key_G:
            p1Blocking = false
            if (director.p1Model.state === 16) {  // StandBlock
                director.p1Model.releaseStandBlock()
            }
            sendInput("block", false)
            break
        case Qt.Key_1:      if (event.modifiers & Qt.KeypadModifier) p2N1Pressed = false; break
        case Qt.Key_2:      if (event.modifiers & Qt.KeypadModifier) p2N2Pressed = false; break
        case Qt.Key_4:
            if (event.modifiers & Qt.KeypadModifier) {
                p2Blocking = false
                if (director.p2Model.state === 16) {  // StandBlock
                    director.p2Model.releaseStandBlock()
                }
                sendInput("block", false)
            }
            break
        }
    }

    // 背景层
    Rectangle {
        anchors.fill: parent
        color: "black"
        z: -2
    }

    Item {
        id: bgView
        anchors.fill: parent
        clip: true
        z: 0

        AnimatedImage {
            id: monacoGif
            width: parent.width * 2
            height: parent.height
            source: "qrc:/images/FightBackGround/Monaco.gif"
            fillMode: Image.PreserveAspectCrop
            smooth: false
            mipmap: false
            cache: true
            asynchronous: true
            paused: false
            horizontalAlignment: Image.AlignLeft
            verticalAlignment: Image.AlignVCenter
            x: Math.min(0, Math.max(-(width - parent.width), -root.width * director.cameraOffset * 0.5))
            y: 0
        }
    }

    // P1 角色渲染, 精灵表视口裁剪
    Item {
        id: p1Layer
        x: root.width * (director.p1Model.posXRatio - director.cameraOffset) - width / 2 + director.p1Model.animOffsetX * fitScale * director.p1Model.visualScale * (director.p1Model.facingLeft ? -1 : 1)
        y: director.p1Model.positionY
        width: director.p1Model.frameWidth
        height: director.p1Model.frameHeight
        transformOrigin: Item.Bottom
        clip: true
        z: 1

        transform: [
            Scale {
                origin.x: p1Layer.width / 2
                origin.y: p1Layer.height
                xScale: fitScale * (director.p1Model.facingLeft ? -1 : 1) * director.p1Model.visualScale
                yScale: fitScale * director.p1Model.visualScale
            }
        ]

        Image {
            source: director.p1Model.sourcePath
            width: director.p1Model.totalFrames * director.p1Model.frameWidth
            height: director.p1Model.frameHeight
            x: -(director.p1Model.currentFrame * director.p1Model.frameWidth)
            y: 0
            fillMode: Image.Stretch
            smooth: false
            mipmap: false
            cache: true
            asynchronous: false
        }
    }

    // P2 角色渲染
    Item {
        id: p2Layer
        x: root.width * (director.p2Model.posXRatio - director.cameraOffset) - width / 2 + director.p2Model.animOffsetX * fitScale * director.p2Model.visualScale * (director.p2Model.facingLeft ? -1 : 1)
        y: director.p2Model.positionY
        width: director.p2Model.frameWidth
        height: director.p2Model.frameHeight
        transformOrigin: Item.Bottom
        clip: true
        z: 1

        transform: [
            Scale {
                origin.x: p2Layer.width / 2
                origin.y: p2Layer.height
                xScale: fitScale * (director.p2Model.facingLeft ? -1 : 1) * director.p2Model.visualScale
                yScale: fitScale * director.p2Model.visualScale
            }
        ]

        Image {
            source: director.p2Model.sourcePath
            width: director.p2Model.totalFrames * director.p2Model.frameWidth
            height: director.p2Model.frameHeight
            x: -(director.p2Model.currentFrame * director.p2Model.frameWidth)
            y: 0
            fillMode: Image.Stretch
            smooth: false
            mipmap: false
            cache: true
            asynchronous: false
        }
    }

    // Debug 可视化: 判定框
    property bool showDebugHitbox: false  // 调试判定框显示

    // P1 受击框 (绿色)
    Rectangle {
        visible: showDebugHitbox
        x: root.width * (director.p1Model.hurtboxX - director.cameraOffset) - width / 2
        y: director.p1Model.hurtboxY - height / 2
        width: director.p1Model.hurtboxW * fitScale
        height: director.p1Model.hurtboxH * fitScale
        color: "transparent"
        border.color: "lime"
        border.width: 2
        z: 20
    }

    // P2 受击框 (绿色)
    Rectangle {
        visible: showDebugHitbox
        x: root.width * (director.p2Model.hurtboxX - director.cameraOffset) - width / 2
        y: director.p2Model.hurtboxY - height / 2
        width: director.p2Model.hurtboxW * fitScale
        height: director.p2Model.hurtboxH * fitScale
        color: "transparent"
        border.color: "lime"
        border.width: 2
        z: 20
    }

    // P1 攻击框 (红色)
    Rectangle {
        visible: showDebugHitbox && director.p1Model.attackActive
        x: root.width * (director.p1Model.hitboxX - director.cameraOffset) - width / 2
        y: director.p1Model.hitboxY - height / 2
        width: director.p1Model.hitboxRadius * 2 * fitScale
        height: director.p1Model.hitboxRadius * 2 * fitScale
        radius: width / 2
        color: Qt.rgba(1, 0, 0, 0.3)
        border.color: "red"
        border.width: 2
        z: 20
    }

    // P2 攻击框 (红色)
    Rectangle {
        visible: showDebugHitbox && director.p2Model.attackActive
        x: root.width * (director.p2Model.hitboxX - director.cameraOffset) - width / 2
        y: director.p2Model.hitboxY - height / 2
        width: director.p2Model.hitboxRadius * 2 * fitScale
        height: director.p2Model.hitboxRadius * 2 * fitScale
        radius: width / 2
        color: Qt.rgba(1, 0, 0, 0.3)
        border.color: "red"
        border.width: 2
        z: 20
    }

    // Attack VFX 攻击特效
    Item {
        id: attackVfx
        visible: false
        z: 15
        width: 32
        height: 31
        scale: fitScale * 3.0

        Image { id: vfx1; source: "qrc:/images/Attack/1.png"; anchors.centerIn: parent }
        Image { id: vfx2; source: "qrc:/images/Attack/2.png"; anchors.centerIn: parent }
        Image { id: vfx3; source: "qrc:/images/Attack/3.png"; anchors.centerIn: parent }

        Timer {
            id: vfxTimer
            interval: 50
            repeat: true
            property int step: 0
            onTriggered: {
                vfx1.visible = (step === 0)
                vfx2.visible = (step === 1)
                vfx3.visible = (step === 2)
                step++
                if (step >= 3) {
                    stop()
                    attackVfx.visible = false
                    step = 0
                    vfx1.visible = false
                    vfx2.visible = false
                    vfx3.visible = false
                }
            }
        }

        function play(atkModel, defModel) {
            var midX = atkModel.posXRatio * 0.3 + defModel.posXRatio * 0.7
            var neckY = defModel.positionY + defModel.frameHeight * (1 - defModel.visualScale * 0.8)
            var s = atkModel.state
            var offset
            if (atkModel.visualScale <= 1.5) {
                offset = -30
            } else {
                if (s === 8 || s === 10) offset = 120
                else offset = 60
            }
            x = root.width * (midX - director.cameraOffset) - width / 2
            y = neckY - height / 2 + offset
            visible = true
            vfx1.visible = false
            vfx2.visible = false
            vfx3.visible = false
            vfxTimer.step = 0
            vfxTimer.restart()
        }
    }

    // HUD 顶部栏
    Rectangle {
        id: hudBar
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: 82
        color: Qt.rgba(0, 0, 0, 0.85)
        z: 10

        Rectangle {
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: 2
            color: "darkgoldenrod"
        }
        Rectangle {
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            height: 2
            color: "darkgoldenrod"
        }
    }

    // P1 HUD, 头像 + 名字 + 血条 + 能量条
    Item {
        anchors {
            left: parent.left
            leftMargin: 12
            top: parent.top
            topMargin: 8
        }
        width: 340
        height: 76
        z: 11

        Rectangle {
            id: p1PortraitFrame
            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }
            width: 54
            height: 54
            color: "black"
            border.color: "darkgoldenrod"
            border.width: 2
            radius: 2

            Image {
                anchors.fill: parent
                anchors.margins: 2
                source: p1Avatar
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                smooth: false
            }
        }

        Column {
            anchors {
                left: p1PortraitFrame.right
                leftMargin: 10
                verticalCenter: parent.verticalCenter
            }
            spacing: 4

            Text {
                text: p1Name
                font.pixelSize: 12
                font.bold: true
                color: "wheat"
                font.family: "monospace"
            }
            // 血条
            Rectangle {
                width: 240
                height: 16
                color: "black"
                border.color: "darkgoldenrod"
                border.width: 1
                radius: 1

                Rectangle {
                    anchors {
                        left: parent.left
                        leftMargin: 1
                        verticalCenter: parent.verticalCenter
                    }
                    width: Math.max(0, (parent.width - 2) * (p1Health / 100.0))
                    height: parent.height - 2
                    color: "firebrick"
                    radius: 1

                    Rectangle {
                        anchors {
                            top: parent.top
                            bottom: parent.bottom
                            right: parent.right
                        }
                        width: 6
                        color: "gold"
                        visible: p1Health > 0
                    }
                    Behavior on width {
                        NumberAnimation {
                            duration: 200
                            easing.type: Easing.OutCubic
                        }
                    }
                }
            }
        }
    }

    // 倒计时器
    Rectangle {
        anchors {
            centerIn: hudBar
            verticalCenterOffset: 2
        }
        width: 52
        height: 52
        color: "black"
        border.color: "darkgoldenrod"
        border.width: 2
        radius: 26
        z: 11

        Text {
            anchors.centerIn: parent
            text: timerSeconds
            font.pixelSize: 26
            font.bold: true
            color: timerSeconds <= 10 ? "red" : "wheat"
            font.family: "monospace"
        }
    }

    // 回合显示 + 联机模式标识
    Text {
        id: roundText
        anchors.horizontalCenter: parent.horizontalCenter
        y: 8
        text: (isOnline ? "联机对战 · " : "") + "Round " + currentRound
        font.pixelSize: 14
        font.bold: true
        color: "wheat"
        font.family: "monospace"
        z: 12
    }

    // P1 胜场标记
    Row {
        anchors {
            left: parent.left
            leftMargin: 100
            top: parent.top
            topMargin: 8
        }
        spacing: 6
        z: 12
        
        Repeater {
            model: 2
            Rectangle {
                width: 12
                height: 12
                radius: 6
                color: index < p1Wins ? "gold" : "gray"
                border.color: "darkgoldenrod"
                border.width: 1
            }
        }
    }

    // P2 胜场标记
    Row {
        anchors {
            right: parent.right
            rightMargin: 100
            top: parent.top
            topMargin: 8
        }
        spacing: 6
        z: 12
        
        Repeater {
            model: 2
            Rectangle {
                width: 12
                height: 12
                radius: 6
                color: index < p2Wins ? "gold" : "gray"
                border.color: "darkgoldenrod"
                border.width: 1
            }
        }
    }

    // 回合结束提示
    Text {
        id: roundResultText
        anchors.centerIn: parent
        visible: roundEnding
        font.pixelSize: 48
        font.bold: true
        color: "gold"
        style: Text.Outline
        styleColor: "black"
        z: 20
        text: {
            if (p1Wins >= 2) return p1Name + " WINS!"
            if (p2Wins >= 2) return p2Name + " WINS!"
            if (p1Health <= 0 && p2Health > 0) return p2Name + " WIN!"
            if (p2Health <= 0 && p1Health > 0) return p1Name + " WIN!"
            return "DRAW!"
        }
    }

    // P2 HUD, 头像 + 名字 + 血条 + 能量条
    Item {
        anchors {
            right: parent.right
            rightMargin: 12
            top: parent.top
            topMargin: 8
        }
        width: 340
        height: 76
        z: 11

        Rectangle {
            id: p2PortraitFrame
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
            }
            width: 54
            height: 54
            color: "black"
            border.color: "darkgoldenrod"
            border.width: 2
            radius: 2

            Image {
                anchors.fill: parent
                anchors.margins: 2
                source: p2Avatar
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                smooth: false
                mirror: true
            }
        }

        Column {
            anchors {
                right: p2PortraitFrame.left
                rightMargin: 10
                verticalCenter: parent.verticalCenter
            }
            spacing: 4

            Text {
                anchors.right: parent.right
                text: p2Name
                font.pixelSize: 12
                font.bold: true
                color: "wheat"
                font.family: "monospace"
                horizontalAlignment: Text.AlignRight
            }
            // 血条
            Rectangle {
                width: 240
                height: 16
                color: "black"
                border.color: "darkgoldenrod"
                border.width: 1
                radius: 1

                Rectangle {
                    anchors {
                        right: parent.right
                        rightMargin: 1
                        verticalCenter: parent.verticalCenter
                    }
                    width: Math.max(0, (parent.width - 2) * (p2Health / 100.0))
                    height: parent.height - 2
                    color: "firebrick"
                    radius: 1

                    Rectangle {
                        anchors {
                            top: parent.top
                            bottom: parent.bottom
                            left: parent.left
                        }
                        width: 6
                        color: "gold"
                        visible: p2Health > 0
                    }
                    Behavior on width {
                        NumberAnimation {
                            duration: 200
                            easing.type: Easing.OutCubic
                        }
                    }
                }
            }
        }
    }

    property int p1Health: 100
    property int p2Health: 100
    property int timerSeconds: 60
    property int p1Wins: 0
    property int p2Wins: 0
    property int currentRound: 1
    property bool roundEnding: false
    property bool resettingRound: false
    property bool p1Blocking: false
    property bool p2Blocking: false

    // 底部按钮与调试信息
    Button {
        anchors {
            left: parent.left
            bottom: parent.bottom
            margins: 12
        }
        width: 80
        height: 28
        z: 10
        text: "<- BACK"
        onClicked: { if (stackViewRef) stackViewRef.pop() }

        contentItem: Text {
            text: "<- BACK"
            color: "darkgoldenrod"
            font.pixelSize: 11
            font.family: "monospace"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            color: Qt.rgba(0, 0, 0, 0.8)
            border.color: "darkgoldenrod"
            border.width: 1
            radius: 2
        }
    }

    Button {
        anchors {
            right: parent.right
            bottom: parent.bottom
            margins: 12
        }
        width: 80
        height: 28
        z: 10
        text: ">>"
        onClicked: { director.p1Model.playForward() }

        contentItem: Text {
            text: ">>"
            color: "darkgoldenrod"
            font.pixelSize: 11
            font.family: "monospace"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            color: Qt.rgba(0, 0, 0, 0.8)
            border.color: "darkgoldenrod"
            border.width: 1
            radius: 2
        }
    }

    // 调试信息, 显示双方当前帧 / 总帧数
    Rectangle {
        anchors {
            right: parent.right
            bottom: parent.bottom
            margins: 12
        }
        width: debugText.implicitWidth + 16
        height: 22
        color: Qt.rgba(0, 0, 0, 0.6)
        radius: 2
        z: 12

        Text {
            id: debugText
            anchors.centerIn: parent
            text: "P1:" + director.p1Model.currentFrame + "/" + director.p1Model.totalFrames
                  + " P2:" + director.p2Model.currentFrame + "/" + director.p2Model.totalFrames
            color: "darkgreen"
            font.pixelSize: 10
            font.family: "monospace"
        }
    }

    // 初始化, 组件加载完成后开始战斗
    Component.onCompleted: {
        director.start(p1CharId, p2CharId)
        root.forceActiveFocus()
    }

    Connections {
        target: director.p1Model
        function onJumpFinished() {
            p1Jumping = false
            updateFacing()
            director.updateCamera()
            if (moveRightPressed && !moveLeftPressed) {
                isMoving = true
                moveTimer.moveLeft = false
                moveTimer.moveRight = true
                if (director.p1Model.facingLeft) {
                    director.p1Model.playBackward()
                    p1CurrentAnim = "backward"
                } else {
                    director.p1Model.playForward()
                    p1CurrentAnim = "forward"
                }
                moveTimer.start()
            } else if (moveLeftPressed && !moveRightPressed) {
                isMoving = true
                moveTimer.moveRight = false
                moveTimer.moveLeft = true
                if (director.p1Model.facingLeft) {
                    director.p1Model.playForward()
                    p1CurrentAnim = "forward"
                } else {
                    director.p1Model.playBackward()
                    p1CurrentAnim = "backward"
                }
                moveTimer.start()
            } else if (moveRightPressed || moveLeftPressed) {
                isMoving = true
                moveTimer.moveRight = moveRightPressed
                moveTimer.moveLeft = moveLeftPressed
                if (director.p1Model.facingLeft) {
                    director.p1Model.playBackward()
                    p1CurrentAnim = "backward"
                } else {
                    director.p1Model.playForward()
                    p1CurrentAnim = "forward"
                }
                moveTimer.start()
            } else {
                director.p1Model.playStand()
                moveTimer.moveRight = false
                moveTimer.moveLeft = false
                isMoving = false
            }
        }
    }

    Connections {
        target: director.p1Model
        function onAttackFinished() {
            p1Attacking = false
            updateFacing()
            director.updateCamera()
            if (p1Crouching) {
                // 蹲攻击完毕, 已回到蹲姿, 不做任何操作
                return
            }
            if (moveRightPressed && !moveLeftPressed) {
                isMoving = true
                moveTimer.moveLeft = false
                moveTimer.moveRight = true
                if (director.p1Model.facingLeft) {
                    director.p1Model.playBackward()
                    p1CurrentAnim = "backward"
                } else {
                    director.p1Model.playForward()
                    p1CurrentAnim = "forward"
                }
                moveTimer.start()
            } else if (moveLeftPressed && !moveRightPressed) {
                isMoving = true
                moveTimer.moveRight = false
                moveTimer.moveLeft = true
                if (director.p1Model.facingLeft) {
                    director.p1Model.playForward()
                    p1CurrentAnim = "forward"
                } else {
                    director.p1Model.playBackward()
                    p1CurrentAnim = "backward"
                }
                moveTimer.start()
            } else {
                director.p1Model.playStand()
                moveTimer.moveRight = false
                moveTimer.moveLeft = false
                isMoving = false
            }
        }
        function onHurtFinished() {
            p1Attacking = false
            p1Jumping = false
            p1Crouching = false
            updateFacing()
            director.updateCamera()
            if (moveRightPressed && !moveLeftPressed) {
                isMoving = true
                moveTimer.moveLeft = false
                moveTimer.moveRight = true
                if (director.p1Model.facingLeft) {
                    director.p1Model.playBackward()
                    p1CurrentAnim = "backward"
                } else {
                    director.p1Model.playForward()
                    p1CurrentAnim = "forward"
                }
                moveTimer.start()
            } else if (moveLeftPressed && !moveRightPressed) {
                isMoving = true
                moveTimer.moveRight = false
                moveTimer.moveLeft = true
                if (director.p1Model.facingLeft) {
                    director.p1Model.playForward()
                    p1CurrentAnim = "forward"
                } else {
                    director.p1Model.playBackward()
                    p1CurrentAnim = "backward"
                }
                moveTimer.start()
            } else {
                director.p1Model.playStand()
                moveTimer.moveRight = false
                moveTimer.moveLeft = false
                isMoving = false
            }
        }
        function onDodgeFinished() {
            p1Attacking = false
            updateFacing()
            director.updateCamera()
            if (moveRightPressed && !moveLeftPressed) {
                isMoving = true
                moveTimer.moveLeft = false
                moveTimer.moveRight = true
                if (director.p1Model.facingLeft) {
                    director.p1Model.playBackward()
                    p1CurrentAnim = "backward"
                } else {
                    director.p1Model.playForward()
                    p1CurrentAnim = "forward"
                }
                moveTimer.start()
            } else if (moveLeftPressed && !moveRightPressed) {
                isMoving = true
                moveTimer.moveRight = false
                moveTimer.moveLeft = true
                if (director.p1Model.facingLeft) {
                    director.p1Model.playForward()
                    p1CurrentAnim = "forward"
                } else {
                    director.p1Model.playBackward()
                    p1CurrentAnim = "backward"
                }
                moveTimer.start()
            } else {
                director.p1Model.playStand()
                moveTimer.moveRight = false
                moveTimer.moveLeft = false
                isMoving = false
            }
        }
    }

    Connections {
        target: director.p2Model
        function onJumpFinished() {
            p2Jumping = false
            updateFacing()
            director.updateCamera()
            if (moveRight2Pressed && !moveLeft2Pressed) {
                isMoving2 = true
                moveTimer2.moveLeft = false
                moveTimer2.moveRight = true
                if (director.p2Model.facingLeft) {
                    director.p2Model.playBackward()
                    p2CurrentAnim = "backward"
                } else {
                    director.p2Model.playForward()
                    p2CurrentAnim = "forward"
                }
                moveTimer2.start()
            } else if (moveLeft2Pressed && !moveRight2Pressed) {
                isMoving2 = true
                moveTimer2.moveRight = false
                moveTimer2.moveLeft = true
                if (director.p2Model.facingLeft) {
                    director.p2Model.playForward()
                    p2CurrentAnim = "forward"
                } else {
                    director.p2Model.playBackward()
                    p2CurrentAnim = "backward"
                }
                moveTimer2.start()
            } else if (moveRight2Pressed || moveLeft2Pressed) {
                isMoving2 = true
                moveTimer2.moveRight = moveRight2Pressed
                moveTimer2.moveLeft = moveLeft2Pressed
                if (director.p2Model.facingLeft) {
                    director.p2Model.playBackward()
                    p2CurrentAnim = "backward"
                } else {
                    director.p2Model.playForward()
                    p2CurrentAnim = "forward"
                }
                moveTimer2.start()
            } else {
                director.p2Model.playStand()
                moveTimer2.moveRight = false
                moveTimer2.moveLeft = false
                isMoving2 = false
            }
        }
    }

    Connections {
        target: director.p2Model
        function onAttackFinished() {
            p2Attacking = false
            updateFacing()
            director.updateCamera()
            if (p2Crouching) {
                // 蹲攻击完毕, 已回到蹲姿, 不做任何操作
                return
            }
            if (moveRight2Pressed && !moveLeft2Pressed) {
                isMoving2 = true
                moveTimer2.moveLeft = false
                moveTimer2.moveRight = true
                if (director.p2Model.facingLeft) {
                    director.p2Model.playBackward()
                    p2CurrentAnim = "backward"
                } else {
                    director.p2Model.playForward()
                    p2CurrentAnim = "forward"
                }
                moveTimer2.start()
            } else if (moveLeft2Pressed && !moveRight2Pressed) {
                isMoving2 = true
                moveTimer2.moveRight = false
                moveTimer2.moveLeft = true
                if (director.p2Model.facingLeft) {
                    director.p2Model.playForward()
                    p2CurrentAnim = "forward"
                } else {
                    director.p2Model.playBackward()
                    p2CurrentAnim = "backward"
                }
                moveTimer2.start()
            } else {
                director.p2Model.playStand()
                moveTimer2.moveRight = false
                moveTimer2.moveLeft = false
                isMoving2 = false
            }
        }
        function onHurtFinished() {
            p2Attacking = false
            p2Jumping = false
            p2Crouching = false
            updateFacing()
            director.updateCamera()
            if (moveRight2Pressed && !moveLeft2Pressed) {
                isMoving2 = true
                moveTimer2.moveLeft = false
                moveTimer2.moveRight = true
                if (director.p2Model.facingLeft) {
                    director.p2Model.playBackward()
                    p2CurrentAnim = "backward"
                } else {
                    director.p2Model.playForward()
                    p2CurrentAnim = "forward"
                }
                moveTimer2.start()
            } else if (moveLeft2Pressed && !moveRight2Pressed) {
                isMoving2 = true
                moveTimer2.moveRight = false
                moveTimer2.moveLeft = true
                if (director.p2Model.facingLeft) {
                    director.p2Model.playForward()
                    p2CurrentAnim = "forward"
                } else {
                    director.p2Model.playBackward()
                    p2CurrentAnim = "backward"
                }
                moveTimer2.start()
            } else {
                director.p2Model.playStand()
                moveTimer2.moveRight = false
                moveTimer2.moveLeft = false
                isMoving2 = false
            }
        }
        function onDodgeFinished() {
            p2Attacking = false
            updateFacing()
            director.updateCamera()
            if (moveRight2Pressed && !moveLeft2Pressed) {
                isMoving2 = true
                moveTimer2.moveLeft = false
                moveTimer2.moveRight = true
                if (director.p2Model.facingLeft) {
                    director.p2Model.playBackward()
                    p2CurrentAnim = "backward"
                } else {
                    director.p2Model.playForward()
                    p2CurrentAnim = "forward"
                }
                moveTimer2.start()
            } else if (moveLeft2Pressed && !moveRight2Pressed) {
                isMoving2 = true
                moveTimer2.moveRight = false
                moveTimer2.moveLeft = true
                if (director.p2Model.facingLeft) {
                    director.p2Model.playForward()
                    p2CurrentAnim = "forward"
                } else {
                    director.p2Model.playBackward()
                    p2CurrentAnim = "backward"
                }
                moveTimer2.start()
            } else {
                director.p2Model.playStand()
                moveTimer2.moveRight = false
                moveTimer2.moveLeft = false
                isMoving2 = false
            }
        }
    }

    Connections {
        target: director.p1Model
        function onPosXRatioChanged() { director.updateCamera() }
    }

    Connections {
        target: director.p2Model
        function onPosXRatioChanged() { director.updateCamera() }
    }

    Connections {
        id: netInput
        target: networkMgr
        enabled: isOnline && networkMgr !== null

        function onMessageReceived(msg) {
            if (msg.type === "input") {
                handleRemoteInput(msg.action, msg.pressed)
            } else if (msg.type === "hit" && !isHost) {
                p1Health = msg.p1Health
                p2Health = msg.p2Health

                var attackerState = msg.attackerState
                var playHurt = function(targetModel, atkState) {
                    if (atkState === 7 || atkState === 8) {
                        targetModel.playHurt1()
                    } else if (atkState === 9) {
                        targetModel.playHurt3()
                    } else if (atkState === 10) {
                        targetModel.playHurt2()
                    } else {
                        targetModel.playHurt()
                    }
                }
                if (msg.attacker === 1) {
                    if (!msg.defenderBlocking) {
                        playHurt(director.p2Model, attackerState)
                    }
                    if (attackerState >= 7 && attackerState <= 11) attackVfx.play(director.p1Model, director.p2Model)
                } else {
                    if (!msg.defenderBlocking) {
                        playHurt(director.p1Model, attackerState)
                    }
                    if (attackerState >= 7 && attackerState <= 11) attackVfx.play(director.p2Model, director.p1Model)
                }
                checkRoundEnd()
            } else if (msg.type === "sync" && !isHost) {
                p1Health = msg.p1Health
                p2Health = msg.p2Health
                timerSeconds = msg.timerSeconds
                p1Wins = msg.p1Wins
                p2Wins = msg.p2Wins
                if (currentRound !== msg.currentRound) {
                    roundResetTimer.stop()
                    matchResultTimer.stop()
                    currentRound = msg.currentRound
                    roundEnding = false
                    p1Health = 100
                    p2Health = 100
                    timerSeconds = 60
                    director.resetForNewRound(p1CharId, p2CharId)
                }
                if (msg.roundEnding !== undefined) roundEnding = msg.roundEnding
                if (!roundEnding) {
                    roundResetTimer.stop()
                    matchResultTimer.stop()
                }
                director.p1Model.posXRatio = msg.p1x
                director.p2Model.posXRatio = msg.p2x
                director.updateCamera()
            } else if (msg.type === "round_event" && !isHost) {
                p1Wins = msg.p1Wins
                p2Wins = msg.p2Wins
                roundEnding = true
                if (p1Wins >= 2 || p2Wins >= 2) {
                    matchResultTimer.start()
                } else {
                    roundResetTimer.start()
                }
            } else if (msg.type === "match_end" && !isHost) {
                roundEnding = true
                if (!matchResultTimer.running) matchResultTimer.start()
            }
        }
    }
}
