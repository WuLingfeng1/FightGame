// Module
// File: main.cpp   Version: 0.1.0   License: AGPLv3
// Created:Cheng Wang         2026-06-22 15:57:39
// Description:
//     在线对战大厅界面, 支持创建房间(Host)和加入房间(Join)
//     Host 模式: 创建房间等待对方连接
//     Join 模式: 浏览局域网房间列表或手动输入IP连接
import QtQuick
import QtQuick.Controls
import FightGame

Item {
    id: root

    property var stackViewRef: null

    NetworkManager {
        id: networkMgr
    }

    // 状态变量
    property bool isHost: true
    property bool isWaiting: false
    property string statusText: ""
    property string manualIp: ""

    property real fitScale: Math.min(root.width / 900, root.height / 640)
    property string accent: "darkgoldenrod"

    Rectangle { anchors.fill: parent; color: "black" }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 24 * fitScale
        text: "NETWORK BATTLE"
        font.pixelSize: 28 * fitScale; font.bold: true
        color: accent; font.family: "monospace"
    }

    // HOST / JOIN 切换标签
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 80 * fitScale
        spacing: 0

        Rectangle {
            width: 140 * fitScale; height: 36 * fitScale
            color: isHost ? "black" : Qt.rgba(0, 0, 0, 0.5)
            border.color: isHost ? accent : "dimgray"
            border.width: 1; radius: 4

            Text {
                anchors.centerIn: parent
                text: "HOST GAME"
                font.pixelSize: 14 * fitScale; font.family: "monospace"
                color: isHost ? accent : "dimgray"
            }

            MouseArea {
                anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                onClicked: { isHost = true; isWaiting = false; networkMgr.stopDiscovery(); networkMgr.disconnectFromHost(); statusText = "" }
            }
        }

        Rectangle {
            width: 140 * fitScale; height: 36 * fitScale
            color: !isHost ? "black" : Qt.rgba(0, 0, 0, 0.5)
            border.color: !isHost ? accent : "dimgray"
            border.width: 1; radius: 4

            Text {
                anchors.centerIn: parent
                text: "JOIN GAME"
                font.pixelSize: 14 * fitScale; font.family: "monospace"
                color: !isHost ? accent : "dimgray"
            }

            MouseArea {
                anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                onClicked: { isHost = false; isWaiting = false; networkMgr.stopServer(); networkMgr.disconnectFromHost(); networkMgr.startDiscovery(); statusText = "" }
            }
        }
    }

    // ====== Host 模式面板 ======
    Item {
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top; topMargin: 140 * fitScale
        }
        width: 440 * fitScale; height: 280 * fitScale
        visible: isHost

        Rectangle {
            anchors.fill: parent
            color: Qt.rgba(0, 0, 0, 0.6)
            border.color: "dimgray"; border.width: 1; radius: 6
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 20 * fitScale
            text: "CREATE A ROOM"
            font.pixelSize: 16 * fitScale; font.family: "monospace"
            color: accent
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 60 * fitScale
            width: 360 * fitScale; height: 40 * fitScale
            color: "black"; border.color: "dimgray"
            border.width: 1; radius: 4

            Row {
                anchors.centerIn: parent; spacing: 10 * fitScale
                Text {
                    text: "Local IP:"; font.pixelSize: 13 * fitScale
                    color: "dimgray"; font.family: "monospace"
                }
                Text {
                    text: networkMgr.localIp; font.pixelSize: 13 * fitScale
                    color: "wheat"; font.family: "monospace"
                }
            }
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            y: isWaiting ? 170 * fitScale : 145 * fitScale
            width: 360 * fitScale; height: 1
            color: "dimgray"
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            y: isWaiting ? 190 * fitScale : 165 * fitScale
            width: 160 * fitScale; height: 36 * fitScale
            color: "black"
            border.color: networkMgr.isConnected ? "darkgreen" : accent
            border.width: 2; radius: 4
            visible: !networkMgr.isConnected

            Text {
                anchors.centerIn: parent
                text: isWaiting ? (networkMgr.isConnected ? "CONNECTED" : "SEARCHING...") : "CREATE ROOM"
                font.pixelSize: 14 * fitScale; font.family: "monospace"
                color: isWaiting ? "dimgray" : accent
            }

            MouseArea {
                anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                enabled: !isWaiting
                onClicked: {
                    isWaiting = true
                    statusText = "Waiting for opponent to join..."
                    networkMgr.startServer()
                    networkMgr.announceRoom("Fight Room")
                }
            }
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 190 * fitScale
            width: 360 * fitScale; height: 28 * fitScale
            color: "transparent"; visible: networkMgr.isConnected

            Text {
                anchors.centerIn: parent
                text: "PLAYER 2 CONNECTED"
                font.pixelSize: 13 * fitScale; font.family: "monospace"
                color: "darkgreen"
            }
        }
    }

    // ====== Join 模式面板 ======
    Item {
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top; topMargin: 140 * fitScale
        }
        width: 440 * fitScale; height: 320 * fitScale
        visible: !isHost

        Rectangle {
            anchors.fill: parent
            color: Qt.rgba(0, 0, 0, 0.6)
            border.color: "dimgray"; border.width: 1; radius: 6
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 14 * fitScale
            text: "JOIN A ROOM"
            font.pixelSize: 16 * fitScale; font.family: "monospace"
            color: accent
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 46 * fitScale
            width: 390 * fitScale; height: 160 * fitScale
            color: "black"; border.color: "dimgray"
            border.width: 1; radius: 4

            ListView {
                anchors.fill: parent
                anchors.margins: 4
                model: networkMgr.discoveredRooms
                spacing: 4

                header: Text {
                    width: 380 * fitScale
                    text: networkMgr.discoveredRooms.length === 0
                          ? "No rooms found. Scanning LAN..."
                          : "Found Rooms:"
                    font.pixelSize: 11 * fitScale; color: "gray"
                    font.family: "monospace"
                    topPadding: 6 * fitScale; leftPadding: 8 * fitScale
                }

                delegate: Rectangle {
                    width: 370 * fitScale; height: 34 * fitScale
                    color: "black"; border.color: "dimgray"
                    border.width: 1; radius: 3

                    Row {
                        anchors.fill: parent
                        anchors.leftMargin: 12 * fitScale
                        spacing: 12 * fitScale

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.name + "  (" + modelData.ip + ")"
                            font.pixelSize: 12 * fitScale; color: "silver"
                            font.family: "monospace"
                            width: 240 * fitScale; elide: Text.ElideRight
                        }

                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 70 * fitScale; height: 24 * fitScale
                            color: "black"; border.color: accent
                            border.width: 1; radius: 3

                            Text {
                                anchors.centerIn: parent
                                text: "JOIN"
                                font.pixelSize: 11 * fitScale
                                font.family: "monospace"; color: accent
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    manualIp = modelData.ip
                                    statusText = "Connecting to " + modelData.ip + "..."
                                    networkMgr.connectToHost(modelData.ip)
                                }
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 216 * fitScale
            width: 390 * fitScale; height: 1
            color: "dimgray"
        }

        Text {
            x: 34 * fitScale; y: 228 * fitScale
            text: "Manual Connect:"
            font.pixelSize: 12 * fitScale; color: "dimgray"
            font.family: "monospace"
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 252 * fitScale
            spacing: 10 * fitScale

            Rectangle {
                width: 230 * fitScale; height: 34 * fitScale
                color: "black"; border.color: "dimgray"
                border.width: 1; radius: 4

                TextInput {
                    anchors.fill: parent
                    anchors.leftMargin: 10 * fitScale
                    verticalAlignment: TextInput.AlignVCenter
                    text: manualIp
                    font.pixelSize: 13 * fitScale; color: "wheat"
                    font.family: "monospace"
                    maximumLength: 21
                    onTextChanged: manualIp = text
                }

                Text {
                    anchors.fill: parent
                    anchors.leftMargin: 10 * fitScale
                    verticalAlignment: Text.AlignVCenter
                    text: "输入 IP 地址..."; font.pixelSize: 13 * fitScale
                    color: "dimgray"; font.family: "monospace"
                    visible: manualIp === ""
                }
            }

            Rectangle {
                width: 100 * fitScale; height: 34 * fitScale
                color: "black"
                border.color: manualIp !== "" ? accent : "dimgray"
                border.width: 2; radius: 4

                Text {
                    anchors.centerIn: parent
                    text: "CONNECT"; font.pixelSize: 13 * fitScale
                    font.family: "monospace"
                    color: manualIp !== "" ? accent : "dimgray"
                }

                MouseArea {
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                    enabled: manualIp !== ""
                    onClicked: {
                        statusText = "Connecting to " + manualIp + "..."
                        networkMgr.connectToHost(manualIp)
                    }
                }
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 296 * fitScale
            text: statusText
            font.pixelSize: 11 * fitScale; color: "dimgray"
            font.family: "monospace"
            visible: statusText !== ""
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 420 * fitScale
        text: statusText
        font.pixelSize: 12 * fitScale
        color: networkMgr.isConnected ? "darkgreen" : "dimgray"
        font.family: "monospace"
        visible: isHost && statusText !== ""
    }

    // BACK
    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom; anchors.bottomMargin: 30 * fitScale
        width: 100 * fitScale; height: 32 * fitScale
        color: "transparent"; border.color: "dimgray"
        border.width: 1; radius: 4

        Text {
            anchors.centerIn: parent
            text: "BACK"; font.pixelSize: 13 * fitScale
            font.family: "monospace"; color: "dimgray"
        }

        MouseArea {
            anchors.fill: parent; cursorShape: Qt.PointingHandCursor
            onClicked: {
                networkMgr.stopServer()
                networkMgr.stopDiscovery()
                networkMgr.disconnectFromHost()
                if (stackViewRef) stackViewRef.pop()
            }
        }
    }

    Connections {
        target: networkMgr

        function onClientConnected() {
            isWaiting = false
            statusText = "Player 2 connected!"
        }

        function onConnectedToHost() {
            networkMgr.stopDiscovery()
            statusText = "Connected to host!"
        }

        function onDisconnected() {
            isWaiting = false
            statusText = "Disconnected"
        }

        function onErrorOccurred(error) {
            isWaiting = false
            statusText = error
        }
    }
}
