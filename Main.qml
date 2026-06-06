// Module
// File: Main.qml   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-06 18:01:23
// Description:
//     Created the start interface
import QtQuick
import QtQuick.Controls

Window {
    id: root
    width: 900
    height: 640
    visible: true
    title: "FIGHT GAME"
    color: "lightgray"

    Image {
        anchors.fill: parent
        source: "file:///wlf/FightGame/images/background.jpg"
        fillMode: Image.PreserveAspectFit
    }

    Column {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 40
        spacing: 4

        Text {
            text: "FIGHT GAME"
            font.pixelSize: 36
            font.bold: true
            color: "black"
        }
        Text {
            text: "SELECT YOUR FIGHTER"
            font.pixelSize: 12
            color: "dimgray"
        }
    }

    Column {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 40
        spacing: 8

        Button {
            text: "Local Two-Player"
            width: 120
            flat: true

            contentItem: Text {
                text: parent.text
                font.pixelSize: 14
                color: "black"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                color: parent.hovered ? "lightgray" : "white"
                border.color: parent.hovered ? "dimgray" : "gray"
                border.width: 1
                radius: 2
            }
        }

        Button {
            text: "Online Two-Player"
            width: 120
            flat: true

            contentItem: Text {
                text: parent.text
                font.pixelSize: 14
                color: "black"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                color: parent.hovered ? "lightgray" : "white"
                border.color: parent.hovered ? "dimgray" : "gray"
                border.width: 1
                radius: 2
            }
        }

        Button {
            text: "EXIT"
            width: 120
            flat: true
            onClicked: Qt.quit()

            contentItem: Text {
                text: parent.text
                font.pixelSize: 14
                color: "black"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                color: parent.hovered ? "lightgray" : "white"
                border.color: parent.hovered ? "dimgray" : "gray"
                border.width: 1
                radius: 2
            }
        }
    }
}
