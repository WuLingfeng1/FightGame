// Module
// File: Main.qml   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-06 18:01:23
// Description:
//     Created the start interface
// Change Log:
//     [v0.1.1]     2026-06-07 21:19:37
//         * Integrate the character selection screen into the "Local Two-Player" button
import QtQuick
import QtQuick.Controls
import FightGame

ApplicationWindow {
    id: root
    width: 900
    height: 640
    visible: true
    title: "FIGHT GAME"
    color: "whitesmoke"

    component MenuButton: Button {
        flat: true
        implicitWidth: 120
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

    // 全局键位配置实例
    KeyBindingConfig {
        id: globalKeyBindings
        Component.onCompleted: {
            if (!loadFromFile("config/keybindings.json"))
                loadFromFile(":/config/keybindings.json")
        }
    }

    StackView {
        id: stackView
        anchors.fill: parent

        initialItem: Item {
            Component.onCompleted: MusicManager.playBegin()
            Component.onDestruction: MusicManager.stop()
            StackView.onActivated: MusicManager.playBegin()

            Image {
                anchors.fill: parent
                source: "qrc:/images/background.jpg"
                fillMode: Image.PreserveAspectFit
                z: -1
            }

            Column {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: 40
                spacing: 4

                Text {
                    text: "FIGHT GAME"
                    font.pixelSize: 36; font.bold: true; color: "black"
                }
                Text {
                    text: "SELECT YOUR FIGHTER"
                    font.pixelSize: 12; color: "dimgray"
                }
            }

            Column {
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 40
                spacing: 8

                MenuButton {
                    text: "Local Two-Player"
                    onClicked: stackView.push("SelectScreen.qml", { "stackViewRef": stackView })
                }
                MenuButton {
                    text: "Online Two-Player"
                    onClicked: stackView.push("OnlineLobby.qml", { "stackViewRef": stackView })
                }
                MenuButton {
                    text: "Key Settings"
                    onClicked: stackView.push("KeyBindingScreen.qml", { "stackViewRef": stackView, "keyBindingConfig": globalKeyBindings })
                }
                MenuButton {
                    text: "EXIT"
                    onClicked: Qt.quit()
                }
            }
        }
    }
}
