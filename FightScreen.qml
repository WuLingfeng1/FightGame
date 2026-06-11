// Module
// File: FightScreen.qml   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-10 14:56:24
// Description:
//     Created the Fight screen
// Change Log:
//     [v0.1.1]     2026-06-11 22:43:30
//         * Added character intro animations
import QtQuick
import QtQuick.Controls
import FightGame

Item {
    id: root

    // 入口属性（由 SelectScreen 传入）
    property var    stackViewRef: null
    property string p1Name:     ""
    property string p1Avatar:   ""
    property string p1Portrait: ""
    property string p1CharId:   ""
    property string p2Name:     ""
    property string p2Avatar:   ""
    property string p2Portrait: ""
    property string p2CharId:   ""

    FighterModel { id: p1Model }
    FighterModel { id: p2Model }

    // Orochi（P2 / 右侧 / char1）
    property int fw: 384           // 开场帧宽
    property int fh: 512           // 开场帧高
    property int openingFrames: 70 // 开场总帧数
    property int standFrames: 27   // 待机总帧数
    property int stand1Fw: 251     // 待机帧宽
    property int stand1Fh: 335     // 待机帧高

    // Yagami（P1 / 左侧 / char2）
    property int opening2Frames: 12 // 开场总帧数
    property int stand2Frames: 9    // 待机总帧数
    property int stand2Fw: 256      // 统一帧宽
    property int stand2Fh: 342      // 统一帧高

    // 状态机 — Yagami 先开场 → 完毕后 Orochi 再开场 → 双方待机
    property string state1: "waiting"   // Orochi: waiting | opening | stand
    property string state2: "opening2"  // Yagami: opening2 | stand2

    // 自适应缩放 — 以设计尺寸 900×640 为基准
    property real fitScale: Math.min(root.width / 900, root.height / 640)

    // 统一动画驱动 — 单 Timer + tick 计数器
    Timer {
        id: animTimer
        interval: 50      // 20 ticks/秒
        repeat: true
        running: false
        property int frame1: 0   // Orochi 当前帧
        property int frame2: 0   // Yagami 当前帧
        property int tick: 0     // 全局 tick 计数

        onTriggered: {
            tick++

            // ── Orochi ──
            if (state1 === "opening") {
                char1Img.x = -frame1 * fw
                frame1++
                if (frame1 >= openingFrames) {
                    state1 = "stand"
                    frame1 = 2
                    char1Img.source = "qrc:/images/character/" + p2CharId + "/Stand.png"
                }
            } else if (state1 === "stand") {
                if (tick % 2 === 0) {           // 每 2 tick 推进 (100ms/帧 ≈ 10 FPS)
                    char1Img.x = -frame1 * stand1Fw
                    frame1++
                    if (frame1 >= standFrames)
                        frame1 = 0
                }
            }

            // ── Yagami ──
            if (state2 === "opening2") {
                if (tick % 2 === 0) {           // 每 2 tick 推进 (100ms/帧 ≈ 10 FPS)
                    char2Img.x = -frame2 * stand2Fw
                    frame2++
                    if (frame2 >= opening2Frames) {
                        state2 = "stand2"
                        frame2 = 0
                        char2Img.source = "qrc:/images/character/" + p1CharId + "/Stand.png"
                        state1 = "opening"       // 触发 Orochi 开场
                    }
                }
            } else if (state2 === "stand2") {
                if (tick % 3 === 0) {           // 每 3 tick 推进 (150ms/帧 ≈ 7 FPS)
                    char2Img.x = -frame2 * stand2Fw
                    frame2++
                    if (frame2 >= stand2Frames)
                        frame2 = 0
                }
            }
        }
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
            x: Math.min(0, Math.max(-(width - parent.width), 0))
            y: 0
        }
    }

    // Orochi 精灵层（P2 / 右侧）
    property int feetMarginOrochi: 82   // 脚底到屏幕底部的固定距离

    Item {
        id: char1Wrapper
        width: (state1 === "opening" || state1 === "waiting") ? fw : stand1Fw
        height: (state1 === "opening" || state1 === "waiting") ? fh : stand1Fh
        x: root.width * 0.78 - width / 2
        y: (state1 === "opening" || state1 === "waiting")
            ? root.height - fh - 60 * fitScale
            : root.height - feetMarginOrochi * fitScale - 327
        scale: fitScale
        transformOrigin: Item.Bottom
        clip: true
        z: 1

        Image {
            id: char1Img
            source: "qrc:/images/character/Orochi/opening.png"
            width: (state1 === "opening" || state1 === "waiting")
                ? openingFrames * fw
                : standFrames * stand1Fw
            height: (state1 === "opening" || state1 === "waiting") ? fh : stand1Fh
            x: 0
            y: 0
            fillMode: Image.Stretch
            smooth: false
            mipmap: false
            cache: true
            asynchronous: false
            mirror: true
        }
    }

    // Yagami 精灵层（P1 / 左侧）
    Item {
        id: char2Wrapper
        width: stand2Fw
        height: stand2Fh
        x: root.width * 0.22 - width / 2
        y: root.height - stand2Fh - 60 * fitScale
        scale: fitScale
        transformOrigin: Item.Bottom
        clip: true
        z: 1

        Image {
            id: char2Img
            source: "qrc:/images/character/Yagami/opening.png"
            width: state2 === "opening2"
                ? opening2Frames * stand2Fw
                : stand2Frames * stand2Fw
            height: stand2Fh
            x: 0
            y: 0
            fillMode: Image.Stretch
            smooth: false
            mipmap: false
            cache: true
            asynchronous: false
        }
    }

    // HUD 栏（血条 / 能量 / 计时器 / 头像 / 名字）
    Rectangle {
        id: hudBar
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 82
        color: Qt.rgba(0, 0, 0, 0.85)
        z: 10

        Rectangle {
            anchors { top: parent.top; left: parent.left; right: parent.right }
            height: 2
            color: "darkgoldenrod"
        }
        Rectangle {
            anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
            height: 2
            color: "darkgoldenrod"
        }
    }

    // P1 HUD
    Item {
        anchors { left: parent.left; leftMargin: 12; top: parent.top; topMargin: 8 }
        width: 340
        height: 76
        z: 11

        Rectangle {
            id: p1PortraitFrame
            anchors { left: parent.left; verticalCenter: parent.verticalCenter }
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
            anchors { left: p1PortraitFrame.right; leftMargin: 10; verticalCenter: parent.verticalCenter }
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
                width: 240; height: 16
                color: "black"
                border.color: "darkgoldenrod"
                border.width: 1
                radius: 1

                Rectangle {
                    anchors { left: parent.left; leftMargin: 1; verticalCenter: parent.verticalCenter }
                    width: Math.max(0, (parent.width - 2) * (p1Health / 100.0))
                    height: parent.height - 2
                    color: "firebrick"
                    radius: 1

                    Rectangle {
                        anchors { top: parent.top; bottom: parent.bottom; right: parent.right }
                        width: 6
                        color: "gold"
                        visible: p1Health > 0
                    }
                    Behavior on width { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }
                }
            }

            // 能量条
            Rectangle {
                width: 240; height: 8
                color: "black"
                border.color: "darkslateblue"
                border.width: 1
                radius: 1

                Rectangle {
                    anchors { left: parent.left; leftMargin: 1; verticalCenter: parent.verticalCenter }
                    width: Math.max(0, (parent.width - 2) * (p1Energy / 100.0))
                    height: parent.height - 2
                    color: "royalblue"
                    radius: 1
                    Behavior on width { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
                }
            }
        }
    }

    // 计时器
    Rectangle {
        anchors { centerIn: hudBar; verticalCenterOffset: 2 }
        width: 52; height: 52
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

    // P2 HUD
    Item {
        anchors { right: parent.right; rightMargin: 12; top: parent.top; topMargin: 8 }
        width: 340
        height: 76
        z: 11

        Rectangle {
            id: p2PortraitFrame
            anchors { right: parent.right; verticalCenter: parent.verticalCenter }
            width: 54; height: 54
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
            anchors { right: p2PortraitFrame.left; rightMargin: 10; verticalCenter: parent.verticalCenter }
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
                width: 240; height: 16
                color: "black"
                border.color: "darkgoldenrod"
                border.width: 1
                radius: 1

                Rectangle {
                    anchors { right: parent.right; rightMargin: 1; verticalCenter: parent.verticalCenter }
                    width: Math.max(0, (parent.width - 2) * (p2Health / 100.0))
                    height: parent.height - 2
                    color: "firebrick"
                    radius: 1

                    Rectangle {
                        anchors { top: parent.top; bottom: parent.bottom; left: parent.left }
                        width: 6
                        color: "gold"
                        visible: p2Health > 0
                    }
                    Behavior on width { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }
                }
            }

            // 能量条
            Rectangle {
                width: 240; height: 8
                color: "black"
                border.color: "darkslateblue"
                border.width: 1
                radius: 1

                Rectangle {
                    anchors { right: parent.right; rightMargin: 1; verticalCenter: parent.verticalCenter }
                    width: Math.max(0, (parent.width - 2) * (p2Energy / 100.0))
                    height: parent.height - 2
                    color: "royalblue"
                    radius: 1
                    Behavior on width { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
                }
            }
        }
    }

    // 标题
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 10
        text: "K.O.F. '97"
        font.pixelSize: 8
        font.bold: true
        color: "darkgoldenrod"
        font.family: "monospace"
        z: 12
    }

    // 游戏状态
    property int p1Health: 100
    property int p2Health: 100
    property int p1Energy: 0
    property int p2Energy: 0
    property int timerSeconds: 60

    // 返回按钮
    Button {
        anchors { left: parent.left; bottom: parent.bottom; margins: 12 }
        width: 80
        height: 28
        z: 10
        text: "<- BACK"
        onClicked: {
            animTimer.stop()
            if (stackViewRef) stackViewRef.pop()
        }
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

    // 调试信息
    Rectangle {
        anchors { right: parent.right; bottom: parent.bottom; margins: 12 }
        width: debugText.implicitWidth + 16
        height: 22
        color: Qt.rgba(0, 0, 0, 0.6)
        radius: 2
        z: 12

        Text {
            id: debugText
            anchors.centerIn: parent
            text: state1 + "(f" + animTimer.frame1 + ")"
                  + " | " + state2 + "(f" + animTimer.frame2 + ")"
                  + " tick:" + animTimer.tick
            color: "darkgreen"
            font.pixelSize: 10
            font.family: "monospace"
        }
    }

    // 启动
    Component.onCompleted: {
        animTimer.start()
    }
}
