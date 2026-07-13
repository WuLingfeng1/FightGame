// Module
// File: StageSelectScreen.qml   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-07-11 14:00:00
// Description:
//     地图选择界面
import QtQuick
import QtQuick.Controls
import FightGame

Item {
    id: root

    property var    stackViewRef: null
    property bool   isOnline:   false
    property bool   isHost:     false
    property var    networkMgr: null
    property string p1Name:     ""
    property string p1Avatar:   ""
    property string p1Portrait: ""
    property string p1CharId:   ""
    property string p2Name:     ""
    property string p2Avatar:   ""
    property string p2Portrait: ""
    property string p2CharId:   ""

    property var maps: [
        { id: "AmusementPark",  name: "Amusement Park" },
        { id: "Bali",           name: "Bali" },
        { id: "Gyeongbokgung",  name: "Gyeongbokgung" },
        { id: "jiulong",        name: "Jiulong" },
        { id: "Monaco",         name: "Monaco" },
        { id: "OrochiShermie",  name: "Orochi Shermie" }
    ]

    property int selectedIndex: 4  // 默认选中 Monaco
    property real cardW: 260
    property real cardH: 180

    // 纯黑背景
    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    // 返回按钮
    Button {
        id: btnBack
        text: "BACK"
        width: 80; height: 28
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 16
        flat: true
        contentItem: Text {
            text: btnBack.text
            font.pixelSize: 13
            color: "dimgray"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            color: "transparent"
            border.color: btnBack.hovered ? "gray" : "dimgray"
            border.width: 1
            radius: 2
        }
        onClicked: {
            if (isOnline) {
                if (stackViewRef) stackViewRef.pop()
            } else {
                if (stackViewRef) stackViewRef.pop()
            }
        }
    }

    // 标题
    Text {
        text: (isOnline && !isHost) ? "WAITING FOR STAGE..." : "SELECT STAGE"
        font.pixelSize: 24
        font.bold: true
        color: "white"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 18
    }

    // 地图卡片网格: 2行×3列
    Grid {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 14
        columns: 3
        rows: 2
        spacing: 16

        Repeater {
            model: maps

            Item {
                width: cardW
                height: cardH

                // GIF 预览
                AnimatedImage {
                    anchors.fill: parent
                    anchors.margins: 2
                    source: "qrc:/images/FightBackGround/" + modelData.id + ".gif"
                    fillMode: Image.PreserveAspectCrop
                    smooth: false
                    paused: false
                    playing: true
                    cache: true
                    asynchronous: true
                }

                // 底部名称条
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 26
                    color: root.selectedIndex === index ? "black" : "black"

                    Text {
                        anchors.centerIn: parent
                        text: modelData.name
                        font.pixelSize: 12
                        color: root.selectedIndex === index ? "white" : "dimgray"
                    }
                }

                // 选中标记线
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: root.selectedIndex === index ? "gray" : "transparent"
                }

                // 边框
                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    border.color: root.selectedIndex === index ? "gray" : "dimgray"
                    border.width: root.selectedIndex === index ? 2 : 1
                }

                // 点击区域
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    enabled: !isOnline || isHost
                    onClicked: {
                        root.selectedIndex = index
                    }
                }
            }
        }
    }

    // 底部 Fight 按钮
    Button {
        id: btnFight
        text: (isOnline && isHost) ? "CONFIRM" : "FIGHT"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24
        anchors.horizontalCenter: parent.horizontalCenter
        width: 140; height: 36
        flat: true
        enabled: !isOnline || isHost
        contentItem: Text {
            text: btnFight.text
            font.pixelSize: 16
            font.bold: true
            color: btnFight.enabled ? "lightgray" : "dimgray"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            color: "transparent"
            border.color: btnFight.hovered ? "gray" : "dimgray"
            border.width: 1
            radius: 2
        }
        onClicked: {
            if (!stackViewRef) return
            if (isOnline && isHost && networkMgr) {
                networkMgr.sendMessage({"type": "stage_select", "stageId": maps[root.selectedIndex].id})
            }
            var stage = maps[root.selectedIndex]
            var comp = Qt.createComponent("qrc:/qt/qml/FightGame/FightScreen.qml")
            if (comp.status === Component.Ready) {
                var props = {
                    "stackViewRef": stackViewRef,
                    "p1Name":     root.p1Name,
                    "p1Avatar":   root.p1Avatar,
                    "p1Portrait": root.p1Portrait,
                    "p1CharId":   root.p1CharId,
                    "p2Name":     root.p2Name,
                    "p2Avatar":   root.p2Avatar,
                    "p2Portrait": root.p2Portrait,
                    "p2CharId":   root.p2CharId,
                    "stageId":    stage.id,
                    "isOnline":   isOnline,
                    "isHost":     isHost,
                    "networkMgr": networkMgr
                }
                stackViewRef.push(comp, props)
            }
        }
    }

    // 键盘导航: 左右切换, 回车确认
    focus: true
    Keys.onPressed: function(event) {
        if (isOnline && !isHost) return
        if (event.key === Qt.Key_Left) {
            root.selectedIndex = (root.selectedIndex + 5) % 6
        } else if (event.key === Qt.Key_Right) {
            root.selectedIndex = (root.selectedIndex + 1) % 6
        } else if (event.key === Qt.Key_Up) {
            root.selectedIndex = (root.selectedIndex + 3) % 6
        } else if (event.key === Qt.Key_Down) {
            root.selectedIndex = (root.selectedIndex + 3) % 6
        } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            btnFight.clicked()
        }
    }

    Connections {
        target: networkMgr
        enabled: isOnline && !isHost && networkMgr !== null

        function onMessageReceived(msg) {
            if (msg.type === "stage_select") {
                var comp = Qt.createComponent("qrc:/qt/qml/FightGame/FightScreen.qml")
                if (comp.status === Component.Ready) {
                    var props = {
                        "stackViewRef": stackViewRef,
                        "p1Name":     root.p1Name,
                        "p1Avatar":   root.p1Avatar,
                        "p1Portrait": root.p1Portrait,
                        "p1CharId":   root.p1CharId,
                        "p2Name":     root.p2Name,
                        "p2Avatar":   root.p2Avatar,
                        "p2Portrait": root.p2Portrait,
                        "p2CharId":   root.p2CharId,
                        "stageId":    msg.stageId,
                        "isOnline":   true,
                        "isHost":     false,
                        "networkMgr": networkMgr
                    }
                    stackViewRef.push(comp, props)
                }
            }
        }
    }

    // 客机等待遮罩
    Rectangle {
        anchors.fill: parent
        color: "black"
        opacity: 0.65
        visible: isOnline && !isHost
        z: 10
    }
    Text {
        anchors.centerIn: parent
        text: "等待主机选择地图..."
        font.pixelSize: 18
        font.family: "monospace"
        color: "darkgoldenrod"
        visible: isOnline && !isHost
        z: 11
    }
}
