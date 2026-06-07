// Module
// File: FightScreen.qml   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-07 19:59:27
// Description:
//     Basic settings translation for the battle interface completed
import QtQuick
import QtQuick.Controls

Item {
    id: root

    property var stackViewRef: null
    property string p1Name: ""
    property string p1Avatar: ""
    property string p1Portrait: ""
    property string p2Name: ""
    property string p2Avatar: ""
    property string p2Portrait: ""

    // 镜头偏移
    property real cameraX: 0
    readonly property real bgWidth: 1920

    // 键盘焦点

    focus: true
    Component.onCompleted: forceActiveFocus()

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Left)  cameraX = Math.max(0, cameraX - 8)
        if (event.key === Qt.Key_Right) cameraX = Math.min(bgWidth - 960, cameraX + 8)
    }

    // 背景

    Rectangle { anchors.fill: parent; color: "black" }

    Item {
        anchors.fill: parent
        anchors.topMargin: 90
        clip: true

        AnimatedImage {
            width: bgWidth
            height: parent.height
            x: -cameraX
            source: "file:///wlf/FightGame/images/FightBackGround/Monaco.gif"
            playing: true
            fillMode: Image.PreserveAspectCrop
            cache: false
        }
    }

    // 角色立绘

    Image {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        x: 100
        width: 192; height: 200
        fillMode: Image.PreserveAspectFit
        source: p1Portrait
    }
    Image {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        x: 668
        width: 192; height: 200
        fillMode: Image.PreserveAspectFit
        source: p2Portrait
    }

    // HUD（顶栏，覆盖在背景和立绘之上）

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 90
        color: Qt.rgba(0, 0, 0, 0.85)

        // P1 头像
        Rectangle {
            x: 18; y: 14; width: 58; height: 58
            color: "black"; border.color: "darkgoldenrod"; border.width: 2
            Image { anchors.fill: parent; anchors.margins: 1; source: p1Avatar; fillMode: Image.PreserveAspectCrop }
        }
        Text { x: 84; y: 10; text: p1Name; font.pixelSize: 9; font.bold: true; color: "white" }

        // P1 血条
        Rectangle {
            x: 84; y: 26; width: 320; height: 18
            color: "black"; border.color: "darkgoldenrod"; border.width: 1
            Rectangle { anchors.fill: parent; anchors.margins: 1; color: "darkred" }
            Rectangle {
                anchors.left: parent.left; anchors.top: parent.top
                anchors.bottom: parent.bottom; anchors.margins: 1
                width: (parent.width - 2) * 0.78; color: "gold"
            }
        }
        Row { x: 84; y: 48; spacing: 5
            Repeater { model: 6; Rectangle { width: 14; height: 10; color: "royalblue"; border.color: "darkblue"; border.width: 1 } }
        }

        // P2 头像
        Rectangle {
            x: parent.width - 18 - 58; y: 14; width: 58; height: 58
            color: "black"; border.color: "darkgoldenrod"; border.width: 2
            Image { anchors.fill: parent; anchors.margins: 1; source: p2Avatar; fillMode: Image.PreserveAspectCrop }
        }
        Text { anchors.right: parent.right; anchors.rightMargin: 84; y: 10; text: p2Name; font.pixelSize: 9; font.bold: true; color: "white" }

        // P2 血条
        Rectangle {
            anchors.right: parent.right; anchors.rightMargin: 84
            y: 26; width: 320; height: 18
            color: "black"; border.color: "darkgoldenrod"; border.width: 1
            Rectangle { anchors.fill: parent; anchors.margins: 1; color: "darkred" }
            Rectangle {
                anchors.right: parent.right; anchors.top: parent.top
                anchors.bottom: parent.bottom; anchors.margins: 1
                width: (parent.width - 2) * 0.64; color: "gold"
            }
        }
        Row { anchors.right: parent.right; anchors.rightMargin: 84; y: 48; spacing: 5; layoutDirection: Qt.RightToLeft
            Repeater { model: 6; Rectangle { width: 14; height: 10; color: "royalblue"; border.color: "darkblue"; border.width: 1 } }
        }

        // 倒计时
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter; y: 30
            width: 44; height: 44; color: "black"
            border.color: "darkgoldenrod"; border.width: 2; radius: 22
            Text { anchors.centerIn: parent; text: "60"; font.pixelSize: 22; font.bold: true; color: "white" }
        }
    }

    // BACK
    Button {
        id: btnBack
        anchors.left: parent.left; anchors.bottom: parent.bottom; anchors.margins: 10
        text: "BACK"; width: 80; height: 24
        onClicked: { if (stackViewRef) stackViewRef.pop() }
        contentItem: Text {
            text: btnBack.text; font.pixelSize: 11; font.bold: true; color: "dimgray"
            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle { color: "transparent"; border.color: btnBack.hovered ? "darkgoldenrod" : "dimgray"; border.width: 1; radius: 2 }
    }
}
