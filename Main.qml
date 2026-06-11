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

ApplicationWindow {
    id: root
    width: 900
    height: 640
    visible: true
    title: "FIGHT GAME"
    color: "whitesmoke"

    StackView {
        id: stackView
        anchors.fill: parent

        initialItem: Item {
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

                Button {
                    id: btnLocal
                    text: "Local Two-Player"
                    width: 120; flat: true

                    contentItem: Text {
                        text: btnLocal.text; font.pixelSize: 14; color: "black"
                        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: btnLocal.hovered ? "lightgray" : "white"
                        border.color: btnLocal.hovered ? "dimgray" : "gray"
                        border.width: 1; radius: 2
                    }

                    onClicked: stackView.push(
                        "SelectScreen.qml",
                        { "stackViewRef": stackView }
                    )
                }

                Button {
                    id: btnOnline
                    text: "Online Two-Player"
                    width: 120; flat: true

                    contentItem: Text {
                        text: btnOnline.text; font.pixelSize: 14; color: "black"
                        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: btnOnline.hovered ? "lightgray" : "white"
                        border.color: btnOnline.hovered ? "dimgray" : "gray"
                        border.width: 1; radius: 2
                    }
                }

                Button {
                    id: btnExit
                    text: "EXIT"
                    width: 120; flat: true
                    onClicked: Qt.quit()

                    contentItem: Text {
                        text: btnExit.text; font.pixelSize: 14; color: "black"
                        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: btnExit.hovered ? "lightgray" : "white"
                        border.color: btnExit.hovered ? "dimgray" : "gray"
                        border.width: 1; radius: 2
                    }
                }
            }
        }
    }
}
