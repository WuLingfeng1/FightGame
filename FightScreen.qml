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
//     [v0.2.4]     2026-06-25 14:38:16   新增键位修改功能
//     [v0.2.5]     2026-06-25 15:01:36   新增应用程序控制器单例类AppController
//     [v0.2.6]     2026-07-13 15:55:23   新增地图选择功能
import QtQuick
import QtQuick.Controls
import FightGame

Item {
    id: root
    focus: true
    activeFocusOnTab: true

    // 外部传入属性
    property var    stackViewRef: null
    property string p1Name:     ""
    property string p1Avatar:   ""
    property string p1Portrait: ""
    property string p1CharId:   ""
    property string p2Name:     ""
    property string p2Avatar:   ""
    property string p2Portrait: ""
    property string p2CharId:   ""
    property string stageId: "Monaco"
    property bool   isOnline: false
    property bool   isHost: false
    property var    networkMgr: null

    // 游戏状态
    property real fitScale: Math.min(root.width / 900, root.height / 640)
    property real moveStep: 0.008
    property real bodyCollisionDist: 0.15
    property int  p1Health: 100
    property int  p2Health: 100
    property int  timerSeconds: 60
    property int  p1Wins: 0
    property int  p2Wins: 0
    property int  currentRound: 1
    property bool roundEnding: false
    property bool resettingRound: false
    property bool p1Blocking: false
    property bool p2Blocking: false
    property bool isMoving: false
    property bool p1Jumping: false
    property bool p1Attacking: false
    property bool p1Crouching: false
    property bool moveLeftPressed: false
    property bool moveRightPressed: false
    property string p1CurrentAnim: ""
    property bool isMoving2: false
    property bool p2Jumping: false
    property bool p2Attacking: false
    property bool p2Crouching: false
    property bool moveLeft2Pressed: false
    property bool moveRight2Pressed: false
    property string p2CurrentAnim: ""
    property bool p1WaitingCombo: false
    property bool p2WaitingCombo: false
    property bool p1ComboKeyJ: true
    property bool p2ComboKeyJ: true
    property bool p1LightPunchPressed: false
    property bool p1LightKickPressed: false
    property bool p2LightPunchPressed: false
    property bool p2LightKickPressed: false
    property real p1SyncTargetX: 0.0
    property real p2SyncTargetX: 0.0
    property bool syncPosReady: false
    property real p1SyncPrevX: 0.0
    property real p1SyncStep: 0.0
    property int  p1SyncFramesLeft: 0

    // 键位工具
    function getKeyCode(keyName) {
        var keyMap = {
            "A": Qt.Key_A, "B": Qt.Key_B, "C": Qt.Key_C, "D": Qt.Key_D,
            "E": Qt.Key_E, "F": Qt.Key_F, "G": Qt.Key_G, "H": Qt.Key_H,
            "I": Qt.Key_I, "J": Qt.Key_J, "K": Qt.Key_K, "L": Qt.Key_L,
            "M": Qt.Key_M, "N": Qt.Key_N, "O": Qt.Key_O, "P": Qt.Key_P,
            "Q": Qt.Key_Q, "R": Qt.Key_R, "S": Qt.Key_S, "T": Qt.Key_T,
            "U": Qt.Key_U, "V": Qt.Key_V, "W": Qt.Key_W, "X": Qt.Key_X,
            "Y": Qt.Key_Y, "Z": Qt.Key_Z,
            "0": Qt.Key_0, "1": Qt.Key_1, "2": Qt.Key_2, "3": Qt.Key_3,
            "4": Qt.Key_4, "5": Qt.Key_5, "6": Qt.Key_6, "7": Qt.Key_7,
            "8": Qt.Key_8, "9": Qt.Key_9,
            "Left": Qt.Key_Left, "Right": Qt.Key_Right, "Up": Qt.Key_Up, "Down": Qt.Key_Down,
            "Space": Qt.Key_Space, "Return": Qt.Key_Return, "Escape": Qt.Key_Escape,
            "Shift": Qt.Key_Shift, "Control": Qt.Key_Control, "Alt": Qt.Key_Alt,
            "NumPad0": Qt.Key_0, "NumPad1": Qt.Key_1, "NumPad2": Qt.Key_2,
            "NumPad3": Qt.Key_3, "NumPad4": Qt.Key_4, "NumPad5": Qt.Key_5,
            "NumPad6": Qt.Key_6, "NumPad7": Qt.Key_7, "NumPad8": Qt.Key_8, "NumPad9": Qt.Key_9
        }
        return keyMap[keyName] || 0
    }

    function isKeyMatch(event, player, action) {
        var configuredKey = director.keyBindings.getBinding(player, action)
        if (!configuredKey) return false
        var keyCode = getKeyCode(configuredKey)
        if (configuredKey.startsWith("NumPad")) {
            return event.key === keyCode && (event.modifiers & Qt.KeypadModifier)
        }
        return event.key === keyCode
    }

    function sendInput(action, pressed) {
        if (!isOnline || !networkMgr) return
        var msg = {"type": "input", "action": action}
        if (pressed !== undefined) msg.pressed = pressed
        networkMgr.sendMessage(msg)
    }

    // 输入处理
    function handleKeyEvent(event, pressed) {
        var keyAct   = ["moveRight","moveLeft","crouch","jump","lightPunch","lightKick","heavyPunch","heavyKick","heavyStrike","block"]
        var protoAct = ["move_right","move_left","crouch","jump","light_punch","light_kick","heavy_punch","heavy_kick","heavy_strike","block"]
        for (var i = 0; i < keyAct.length; i++) {
            if (isKeyMatch(event, "P1", keyAct[i])) {
                if (pressed || keyAct[i] !== "lightPunch" && keyAct[i] !== "lightKick") {
                    performAction("P1", protoAct[i], pressed)
                    sendInput(protoAct[i], keyAct[i] === "jump" ? undefined : pressed)
                } else {
                    if (keyAct[i] === "lightPunch") p1LightPunchPressed = false
                    else p1LightKickPressed = false
                }
                return
            }
        }
        for (var j = 0; j < keyAct.length; j++) {
            if (isKeyMatch(event, "P2", keyAct[j])) {
                if (pressed || keyAct[j] !== "lightPunch" && keyAct[j] !== "lightKick") {
                    performAction("P2", protoAct[j], pressed)
                    sendInput(protoAct[j], keyAct[j] === "jump" ? undefined : pressed)
                } else {
                    if (keyAct[j] === "lightPunch") p2LightPunchPressed = false
                    else p2LightKickPressed = false
                }
                return
            }
        }
    }

    function performAction(player, action, pressed) {
        var isP1 = (player === "P1")
        if (action === "move_right") {
            if (isP1) { if (pressed) startMoveRight(); else stopMoveRight() }
            else      { if (pressed) startMoveRight2(); else stopMoveRight2() }
        } else if (action === "move_left") {
            if (isP1) { if (pressed) startMoveLeft(); else stopMoveLeft() }
            else      { if (pressed) startMoveLeft2(); else stopMoveLeft2() }
        } else if (action === "crouch") {
            if (isP1) { if (pressed) startCrouch1(); else stopCrouch1() }
            else      { if (pressed) startCrouch2(); else stopCrouch2() }
        } else if (action === "jump") {
            if (isP1) startJump(); else startJump2()
        } else if (action === "light_punch") {
            if (isP1) handleP1LightPunch(); else handleP2LightPunch()
        } else if (action === "light_kick") {
            if (isP1) handleP1LightKick(); else handleP2LightKick()
        } else if (action === "heavy_punch") {
            if (isP1) startAttackHeavyPunch1(); else startAttackHeavyPunch2()
        } else if (action === "heavy_kick") {
            if (isP1) startAttackHeavyKick1(); else startAttackHeavyKick2()
        } else if (action === "heavy_strike") {
            if (isP1) startAttackHeavyStrike1(); else startAttackHeavyStrike2()
        } else if (action === "block") {
            if (pressed) {
                if (isP1) {
                    p1Blocking = true
                    if (!p1Jumping && !p1Attacking && !p1Crouching) {
                        director.p1Model.playStandBlock()
                        moveTimer.stop()
                        isMoving = false
                    }
                } else {
                    p2Blocking = true
                    if (!p2Jumping && !p2Attacking && !p2Crouching) {
                        director.p2Model.playStandBlock()
                        moveTimer2.stop()
                        isMoving2 = false
                    }
                }
            } else {
                if (isP1) {
                    p1Blocking = false
                    if (director.p1Model.state === 16 || director.p1Model.state === 12 || director.p1Model.state === 17) director.p1Model.releaseStandBlock()
                } else {
                    p2Blocking = false
                    if (director.p2Model.state === 16 || director.p2Model.state === 12 || director.p2Model.state === 17) director.p2Model.releaseStandBlock()
                }
            }
        } else if (action === "dodge_forward") {
            if (isP1) startDodge1(true); else startDodge2(true)
        } else if (action === "dodge_backward") {
            if (isP1) startDodge1(false); else startDodge2(false)
        }
    }

    function handleP1LightPunch() {
        p1LightPunchPressed = true
        if (p1WaitingCombo && p1LightKickPressed && (moveRightPressed || moveLeftPressed)) {
            p1ComboTimer.stop()
            p1WaitingCombo = false
            startDodge1(getP1DodgeForward())
        } else {
            p1ComboKeyJ = true
            p1WaitingCombo = true
            p1ComboTimer.restart()
        }
    }
    function handleP1LightKick() {
        p1LightKickPressed = true
        if (p1WaitingCombo && p1LightPunchPressed && (moveRightPressed || moveLeftPressed)) {
            p1ComboTimer.stop()
            p1WaitingCombo = false
            startDodge1(getP1DodgeForward())
        } else {
            p1ComboKeyJ = false
            p1WaitingCombo = true
            p1ComboTimer.restart()
        }
    }
    function handleP2LightPunch() {
        p2LightPunchPressed = true
        var dir1 = moveRight2Pressed || moveLeft2Pressed
        if (p2WaitingCombo && p2LightKickPressed) {
            p2ComboTimer.stop()
            p2WaitingCombo = false
            startDodge2(dir1 ? getP2DodgeForward() : true)
        } else {
            p2ComboKeyJ = true
            p2WaitingCombo = true
            p2ComboTimer.restart()
        }
    }
    function handleP2LightKick() {
        p2LightKickPressed = true
        var dir2 = moveRight2Pressed || moveLeft2Pressed
        if (p2WaitingCombo && p2LightPunchPressed) {
            p2ComboTimer.stop()
            p2WaitingCombo = false
            startDodge2(dir2 ? getP2DodgeForward() : true)
        } else {
            p2ComboKeyJ = false
            p2WaitingCombo = true
            p2ComboTimer.restart()
        }
    }

    function handleRemoteInput(action, pressed) {
        if (isHost) {
            p2Jumping = (director.p2Model.state === 5 || director.p2Model.state === 6)
            var p2state = director.p2Model.state
            p2Attacking = (p2state >= 7 && p2state <= 12) || p2state === 14 || p2state === 15
            p2Blocking = (p2state === 16)
            p2Crouching = (p2state === 13)
            performAction("P2", action, pressed)
        } else {
            p1Jumping = (director.p1Model.state === 5 || director.p1Model.state === 6)
            var p1state = director.p1Model.state
            p1Attacking = (p1state >= 7 && p1state <= 12) || p1state === 14 || p1state === 15
            p1Blocking = (p1state === 16)
            p1Crouching = (p1state === 13)
            performAction("P1", action, pressed)
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

    // 闪避方向
    function getP1DodgeForward() {
        var fwd = !director.p1Model.facingLeft
        if (moveRightPressed && !moveLeftPressed) return fwd
        else if (moveLeftPressed && !moveRightPressed) return !fwd
        return true
    }

    function getP2DodgeForward() {
        var fwd = !director.p2Model.facingLeft
        if (moveRight2Pressed && !moveLeft2Pressed) return fwd
        else if (moveLeft2Pressed && !moveRight2Pressed) return !fwd
        return true
    }

    // P1 移动
    function startMoveRight() {
        moveRightPressed = true
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16 || director.p1Model.state === 12 || director.p1Model.state === 17) return
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
    function startMoveLeft() {
        moveLeftPressed = true
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16 || director.p1Model.state === 12 || director.p1Model.state === 17) return
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
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16 || director.p1Model.state === 12 || director.p1Model.state === 17) return
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
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16 || director.p1Model.state === 12 || director.p1Model.state === 17) return
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

    // P1 跳跃
    function startJump() {
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16 || director.p1Model.state === 12 || director.p1Model.state === 17) return
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

    // P1 攻击
    function startAttack1() {
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16 || director.p1Model.state === 12 || director.p1Model.state === 17) return
        p1Attacking = true
        moveTimer.moveRight = false
        moveTimer.moveLeft = false
        moveTimer.stop()
        isMoving = false
        p1CurrentAnim = ""
        director.p1Model.playLightPunch()
    }
    function startAttackLightKick1() {
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16 || director.p1Model.state === 12 || director.p1Model.state === 17) return
        p1Attacking = true
        moveTimer.moveRight = false
        moveTimer.moveLeft = false
        moveTimer.stop()
        isMoving = false
        p1CurrentAnim = ""
        director.p1Model.playLightKick()
    }
    function startAttackHeavyPunch1() {
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16 || director.p1Model.state === 12 || director.p1Model.state === 17) return
        p1Attacking = true
        moveTimer.moveRight = false
        moveTimer.moveLeft = false
        moveTimer.stop()
        isMoving = false
        p1CurrentAnim = ""
        director.p1Model.playHeavyPunch()
    }
    function startAttackHeavyKick1() {
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16 || director.p1Model.state === 12 || director.p1Model.state === 17) return
        p1Attacking = true
        moveTimer.moveRight = false
        moveTimer.moveLeft = false
        moveTimer.stop()
        isMoving = false
        p1CurrentAnim = ""
        director.p1Model.playHeavyKick()
    }
    function startAttackHeavyStrike1() {
        if (p1Jumping || p1Attacking || p1Crouching || p1Blocking || director.p1Model.state === 16 || director.p1Model.state === 12 || director.p1Model.state === 17) return
        p1Attacking = true
        moveTimer.moveRight = false
        moveTimer.moveLeft = false
        moveTimer.stop()
        isMoving = false
        p1CurrentAnim = ""
        director.p1Model.playHeavyStrike()
    }
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
        if (p1Jumping || p1Attacking || p1Blocking || director.p1Model.state === 16 || director.p1Model.state === 12 || director.p1Model.state === 17) return
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
    function stopCrouch1() {
        p1Crouching = false
        director.p1Model.stopCrouch()
        if (moveRightPressed && !moveLeftPressed) startMoveRight()
        else if (moveLeftPressed && !moveRightPressed) startMoveLeft()
    }

    // P2 移动
    function startMoveRight2() {
        moveRight2Pressed = true
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16 || director.p2Model.state === 12 || director.p2Model.state === 17) return
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
    function startMoveLeft2() {
        moveLeft2Pressed = true
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16 || director.p2Model.state === 12 || director.p2Model.state === 17) return
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
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16 || director.p2Model.state === 12 || director.p2Model.state === 17) return
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
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16 || director.p2Model.state === 12 || director.p2Model.state === 17) return
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

    // P2 跳跃
    function startJump2() {
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16 || director.p2Model.state === 12 || director.p2Model.state === 17) return
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

    // P2 攻击
    function startAttack2() {
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16 || director.p2Model.state === 12 || director.p2Model.state === 17) return
        p2Attacking = true
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = false
        moveTimer2.stop()
        isMoving2 = false
        p2CurrentAnim = ""
        director.p2Model.playLightPunch()
    }
    function startAttackLightKick2() {
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16 || director.p2Model.state === 12 || director.p2Model.state === 17) return
        p2Attacking = true
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = false
        moveTimer2.stop()
        isMoving2 = false
        p2CurrentAnim = ""
        director.p2Model.playLightKick()
    }
    function startAttackHeavyPunch2() {
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16 || director.p2Model.state === 12 || director.p2Model.state === 17) return
        p2Attacking = true
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = false
        moveTimer2.stop()
        isMoving2 = false
        p2CurrentAnim = ""
        director.p2Model.playHeavyPunch()
    }
    function startAttackHeavyKick2() {
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16 || director.p2Model.state === 12 || director.p2Model.state === 17) return
        p2Attacking = true
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = false
        moveTimer2.stop()
        isMoving2 = false
        p2CurrentAnim = ""
        director.p2Model.playHeavyKick()
    }
    function startAttackHeavyStrike2() {
        if (p2Jumping || p2Attacking || p2Crouching || p2Blocking || director.p2Model.state === 16 || director.p2Model.state === 12 || director.p2Model.state === 17) return
        p2Attacking = true
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = false
        moveTimer2.stop()
        isMoving2 = false
        p2CurrentAnim = ""
        director.p2Model.playHeavyStrike()
    }
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
        if (p2Jumping || p2Attacking || p2Blocking || director.p2Model.state === 16 || director.p2Model.state === 12 || director.p2Model.state === 17) return
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
    function stopCrouch2() {
        p2Crouching = false
        director.p2Model.stopCrouch()
        if (moveRight2Pressed && !moveLeft2Pressed) startMoveRight2()
        else if (moveLeft2Pressed && !moveRight2Pressed) startMoveLeft2()
    }

    // 回合逻辑
    function updateFacing() {
        director.p1Model.facingLeft = (director.p1Model.posXRatio > director.p2Model.posXRatio)
        director.p2Model.facingLeft = (director.p2Model.posXRatio > director.p1Model.posXRatio)
    }

    function checkRoundEnd() {
        if (roundEnding) return
        if (p1Health <= 0 || p2Health <= 0 || timerSeconds <= 0) {
            roundEnding = true

            moveTimer.stop()
            moveTimer.moveRight = false
            moveTimer.moveLeft = false
            isMoving = false

            moveTimer2.stop()
            moveTimer2.moveRight = false
            moveTimer2.moveLeft = false
            isMoving2 = false

            director.p1Model.playStand()
            director.p2Model.playStand()

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

    function resetRound() {
        if (resettingRound || roundEnding === false) return
        resettingRound = true
        currentRound++
        p1Health = 100
        p2Health = 100
        timerSeconds = 60
        roundEnding = false
        director.resetForNewRound(p1CharId, p2CharId)

        moveTimer.stop()
        moveTimer.moveRight = false
        moveTimer.moveLeft = false
        isMoving = false
        p1Jumping = false
        p1Attacking = false
        p1Crouching = false
        p1Blocking = false
        moveLeftPressed = false
        moveRightPressed = false
        p1CurrentAnim = ""

        moveTimer2.stop()
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = false
        isMoving2 = false
        p2Jumping = false
        p2Attacking = false
        p2Crouching = false
        p2Blocking = false
        moveLeft2Pressed = false
        moveRight2Pressed = false
        p2CurrentAnim = ""

        resettingRound = false
    }

    function resumeP1Movement() {
        updateFacing()
        director.updateCamera()
        if (p1Crouching || p1Blocking) return
        if (director.p1Model.state === 12 || director.p1Model.state === 17) return
        if (moveRightPressed && !moveLeftPressed) {
            isMoving = true
            moveTimer.moveLeft = false
            moveTimer.moveRight = true
            if (director.p1Model.facingLeft) {
                director.p1Model.playBackward(); p1CurrentAnim = "backward"
            } else {
                director.p1Model.playForward();  p1CurrentAnim = "forward"
            }
            moveTimer.start()
        } else if (moveLeftPressed && !moveRightPressed) {
            isMoving = true
            moveTimer.moveRight = false
            moveTimer.moveLeft = true
            if (director.p1Model.facingLeft) {
                director.p1Model.playForward();  p1CurrentAnim = "forward"
            } else {
                director.p1Model.playBackward(); p1CurrentAnim = "backward"
            }
            moveTimer.start()
        } else if (moveRightPressed || moveLeftPressed) {
            isMoving = true
            moveTimer.moveRight = moveRightPressed
            moveTimer.moveLeft = moveLeftPressed
            if (director.p1Model.facingLeft) {
                director.p1Model.playBackward(); p1CurrentAnim = "backward"
            } else {
                director.p1Model.playForward();  p1CurrentAnim = "forward"
            }
            moveTimer.start()
        } else {
            director.p1Model.playStand()
            moveTimer.moveRight = false
            moveTimer.moveLeft = false
            isMoving = false
        }
    }

    function resumeP2Movement() {
        updateFacing()
        director.updateCamera()
        if (p2Crouching || p2Blocking) return
        if (director.p2Model.state === 12 || director.p2Model.state === 17) return
        if (moveRight2Pressed && !moveLeft2Pressed) {
            isMoving2 = true
            moveTimer2.moveLeft = false
            moveTimer2.moveRight = true
            if (director.p2Model.facingLeft) {
                director.p2Model.playBackward(); p2CurrentAnim = "backward"
            } else {
                director.p2Model.playForward();  p2CurrentAnim = "forward"
            }
            moveTimer2.start()
        } else if (moveLeft2Pressed && !moveRight2Pressed) {
            isMoving2 = true
            moveTimer2.moveRight = false
            moveTimer2.moveLeft = true
            if (director.p2Model.facingLeft) {
                director.p2Model.playForward();  p2CurrentAnim = "forward"
            } else {
                director.p2Model.playBackward(); p2CurrentAnim = "backward"
            }
            moveTimer2.start()
        } else if (moveRight2Pressed || moveLeft2Pressed) {
            isMoving2 = true
            moveTimer2.moveRight = moveRight2Pressed
            moveTimer2.moveLeft = moveLeft2Pressed
            if (director.p2Model.facingLeft) {
                director.p2Model.playBackward(); p2CurrentAnim = "backward"
            } else {
                director.p2Model.playForward();  p2CurrentAnim = "forward"
            }
            moveTimer2.start()
        } else {
            director.p2Model.playStand()
            moveTimer2.moveRight = false
            moveTimer2.moveLeft = false
            isMoving2 = false
        }
    }

    // 战斗导演
    FightDirector {
        id: director
        rootHeight: root.height
        onHitDetected: function(attacker, damage) {
            function playHurtByAttackType(targetModel, attackerState) {
                if (attackerState === 7 || attackerState === 8) {
                    targetModel.playHurt1()
                } else if (attackerState === 9 || attackerState === 10) {
                    targetModel.playHurt2()
                } else if (attackerState === 11) {
                    targetModel.playHurt3()
                } else {
                    targetModel.playHurt()
                }
            }

            if (attacker === 1) {
                p2Health = Math.max(0, p2Health - damage)
                if (!director.p2Model.isBlocking()) {
                    playHurtByAttackType(director.p2Model, director.p1Model.state)
                }
                if (director.p1Model.state >= 7 && director.p1Model.state <= 11)
                    attackVfx.play(director.p1Model, director.p2Model)
            } else {
                p1Health = Math.max(0, p1Health - damage)
                if (!director.p1Model.isBlocking()) {
                    playHurtByAttackType(director.p1Model, director.p2Model.state)
                }
                if (director.p2Model.state >= 7 && director.p2Model.state <= 11)
                    attackVfx.play(director.p2Model, director.p1Model)
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

    // 定时器
    Timer {
        id: collisionTimer
        interval: 33
        repeat: true
        running: director.phase === FightDirector.Fighting && !resettingRound && (!isOnline || isHost)
        onTriggered: {
            director.checkCollision()
        }
    }

    Timer {
        id: countdownTimer
        interval: 1000
        repeat: true
        // running: director.phase === FightDirector.Fighting && !roundEnding && !resettingRound
        running: false  // DEBUG: 计时器已停
        onTriggered: {
            if (timerSeconds > 0) timerSeconds--
            if (timerSeconds <= 0) checkRoundEnd()
        }
    }

    Timer {
        id: syncTimer
        interval: 33
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

    Timer {
        id: posInterpTimer
        interval: 16
        repeat: true
        running: isOnline && !isHost && syncPosReady && director.phase === FightDirector.Fighting && !resettingRound
        onTriggered: {
            if (p1SyncFramesLeft > 0) {
                director.p1Model.posXRatio += p1SyncStep
                p1SyncFramesLeft--
            }
            director.p1Model.posXRatio += (p1SyncTargetX - director.p1Model.posXRatio) * 0.05
            director.p2Model.posXRatio += (p2SyncTargetX - director.p2Model.posXRatio) * 0.03
            director.updateCamera()
        }
    }

    Timer {
        id: moveTimer
        interval: 16
        repeat: true
        running: false
        property bool moveRight: false
        property bool moveLeft: false
        onTriggered: {
            if (isOnline && !isHost) return
            if (director.p1Model.state === 12 || director.p1Model.state === 17) return
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

    Timer {
        id: moveTimer2
        interval: 16
        repeat: true
        running: false
        property bool moveRight: false
        property bool moveLeft: false
        onTriggered: {
            if (director.p2Model.state === 12 || director.p2Model.state === 17) return
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

    Timer {
        id: p1ComboTimer
        interval: 100
        onTriggered: {
            if (p1WaitingCombo) {
                p1WaitingCombo = false
                if (p1ComboKeyJ) {
                    startAttack1()
                    if (!isOnline || isHost) sendInput("light_punch")
                } else {
                    startAttackLightKick1()
                    if (!isOnline || isHost) sendInput("light_kick")
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
                    if (!isOnline || !isHost) sendInput("light_punch")
                } else {
                    startAttackLightKick2()
                    if (!isOnline || !isHost) sendInput("light_kick")
                }
            }
        }
    }

    Timer {
        id: roundResetTimer
        interval: 2000
        onTriggered: resetRound()
    }

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

    // 键盘输入
    Keys.onPressed: (event) => {
        if (event.isAutoRepeat || director.phase !== FightDirector.Fighting || resettingRound || roundEnding) return
        if (isOnline) {
            var isP1Key = isKeyMatch(event, "P1", "moveLeft")
                       || isKeyMatch(event, "P1", "moveRight")
                       || isKeyMatch(event, "P1", "jump")
                       || isKeyMatch(event, "P1", "crouch")
                       || isKeyMatch(event, "P1", "lightPunch")
                       || isKeyMatch(event, "P1", "lightKick")
                       || isKeyMatch(event, "P1", "heavyPunch")
                       || isKeyMatch(event, "P1", "heavyKick")
                       || isKeyMatch(event, "P1", "heavyStrike")
                       || isKeyMatch(event, "P1", "block")
            if (isHost && !isP1Key) return
            if (!isHost && isP1Key) return
        }
        handleKeyEvent(event, true)
    }
    Keys.onReleased: (event) => {
        if (event.isAutoRepeat || director.phase !== FightDirector.Fighting || resettingRound || roundEnding) return
        if (isOnline) {
            var isP1Key = isKeyMatch(event, "P1", "moveLeft")
                       || isKeyMatch(event, "P1", "moveRight")
                       || isKeyMatch(event, "P1", "jump")
                       || isKeyMatch(event, "P1", "crouch")
                       || isKeyMatch(event, "P1", "lightPunch")
                       || isKeyMatch(event, "P1", "lightKick")
                       || isKeyMatch(event, "P1", "heavyPunch")
                       || isKeyMatch(event, "P1", "heavyKick")
                       || isKeyMatch(event, "P1", "heavyStrike")
                       || isKeyMatch(event, "P1", "block")
            if (isHost && !isP1Key) return
            if (!isHost && isP1Key) return
        }
        handleKeyEvent(event, false)
    }

    // 背景
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
            id: stageGif
            width: parent.width * 2
            height: parent.height
            source: "qrc:/images/FightBackGround/" + stageId + ".gif"
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

    // 角色渲染
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

    // 攻击特效
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

    // HUD
    Rectangle {
        id: hudBar
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: 82
        color: "black"
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

    // P1 HUD
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

    // 倒计时
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

    // 回合与联机标识
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

    // P2 HUD
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

    // 返回按钮
    Button {
        anchors {
            left: parent.left
            bottom: parent.bottom
            margins: 12
        }
        width: 80
        height: 28
        z: 10
        text: "BACK"
        onClicked: { if (stackViewRef) stackViewRef.pop() }

        contentItem: Text {
            text: "BACK"
            color: "darkgoldenrod"
            font.pixelSize: 11
            font.family: "monospace"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            color: "black"
            border.color: "darkgoldenrod"
            border.width: 1
            radius: 2
        }
    }

    // 生命周期
    Component.onCompleted: {
        director.reloadKeyBindings()
        director.start(p1CharId, p2CharId)
        root.forceActiveFocus()
    }

    Component.onDestruction: {
        collisionTimer.stop()
        countdownTimer.stop()
        moveTimer.stop()
        moveTimer2.stop()
        p1ComboTimer.stop()
        p2ComboTimer.stop()
        vfxTimer.stop()
        roundResetTimer.stop()
        matchResultTimer.stop()
        director.p1Model.playStand()
        director.p2Model.playStand()
    }

    // 信号连接
    Connections {
        target: director.p1Model
        function onJumpFinished() { p1Jumping = false; resumeP1Movement() }
        function onAttackFinished() { p1Attacking = false; resumeP1Movement() }
        function onHurtFinished() { p1Attacking = false; p1Jumping = false; p1Crouching = false; resumeP1Movement() }
        function onDodgeFinished() { p1Attacking = false; resumeP1Movement() }
        function onPosXRatioChanged() { director.updateCamera() }
    }

    Connections {
        target: director.p2Model
        function onJumpFinished() { p2Jumping = false; resumeP2Movement() }
        function onAttackFinished() { p2Attacking = false; resumeP2Movement() }
        function onHurtFinished() { p2Attacking = false; p2Jumping = false; p2Crouching = false; resumeP2Movement() }
        function onDodgeFinished() { p2Attacking = false; resumeP2Movement() }
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
                    } else if (atkState === 9 || atkState === 10) {
                        targetModel.playHurt2()
                    } else if (atkState === 11) {
                        targetModel.playHurt3()
                    } else {
                        targetModel.playHurt()
                    }
                }
                if (msg.attacker === 1) {
                    if (!msg.defenderBlocking) {
                        playHurt(director.p2Model, attackerState)
                    }
                    if (attackerState >= 7 && attackerState <= 11)
                        attackVfx.play(director.p1Model, director.p2Model)
                } else {
                    if (!msg.defenderBlocking) {
                        playHurt(director.p1Model, attackerState)
                    }
                    if (attackerState >= 7 && attackerState <= 11)
                        attackVfx.play(director.p2Model, director.p1Model)
                }
                checkRoundEnd()
            } else if (msg.type === "sync" && !isHost) {
                p1Health = msg.p1Health
                p2Health = msg.p2Health
                timerSeconds = msg.timerSeconds
                p1Wins = msg.p1Wins
                p2Wins = msg.p2Wins
                if (currentRound !== msg.currentRound) {
                    syncPosReady = false
                    roundResetTimer.stop()
                    matchResultTimer.stop()
                    currentRound = msg.currentRound
                    roundEnding = false
                    p1Health = 100
                    p2Health = 100
                    timerSeconds = 60
                    director.resetForNewRound(p1CharId, p2CharId)

                    moveTimer.stop()
                    moveTimer.moveRight = false
                    moveTimer.moveLeft = false
                    isMoving = false
                    p1Jumping = false
                    p1Attacking = false
                    p1Crouching = false
                    p1Blocking = false
                    moveLeftPressed = false
                    moveRightPressed = false
                    p1CurrentAnim = ""

                    moveTimer2.stop()
                    moveTimer2.moveRight = false
                    moveTimer2.moveLeft = false
                    isMoving2 = false
                    p2Jumping = false
                    p2Attacking = false
                    p2Crouching = false
                    p2Blocking = false
                    moveLeft2Pressed = false
                    moveRight2Pressed = false
                    p2CurrentAnim = ""
                }
                if (msg.roundEnding !== undefined) roundEnding = msg.roundEnding
                if (!roundEnding) {
                    roundResetTimer.stop()
                    matchResultTimer.stop()
                }
                p1SyncTargetX = msg.p1x
                p2SyncTargetX = msg.p2x
                if (!syncPosReady) {
                    director.p1Model.posXRatio = msg.p1x
                    director.p2Model.posXRatio = msg.p2x
                    p1SyncPrevX = msg.p1x
                    syncPosReady = true
                } else {
                    var delta = msg.p1x - p1SyncPrevX
                    p1SyncStep = delta / 2.0
                    p1SyncFramesLeft = 2
                    p1SyncPrevX = msg.p1x
                }
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
