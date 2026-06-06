// Module
// File: SelectScreen.qml   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-07 21:17:21
// Description:
//     Created the character selection screen
import QtQuick
import QtQuick.Controls

Item {
    id: root

    property var stackViewRef: null

    property var characters: [
        { cid: "Kusanagi", name: "草薙京",   avatar: "file:///wlf/FightGame/images/avatar/Kusanagi.jpg",  portrait: "file:///wlf/FightGame/images/portrait/Kusanagi.jpg" },
        { cid: "Kula",     name: "库拉",     avatar: "file:///wlf/FightGame/images/avatar/Kula.jpg",      portrait: "file:///wlf/FightGame/images/portrait/Kula.jpg" },
        { cid: "Orochi",   name: "大蛇",     avatar: "file:///wlf/FightGame/images/avatar/Orochi.jpg",    portrait: "file:///wlf/FightGame/images/portrait/Orochi.jpg" },
        { cid: "Yagami",   name: "八神庵",   avatar: "file:///wlf/FightGame/images/avatar/Yagami.jpg",    portrait: "file:///wlf/FightGame/images/portrait/Yagami.jpg" },
        { cid: "Shiranui", name: "不知火舞", avatar: "file:///wlf/FightGame/images/avatar/Shiranui.jpg",  portrait: "file:///wlf/FightGame/images/portrait/Shiranui.jpg" },
        { cid: "Kdash",    name: "K",        avatar: "file:///wlf/FightGame/images/avatar/Kdash.jpg",     portrait: "file:///wlf/FightGame/images/portrait/Kdash.jpg" }
    ]

    property int currentTurn: 1
    property int previewP1: -1
    property int previewP2: -1
    property int lockedP1: -1
    property int lockedP2: -1

    property string ac0: "black"; property string bc0: "dimgray"
    property string ac1: "black"; property string bc1: "dimgray"
    property string ac2: "black"; property string bc2: "dimgray"
    property string ac3: "black"; property string bc3: "dimgray"
    property string ac4: "black"; property string bc4: "dimgray"
    property string ac5: "black"; property string bc5: "dimgray"

    function refresh() {
        var colors = [], borders = []
        for (var i = 0; i < 6; i++) {
            if (i === lockedP1 || i === lockedP2)             { colors[i] = "black";          borders[i] = "darkgoldenrod" }
            else if (i === previewP1 && currentTurn === 1)    { colors[i] = "black";          borders[i] = "darkred" }
            else if (i === previewP2 && currentTurn === 2)    { colors[i] = "black";          borders[i] = "midnightblue" }
            else                                               { colors[i] = "black";          borders[i] = "dimgray" }
        }
        ac0 = colors[0]; bc0 = borders[0]
        ac1 = colors[1]; bc1 = borders[1]
        ac2 = colors[2]; bc2 = borders[2]
        ac3 = colors[3]; bc3 = borders[3]
        ac4 = colors[4]; bc4 = borders[4]
        ac5 = colors[5]; bc5 = borders[5]
    }

    Component.onCompleted: refresh()
    onCurrentTurnChanged: refresh()
    onPreviewP1Changed: refresh()
    onPreviewP2Changed: refresh()
    onLockedP1Changed: refresh()
    onLockedP2Changed: refresh()

    Rectangle { anchors.fill: parent; color: "black" }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 14
        text: "SELECT YOUR FIGHTER"
        font.pixelSize: 28; font.bold: true; color: "darkgoldenrod"
    }

    // P1
    Rectangle {
        x: 60; y: 58; width: 310; height: 350
        color: "black"
        border.color: currentTurn === 1 && lockedP1 < 0 ? "darkred" : (lockedP1 >= 0 ? "darkgoldenrod" : "black")
        border.width: 2; radius: 6

        Image {
            anchors.horizontalCenter: parent.horizontalCenter; y: 8
            width: 290; height: 280; fillMode: Image.PreserveAspectFit
            source: (previewP1 >= 0 || lockedP1 >= 0) ? characters[lockedP1 >= 0 ? lockedP1 : previewP1].portrait : ""
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter; y: 296
            text: (lockedP1 >= 0 || previewP1 >= 0) ? characters[lockedP1 >= 0 ? lockedP1 : previewP1].name : ""
            font.pixelSize: 18; font.bold: true; color: "indianred"
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter; y: 322
            text: lockedP1 >= 0 ? "READY" : (currentTurn === 1 ? "SELECTING" : "")
            font.pixelSize: 12; color: lockedP1 >= 0 ? "darkgoldenrod" : "dimgray"
        }
    }

    Text {
        x: 390; y: 210
        text: "VS"; font.pixelSize: 40; font.bold: true; color: "darkgoldenrod"
    }

    // P2
    Rectangle {
        x: 530; y: 58; width: 310; height: 350
        color: "black"
        border.color: currentTurn === 2 && lockedP2 < 0 ? "darkslateblue" : (lockedP2 >= 0 ? "darkgoldenrod" : "midnightblue")
        border.width: 2; radius: 6

        Image {
            anchors.horizontalCenter: parent.horizontalCenter; y: 8
            width: 290; height: 280; fillMode: Image.PreserveAspectFit
            source: (previewP2 >= 0 || lockedP2 >= 0) ? characters[lockedP2 >= 0 ? lockedP2 : previewP2].portrait : ""
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter; y: 296
            text: (lockedP2 >= 0 || previewP2 >= 0) ? characters[lockedP2 >= 0 ? lockedP2 : previewP2].name : ""
            font.pixelSize: 18; font.bold: true; color: "steelblue"
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter; y: 322
            text: lockedP2 >= 0 ? "READY" : (currentTurn === 2 ? "SELECTING" : "")
            font.pixelSize: 12; color: lockedP2 >= 0 ? "darkgoldenrod" : "dimgray"
        }
    }

    // 回合提示
    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter; y: 424
        width: 200; height: 30; color: Qt.rgba(0, 0, 0, 0.55); radius: 4
        Text {
            anchors.centerIn: parent
            text: currentTurn === 1 ? "PLAYER 1 SELECT" : (currentTurn === 2 ? "PLAYER 2 SELECT" : "READY")
            font.pixelSize: 14; font.bold: true
            color: currentTurn === 1 ? "indianred" : (currentTurn === 2 ? "steelblue" : "darkgoldenrod")
        }
    }

    // 头像网格
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 470; spacing: 10

        Rectangle { width: 100; height: 100; color: ac0; border.color: bc0; border.width: 2; radius: 4
            Image { anchors.centerIn: parent; width: 88; height: 88; fillMode: Image.PreserveAspectFit; source: characters[0].avatar }
            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (currentTurn === 1 && lockedP1 >= 0) return
                    if (currentTurn === 2 && lockedP2 >= 0) return
                    if (currentTurn === 1) previewP1 = 0; else previewP2 = 0
                }
            }
        }
        Rectangle { width: 100; height: 100; color: ac1; border.color: bc1; border.width: 2; radius: 4
            Image { anchors.centerIn: parent; width: 88; height: 88; fillMode: Image.PreserveAspectFit; source: characters[1].avatar }
            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (currentTurn === 1 && lockedP1 >= 0) return
                    if (currentTurn === 2 && lockedP2 >= 0) return
                    if (currentTurn === 1) previewP1 = 1; else previewP2 = 1
                }
            }
        }
        Rectangle { width: 100; height: 100; color: ac2; border.color: bc2; border.width: 2; radius: 4
            Image { anchors.centerIn: parent; width: 88; height: 88; fillMode: Image.PreserveAspectFit; source: characters[2].avatar }
            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (currentTurn === 1 && lockedP1 >= 0) return
                    if (currentTurn === 2 && lockedP2 >= 0) return
                    if (currentTurn === 1) previewP1 = 2; else previewP2 = 2
                }
            }
        }
        Rectangle { width: 100; height: 100; color: ac3; border.color: bc3; border.width: 2; radius: 4
            Image { anchors.centerIn: parent; width: 88; height: 88; fillMode: Image.PreserveAspectFit; source: characters[3].avatar }
            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (currentTurn === 1 && lockedP1 >= 0) return
                    if (currentTurn === 2 && lockedP2 >= 0) return
                    if (currentTurn === 1) previewP1 = 3; else previewP2 = 3
                }
            }
        }
        Rectangle { width: 100; height: 100; color: ac4; border.color: bc4; border.width: 2; radius: 4
            Image { anchors.centerIn: parent; width: 88; height: 88; fillMode: Image.PreserveAspectFit; source: characters[4].avatar }
            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (currentTurn === 1 && lockedP1 >= 0) return
                    if (currentTurn === 2 && lockedP2 >= 0) return
                    if (currentTurn === 1) previewP1 = 4; else previewP2 = 4
                }
            }
        }
        Rectangle { width: 100; height: 100; color: ac5; border.color: bc5; border.width: 2; radius: 4
            Image { anchors.centerIn: parent; width: 88; height: 88; fillMode: Image.PreserveAspectFit; source: characters[5].avatar }
            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (currentTurn === 1 && lockedP1 >= 0) return
                    if (currentTurn === 2 && lockedP2 >= 0) return
                    if (currentTurn === 1) previewP1 = 5; else previewP2 = 5
                }
            }
        }
    }

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom; anchors.bottomMargin: 16
        spacing: 20

        Button {
            id: btnConfirm
            text: "CONFIRM"; width: 120
            enabled: (currentTurn === 1 && previewP1 >= 0) || (currentTurn === 2 && previewP2 >= 0)
            onClicked: {
                if (currentTurn === 1) { lockedP1 = previewP1; previewP1 = -1; currentTurn = 2 }
                else if (currentTurn === 2) { lockedP2 = previewP2; previewP2 = -1; currentTurn = 0 }
            }
            contentItem: Text { text: btnConfirm.text; font.pixelSize: 16; font.bold: true; color: btnConfirm.enabled ? "silver" : "dimgray"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            background: Rectangle { color: btnConfirm.enabled ? (btnConfirm.hovered ? "black" : "black") : "black"; border.color: btnConfirm.enabled ? "darkgreen" : "black"; border.width: 2; radius: 4 }
        }

        Button {
            id: btnFight
            text: "FIGHT"; width: 160
            enabled: lockedP1 >= 0 && lockedP2 >= 0
            onClicked: {
                if (!stackViewRef) return
                var p1 = characters[lockedP1]
                var p2 = characters[lockedP2]
                stackViewRef.push("qrc:/qt/qml/FightGame/FightScreen.qml", {
                    "stackViewRef": stackViewRef,
                    "p1Name": p1.name,
                    "p1Avatar": p1.avatar,
                    "p1Portrait": p1.portrait,
                    "p2Name": p2.name,
                    "p2Avatar": p2.avatar,
                    "p2Portrait": p2.portrait
                })
            }

            contentItem: Text { text: btnFight.text; font.pixelSize: 18; font.bold: true; color: btnFight.enabled ? "silver" : "dimgray"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            background: Rectangle { color: btnFight.enabled ? (btnFight.hovered ? "black" : "black") : "black"; border.color: btnFight.enabled ? "darkred" : "black"; border.width: 2; radius: 4 }
        }

        Button {
            id: btnBack
            text: "BACK"; width: 120
            onClicked: { if (stackViewRef) stackViewRef.pop() }
            contentItem: Text { text: btnBack.text; font.pixelSize: 16; font.bold: true; color: "dimgray"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            background: Rectangle { color: "transparent"; border.color: btnBack.hovered ? "dimgray" : "dimgray"; border.width: 1; radius: 4 }
        }
    }
}
