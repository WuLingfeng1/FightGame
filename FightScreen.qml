// Module
// File: FightScreen.qml   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-10 14:56:24
// Description:
//     Created the Fight screen
import QtQuick
import QtQuick.Controls
import FightGame

Item {
    id: root

    // 一、页面传参
    property var    stackViewRef: null
    property string p1Name:     ""
    property string p1Avatar:   ""
    property string p1Portrait: ""
    property string p1CharId:   ""
    property string p2Name:     ""
    property string p2Avatar:   ""
    property string p2Portrait: ""
    property string p2CharId:   ""

    // 二、C++ 角色模型
    FighterModel { id: p1Model }
    FighterModel { id: p2Model }

    Binding { target: p1Model; property: "charId"; value: p1CharId }
    Binding { target: p2Model; property: "charId"; value: p2CharId }

    // 三、视图参数 + 相机
    property int cellW: 0
    property int cellH: 0
    property int cols:  9
    property int rows:  1
    property bool imgLoaded: false

    property int cameraX: 0
    property int p1Health: 100
    property int p2Health: 100
    property int p1Energy: 0
    property int p2Energy: 0
    property int timerSeconds: 60

    // 四、背景

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
            width:  parent.width * 2
            height: parent.height
            source: "file:///wlf/FightGame/images/FightBackGround/Monaco.gif"
            fillMode: Image.PreserveAspectCrop
            smooth: false
            mipmap: false
            cache:  false
            asynchronous: true
            paused: false
            horizontalAlignment: Image.AlignLeft
            verticalAlignment:   Image.AlignVCenter

            x: clampBgX(-cameraX * 0.5)
            y: 0

            onStatusChanged: {
                if (status === AnimatedImage.Loading)
                    console.log("[FightScreen] Monaco.gif loading...")
                else if (status === AnimatedImage.Ready)
                    console.log("[FightScreen] Monaco.gif loaded")
                else if (status === AnimatedImage.Error)
                    console.log("[FightScreen] Monaco.gif load failed")
            }

            function clampBgX(desiredX) {
                let maxX = 0
                let minX = -(width - parent.width)
                if (minX > 0) minX = 0
                return Math.min(maxX, Math.max(minX, desiredX))
            }
        }
    }

    // 五、角色精灵 — P1
    Item {
        id: p1SpriteContainer
        width:  imgLoaded ? cellW : 0
        height: imgLoaded ? cellH : 0
        x: 60
        y: root.height - height - 95
        clip: true
        z: 1

        Image {
            id: p1SpriteSheet
            source: p1CharId !== ""
                    ? "file:///wlf/FightGame/images/character/" + p1CharId + "/Stand.png"
                    : ""

            fillMode: Image.Pad
            smooth:   false
            mipmap:   false
            cache:    true
            asynchronous: true
            mirror:   false

            onSourceSizeChanged: {
                if (sourceSize.width <= 0 || sourceSize.height <= 0) {
                    imgLoaded = false
                    return
                }
                cellW = sourceSize.width / cols
                cellH = sourceSize.height
                imgLoaded = true
                console.log("[FightScreen] Stand.png loaded | cell:", cellW, "x", cellH)
                refreshFrameP1(p1Model.currentFrame)
            }

            onStatusChanged: {
                if (status === Image.Error && source.toString() !== "") {
                    console.log("[FightScreen] P1 image load failed, fallback -> Yagami")
                    p1SpriteSheet.source = "file:///wlf/FightGame/images/character/Yagami/Stand.png"
                }
            }

            function refreshFrameP1(frameIdx) {
                if (!imgLoaded) return
                let col = frameIdx % cols
                p1SpriteSheet.x = -col * cellW
                p1SpriteSheet.y = 0
            }
        }
    }

    Connections {
        target: p1Model
        function onCurrentFrameChanged() {
            p1SpriteSheet.refreshFrameP1(p1Model.currentFrame)
        }
    }

    // 六、角色精灵 — P2
    Item {
        id: p2SpriteContainer
        width:  imgLoaded ? cellW : 0
        height: imgLoaded ? cellH : 0
        x: root.width - width - 60
        y: root.height - height - 95
        clip: true
        z: 1

        Image {
            id: p2SpriteSheet
            source: p2CharId !== ""
                    ? "file:///wlf/FightGame/images/character/" + p2CharId + "/Stand.png"
                    : ""

            fillMode: Image.Pad
            smooth:   false
            mipmap:   false
            cache:    true
            asynchronous: true
            mirror:   true

            onSourceSizeChanged: {
                if (sourceSize.width <= 0 || sourceSize.height <= 0) return
                refreshFrameP2(p2Model.currentFrame)
            }

            onStatusChanged: {
                if (status === Image.Error && source.toString() !== "") {
                    console.log("[FightScreen] P2 image load failed, fallback -> Yagami")
                    p2SpriteSheet.source = "file:///wlf/FightGame/images/character/Yagami/Stand.png"
                }
            }

            function refreshFrameP2(frameIdx) {
                if (!imgLoaded) return
                let col = frameIdx % cols
                p2SpriteSheet.x = -col * cellW
                p2SpriteSheet.y = 0
            }
        }
    }

    Connections {
        target: p2Model
        function onCurrentFrameChanged() {
            p2SpriteSheet.refreshFrameP2(p2Model.currentFrame)
        }
    }

    // 七、KOF 97 HUD
    Rectangle {
        id: hudBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 82
        color: Qt.rgba(0, 0, 0, 0.85)
        z: 10

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 2
            color: "darkgoldenrod"
        }
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 2
            color: "darkgoldenrod"
        }
    }

    // ---- P1 side (left) ----
    Item {
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.top: parent.top
        anchors.topMargin: 8
        width: 340
        height: 76
        z: 11

        Rectangle {
            id: p1PortraitFrame
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: 54; height: 54
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
            anchors.left: p1PortraitFrame.right
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            spacing: 4

            Text {
                text: p1Name
                font.pixelSize: 12; font.bold: true
                color: "wheat"
                font.family: "monospace"
            }

            Rectangle {
                width: 240; height: 16
                color: "black"
                border.color: "darkgoldenrod"
                border.width: 1
                radius: 1

                Rectangle {
                    anchors.left: parent.left
                    anchors.leftMargin: 1
                    anchors.verticalCenter: parent.verticalCenter
                    width: Math.max(0, (parent.width - 2) * (p1Health / 100.0))
                    height: parent.height - 2
                    color: "firebrick"
                    radius: 1

                    Rectangle {
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        width: 6
                        color: "gold"
                        visible: p1Health > 0
                    }

                    Behavior on width {
                        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                    }
                }
            }

            Rectangle {
                width: 240; height: 8
                color: "black"
                border.color: "darkslateblue"
                border.width: 1
                radius: 1

                Rectangle {
                    anchors.left: parent.left
                    anchors.leftMargin: 1
                    anchors.verticalCenter: parent.verticalCenter
                    width: Math.max(0, (parent.width - 2) * (p1Energy / 100.0))
                    height: parent.height - 2
                    color: "royalblue"
                    radius: 1

                    Behavior on width {
                        NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
                    }
                }
            }
        }
    }

    // ---- Timer (center) ----
    Rectangle {
        anchors.centerIn: hudBar
        anchors.verticalCenterOffset: 2
        width: 52; height: 52
        color: "black"
        border.color: "darkgoldenrod"
        border.width: 2
        radius: 26
        z: 11

        Text {
            anchors.centerIn: parent
            text: timerSeconds
            font.pixelSize: 26; font.bold: true
            color: timerSeconds <= 10 ? "red" : "wheat"
            font.family: "monospace"
        }
    }

    // ---- P2 side (right) ----
    Item {
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.top: parent.top
        anchors.topMargin: 8
        width: 340
        height: 76
        z: 11

        Rectangle {
            id: p2PortraitFrame
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
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
            anchors.right: p2PortraitFrame.left
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            spacing: 4

            Text {
                anchors.right: parent.right
                text: p2Name
                font.pixelSize: 12; font.bold: true
                color: "wheat"
                font.family: "monospace"
                horizontalAlignment: Text.AlignRight
            }

            Rectangle {
                width: 240; height: 16
                color: "black"
                border.color: "darkgoldenrod"
                border.width: 1
                radius: 1

                Rectangle {
                    anchors.right: parent.right
                    anchors.rightMargin: 1
                    anchors.verticalCenter: parent.verticalCenter
                    width: Math.max(0, (parent.width - 2) * (p2Health / 100.0))
                    height: parent.height - 2
                    color: "firebrick"
                    radius: 1

                    Rectangle {
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        width: 6
                        color: "gold"
                        visible: p2Health > 0
                    }

                    Behavior on width {
                        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                    }
                }
            }

            Rectangle {
                width: 240; height: 8
                color: "black"
                border.color: "darkslateblue"
                border.width: 1
                radius: 1

                Rectangle {
                    anchors.right: parent.right
                    anchors.rightMargin: 1
                    anchors.verticalCenter: parent.verticalCenter
                    width: Math.max(0, (parent.width - 2) * (p2Energy / 100.0))
                    height: parent.height - 2
                    color: "royalblue"
                    radius: 1

                    Behavior on width {
                        NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
                    }
                }
            }
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 10
        text: "K.O.F. '97"
        font.pixelSize: 8; font.bold: true
        color: "darkgoldenrod"
        font.family: "monospace"
        z: 12
    }

    // 八、Back button
    Button {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 12
        width: 80; height: 28; z: 10
        text: "<- BACK"

        onClicked: {
            if (stackViewRef) stackViewRef.pop()
        }

        contentItem: Text {
            text: "<- BACK"
            color: "darkgoldenrod"
            font.pixelSize: 11
            font.family: "monospace"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment:   Text.AlignVCenter
        }
        background: Rectangle {
            color: Qt.rgba(0, 0, 0, 0.8)
            border.color: "darkgoldenrod"
            border.width: 1
            radius: 2
        }
    }

    // 九、Debug HUD
    Rectangle {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 12
        width:  debugText.implicitWidth + 16
        height: 22
        color: Qt.rgba(0, 0, 0, 0.6)
        radius: 2
        z: 10

        Text {
            id: debugText
            anchors.centerIn: parent
            text: "P1:" + p1Model.currentFrame + " P2:" + p2Model.currentFrame
            color: "darkgreen"
            font.pixelSize: 10
            font.family: "monospace"
        }
    }

    Component.onCompleted: {
        console.log("[FightScreen] battle screen ready")
    }
}
