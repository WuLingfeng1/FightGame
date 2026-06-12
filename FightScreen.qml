// Module
// File: FightScreen.qml   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-10 14:56:24
// Description:
//     Created the Fight screen
// Change Log:
//     [v0.1.1]     2026-06-11 22:43:30
//         * Added character intro animations
// Change Log:
//     [v0.1.2]     2026-06-12 13:54:47
//         * 分离了渲染与功能实现,同时也增加了forward动作,最后微调了一下人物的单帧尺寸使动作衔接更加流畅
import QtQuick
import QtQuick.Controls
import FightGame

// FightScreen.qml — 纯渲染层, 动画逻辑在 C++ CharacterModel / FightDirector
Item {
    id: root
    focus: true
    activeFocusOnTab: true

    property var    stackViewRef: null
    property string p1Name:     ""
    property string p1Avatar:   ""
    property string p1Portrait: ""
    property string p1CharId:   ""
    property string p2Name:     ""
    property string p2Avatar:   ""
    property string p2Portrait: ""
    property string p2CharId:   ""

    FightDirector {
        id: director
        rootHeight: root.height
    }

    property real fitScale: Math.min(root.width / 900, root.height / 640)
    property real moveStep: 0.003

    // 连续移动用 Timer
    Timer {
        id: moveTimer
        interval: 16
        repeat: true
        running: false
        property bool moveRight: false
        property bool moveLeft: false
        onTriggered: {
            if (moveRight) director.p1Model.posXRatio += moveStep
            if (moveLeft)  director.p1Model.posXRatio -= moveStep
        }
    }

    property bool isMoving: false
    property bool isMoving2: false

    property bool moveLeftPressed: false
    property bool moveRightPressed: false
    property bool moveLeft2Pressed: false
    property bool moveRight2Pressed: false

    function startMoveRight() {
        director.p1Model.facingLeft = false
        moveRightPressed = true
        moveTimer.moveLeft = false
        moveTimer.moveRight = true
        if (!isMoving) {
            isMoving = true
            director.p1Model.playForward()
            moveTimer.start()
        }
    }
    function startMoveLeft() {
        director.p1Model.facingLeft = true
        moveLeftPressed = true
        moveTimer.moveRight = false
        moveTimer.moveLeft = true
        if (!isMoving) {
            isMoving = true
            director.p1Model.playForward()
            moveTimer.start()
        }
    }
    function stopMoveRight() {
        moveRightPressed = false
        if (moveLeftPressed) {
            director.p1Model.facingLeft = true
            moveTimer.moveRight = false
            moveTimer.moveLeft = true
        } else {
            moveTimer.stop()
            moveTimer.moveRight = false
            moveTimer.moveLeft = false
            isMoving = false
            director.p1Model.playStand()
            director.p1Model.facingLeft = false
        }
    }
    function stopMoveLeft() {
        moveLeftPressed = false
        if (moveRightPressed) {
            director.p1Model.facingLeft = false
            moveTimer.moveLeft = false
            moveTimer.moveRight = true
        } else {
            moveTimer.stop()
            moveTimer.moveRight = false
            moveTimer.moveLeft = false
            isMoving = false
            director.p1Model.playStand()
            director.p1Model.facingLeft = false
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
            if (moveRight) director.p2Model.posXRatio += moveStep
            if (moveLeft)  director.p2Model.posXRatio -= moveStep
        }
    }

    function startMoveRight2() {
        director.p2Model.facingLeft = false
        moveRight2Pressed = true
        moveTimer2.moveLeft = false
        moveTimer2.moveRight = true
        if (!isMoving2) {
            isMoving2 = true
            director.p2Model.playForward()
            moveTimer2.start()
        }
    }
    function startMoveLeft2() {
        director.p2Model.facingLeft = true
        moveLeft2Pressed = true
        moveTimer2.moveRight = false
        moveTimer2.moveLeft = true
        if (!isMoving2) {
            isMoving2 = true
            director.p2Model.playForward()
            moveTimer2.start()
        }
    }
    function stopMoveRight2() {
        moveRight2Pressed = false
        if (moveLeft2Pressed) {
            director.p2Model.facingLeft = true
            moveTimer2.moveRight = false
            moveTimer2.moveLeft = true
        } else {
            moveTimer2.stop()
            moveTimer2.moveRight = false
            moveTimer2.moveLeft = false
            isMoving2 = false
            director.p2Model.playStand()
            director.p2Model.facingLeft = false
        }
    }
    function stopMoveLeft2() {
        moveLeft2Pressed = false
        if (moveRight2Pressed) {
            director.p2Model.facingLeft = false
            moveTimer2.moveLeft = false
            moveTimer2.moveRight = true
        } else {
            moveTimer2.stop()
            moveTimer2.moveRight = false
            moveTimer2.moveLeft = false
            isMoving2 = false
            director.p2Model.playStand()
            director.p2Model.facingLeft = false
        }
    }

    Keys.onPressed: (event) => {
        if (event.isAutoRepeat) return
        switch (event.key) {
        case Qt.Key_D:
            startMoveRight(); break
        case Qt.Key_A:
            startMoveLeft(); break
        case Qt.Key_Right:
            startMoveRight2(); break
        case Qt.Key_Left:
            startMoveLeft2(); break
        }
    }
    Keys.onReleased: (event) => {
        if (event.isAutoRepeat) return
        switch (event.key) {
        case Qt.Key_D:
            stopMoveRight(); break
        case Qt.Key_A:
            stopMoveLeft(); break
        case Qt.Key_Right:
            stopMoveRight2(); break
        case Qt.Key_Left:
            stopMoveLeft2(); break
        }
    }

    Rectangle { anchors.fill: parent; color: "black"; z: -2 }

    Item {
        id: bgView; anchors.fill: parent; clip: true; z: 0
        AnimatedImage {
            id: monacoGif
            width: parent.width * 2; height: parent.height
            source: "qrc:/images/FightBackGround/Monaco.gif"
            fillMode: Image.PreserveAspectCrop
            smooth: false; mipmap: false; cache: true; asynchronous: true; paused: false
            horizontalAlignment: Image.AlignLeft; verticalAlignment: Image.AlignVCenter
            x: Math.min(0, Math.max(-(width - parent.width), 0)); y: 0
        }
    }

    // P1 (Yagami)
    Item {
        id: p1Layer
        x: root.width * director.p1Model.posXRatio - width / 2
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
                xScale: fitScale * (director.p1Model.facingLeft ? -1 : 1)
                yScale: fitScale
            }
        ]

        Image {
            source: director.p1Model.sourcePath
            width: director.p1Model.totalFrames * director.p1Model.frameWidth
            height: director.p1Model.frameHeight
            x: -(director.p1Model.currentFrame * director.p1Model.frameWidth)
            y: 0
            fillMode: Image.Stretch; smooth: false; mipmap: false
            cache: true; asynchronous: false
        }
    }

    // P2 (Orochi)
    Item {
        id: p2Layer
        x: root.width * director.p2Model.posXRatio - width / 2
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
                xScale: fitScale * (director.p2Model.facingLeft ? -1 : 1)
                yScale: fitScale
            }
        ]

        Image {
            source: director.p2Model.sourcePath
            width: director.p2Model.totalFrames * director.p2Model.frameWidth
            height: director.p2Model.frameHeight
            x: -(director.p2Model.currentFrame * director.p2Model.frameWidth)
            y: 0
            fillMode: Image.Stretch; smooth: false; mipmap: false
            cache: true; asynchronous: false
        }
    }

    // HUD
    Rectangle {
        id: hudBar
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 82; color: Qt.rgba(0, 0, 0, 0.85); z: 10
        Rectangle { anchors { top: parent.top; left: parent.left; right: parent.right }
            height: 2; color: "darkgoldenrod" }
        Rectangle { anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
            height: 2; color: "darkgoldenrod" }
    }

    Item {
        anchors { left: parent.left; leftMargin: 12; top: parent.top; topMargin: 8 }
        width: 340; height: 76; z: 11
        Rectangle { id: p1PortraitFrame; anchors { left: parent.left; verticalCenter: parent.verticalCenter }
            width: 54; height: 54; color: "black"; border.color: "darkgoldenrod"; border.width: 2; radius: 2
            Image { anchors.fill: parent; anchors.margins: 2; source: p1Avatar
                fillMode: Image.PreserveAspectCrop; asynchronous: true; smooth: false }
        }
        Column { anchors { left: p1PortraitFrame.right; leftMargin: 10; verticalCenter: parent.verticalCenter }
            spacing: 4
            Text { text: p1Name; font.pixelSize: 12; font.bold: true; color: "wheat"; font.family: "monospace" }
            Rectangle { width: 240; height: 16; color: "black"; border.color: "darkgoldenrod"; border.width: 1; radius: 1
                Rectangle { anchors { left: parent.left; leftMargin: 1; verticalCenter: parent.verticalCenter }
                    width: Math.max(0, (parent.width - 2) * (p1Health / 100.0)); height: parent.height - 2
                    color: "firebrick"; radius: 1
                    Rectangle { anchors { top: parent.top; bottom: parent.bottom; right: parent.right }
                        width: 6; color: "gold"; visible: p1Health > 0 }
                    Behavior on width { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }
                }
            }
            Rectangle { width: 240; height: 8; color: "black"; border.color: "darkslateblue"; border.width: 1; radius: 1
                Rectangle { anchors { left: parent.left; leftMargin: 1; verticalCenter: parent.verticalCenter }
                    width: Math.max(0, (parent.width - 2) * (p1Energy / 100.0)); height: parent.height - 2
                    color: "royalblue"; radius: 1
                    Behavior on width { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
                }
            }
        }
    }

    Rectangle { anchors { centerIn: hudBar; verticalCenterOffset: 2 }
        width: 52; height: 52; color: "black"; border.color: "darkgoldenrod"; border.width: 2; radius: 26; z: 11
        Text { anchors.centerIn: parent; text: timerSeconds; font.pixelSize: 26; font.bold: true
            color: timerSeconds <= 10 ? "red" : "wheat"; font.family: "monospace" }
    }

    Item {
        anchors { right: parent.right; rightMargin: 12; top: parent.top; topMargin: 8 }
        width: 340; height: 76; z: 11
        Rectangle { id: p2PortraitFrame; anchors { right: parent.right; verticalCenter: parent.verticalCenter }
            width: 54; height: 54; color: "black"; border.color: "darkgoldenrod"; border.width: 2; radius: 2
            Image { anchors.fill: parent; anchors.margins: 2; source: p2Avatar
                fillMode: Image.PreserveAspectCrop; asynchronous: true; smooth: false; mirror: true }
        }
        Column { anchors { right: p2PortraitFrame.left; rightMargin: 10; verticalCenter: parent.verticalCenter }
            spacing: 4
            Text { anchors.right: parent.right; text: p2Name; font.pixelSize: 12; font.bold: true
                color: "wheat"; font.family: "monospace"; horizontalAlignment: Text.AlignRight }
            Rectangle { width: 240; height: 16; color: "black"; border.color: "darkgoldenrod"; border.width: 1; radius: 1
                Rectangle { anchors { right: parent.right; rightMargin: 1; verticalCenter: parent.verticalCenter }
                    width: Math.max(0, (parent.width - 2) * (p2Health / 100.0)); height: parent.height - 2
                    color: "firebrick"; radius: 1
                    Rectangle { anchors { top: parent.top; bottom: parent.bottom; left: parent.left }
                        width: 6; color: "gold"; visible: p2Health > 0 }
                    Behavior on width { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }
                }
            }
            Rectangle { width: 240; height: 8; color: "black"; border.color: "darkslateblue"; border.width: 1; radius: 1
                Rectangle { anchors { right: parent.right; rightMargin: 1; verticalCenter: parent.verticalCenter }
                    width: Math.max(0, (parent.width - 2) * (p2Energy / 100.0)); height: parent.height - 2
                    color: "royalblue"; radius: 1
                    Behavior on width { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
                }
            }
        }
    }

    Text { anchors.horizontalCenter: parent.horizontalCenter; y: 10; text: "K.O.F. '97"
        font.pixelSize: 8; font.bold: true; color: "darkgoldenrod"; font.family: "monospace"; z: 12 }

    property int p1Health: 100
    property int p2Health: 100
    property int p1Energy: 0
    property int p2Energy: 0
    property int timerSeconds: 60

    Button {
        anchors { left: parent.left; bottom: parent.bottom; margins: 12 }
        width: 80; height: 28; z: 10; text: "<- BACK"
        onClicked: { if (stackViewRef) stackViewRef.pop() }
        contentItem: Text { text: "<- BACK"; color: "darkgoldenrod"; font.pixelSize: 11
            font.family: "monospace"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
        background: Rectangle { color: Qt.rgba(0, 0, 0, 0.8); border.color: "darkgoldenrod"; border.width: 1; radius: 2 }
    }

    Button {
        anchors { right: parent.right; bottom: parent.bottom; margins: 12 }
        width: 80; height: 28; z: 10; text: ">>"
        onClicked: { director.p1Model.playForward() }
        contentItem: Text { text: ">>"; color: "darkgoldenrod"; font.pixelSize: 11
            font.family: "monospace"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
        background: Rectangle { color: Qt.rgba(0, 0, 0, 0.8); border.color: "darkgoldenrod"; border.width: 1; radius: 2 }
    }

    Rectangle {
        anchors { right: parent.right; bottom: parent.bottom; margins: 12 }
        width: debugText.implicitWidth + 16; height: 22
        color: Qt.rgba(0, 0, 0, 0.6); radius: 2; z: 12
        Text { id: debugText; anchors.centerIn: parent
            text: "P1:" + director.p1Model.currentFrame + "/" + director.p1Model.totalFrames
                  + " P2:" + director.p2Model.currentFrame + "/" + director.p2Model.totalFrames
            color: "darkgreen"; font.pixelSize: 10; font.family: "monospace" }
    }

    Component.onCompleted: {
        director.start(p1CharId, p2CharId)
        root.forceActiveFocus()
    }
}
