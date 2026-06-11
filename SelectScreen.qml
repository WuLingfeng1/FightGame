// Module
// File: SelectScreen.qml   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-07 18:17:21
// Description:
//     Created the character selection screen
// Change Log:
//     [v0.1.1]     2026-06-10 18:53:19
//         *Add a jump from the Fight button to the FightScreen interface
// Change Log:
//     [v0.1.2]     2026-06-11 23:11:51
//         *Fixed window stretch problem
import QtQuick
import QtQuick.Controls

Item {
    id: root

    property var stackViewRef: null

    property var characters: [
        { cid: "Kusanagi", name: "草薙京",   avatar: "qrc:/images/avatar/Kusanagi.jpg",  portrait: "qrc:/images/portrait/Kusanagi.jpg" },
        { cid: "Kula",     name: "库拉",     avatar: "qrc:/images/avatar/Kula.jpg",      portrait: "qrc:/images/portrait/Kula.jpg" },
        { cid: "Orochi",   name: "大蛇",     avatar: "qrc:/images/avatar/Orochi.jpg",    portrait: "qrc:/images/portrait/Orochi.jpg" },
        { cid: "Yagami",   name: "八神庵",   avatar: "qrc:/images/avatar/Yagami.jpg",    portrait: "qrc:/images/portrait/Yagami.jpg" },
        { cid: "Shiranui", name: "不知火舞", avatar: "qrc:/images/avatar/Shiranui.jpg",  portrait: "qrc:/images/portrait/Shiranui.jpg" },
        { cid: "Kdash",    name: "K",        avatar: "qrc:/images/avatar/Kdash.jpg",     portrait: "qrc:/images/portrait/Kdash.jpg" }
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
              // 情况 A：这个角色被某个人锁定了 - 金边框
            if (i === lockedP1 || i === lockedP2)             { colors[i] = "black";          borders[i] = "darkgoldenrod" }
             // 情况 B：P1 正在预览这个角色 - 暗红边框
            else if (i === previewP1 && currentTurn === 1)    { colors[i] = "black";          borders[i] = "darkred" }
             // 情况 C：P2 正在预览这个角色 - 深蓝边框
            else if (i === previewP2 && currentTurn === 2)    { colors[i] = "black";          borders[i] = "midnightblue" }
             // 情况 D：没被任何人关注 - 灰色边框
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

    property real fitScale: Math.min(root.width / 900, root.height / 640)

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 14 * fitScale
        text: "SELECT YOUR FIGHTER"
        font.pixelSize: 28 * fitScale; font.bold: true; color: "darkgoldenrod"
    }

    // P1
    Rectangle {
        x: parent.width * 0.07
        y: parent.height * 0.1
        width: parent.width * 0.34
        height: parent.height * 0.54
        color: "black"
        border.color: currentTurn === 1 && lockedP1 < 0 ? "darkred" : (lockedP1 >= 0 ? "darkgoldenrod" : "black")
        border.width: 2; radius: 6

        Image {
            anchors.horizontalCenter: parent.horizontalCenter; y: 8 * fitScale
            width: parent.width * 0.93; height: parent.height * 0.78
            fillMode: Image.PreserveAspectFit
            source: (previewP1 >= 0 || lockedP1 >= 0) ? characters[lockedP1 >= 0 ? lockedP1 : previewP1].portrait : ""
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height * 0.85
            text: (lockedP1 >= 0 || previewP1 >= 0) ? characters[lockedP1 >= 0 ? lockedP1 : previewP1].name : ""
            font.pixelSize: 18 * fitScale; font.bold: true; color: "indianred"
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height * 0.92
            text: lockedP1 >= 0 ? "READY" : (currentTurn === 1 ? "SELECTING" : "")
            font.pixelSize: 12 * fitScale; color: lockedP1 >= 0 ? "darkgoldenrod" : "dimgray"
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height * 0.33
        text: "VS"
        font.pixelSize: 40 * fitScale; font.bold: true; color: "darkgoldenrod"
    }

    // P2
    Rectangle {
        x: parent.width * 0.59
        y: parent.height * 0.1
        width: parent.width * 0.34
        height: parent.height * 0.54
        color: "black"
        border.color: currentTurn === 2 && lockedP2 < 0 ? "darkslateblue" : (lockedP2 >= 0 ? "darkgoldenrod" : "midnightblue")
        border.width: 2; radius: 6

        Image {
            anchors.horizontalCenter: parent.horizontalCenter; y: 8 * fitScale
            width: parent.width * 0.93; height: parent.height * 0.78
            fillMode: Image.PreserveAspectFit
            source: (previewP2 >= 0 || lockedP2 >= 0) ? characters[lockedP2 >= 0 ? lockedP2 : previewP2].portrait : ""
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height * 0.85
            text: (lockedP2 >= 0 || previewP2 >= 0) ? characters[lockedP2 >= 0 ? lockedP2 : previewP2].name : ""
            font.pixelSize: 18 * fitScale; font.bold: true; color: "steelblue"
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height * 0.92
            text: lockedP2 >= 0 ? "READY" : (currentTurn === 2 ? "SELECTING" : "")
            font.pixelSize: 12 * fitScale; color: lockedP2 >= 0 ? "darkgoldenrod" : "dimgray"
        }
    }

    // 回合提示
    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height * 0.67
        width: 200 * fitScale; height: 30 * fitScale
        color: Qt.rgba(0, 0, 0, 0.55); radius: 4
        Text {
            anchors.centerIn: parent
            text: currentTurn === 1 ? "PLAYER 1 SELECT" : (currentTurn === 2 ? "PLAYER 2 SELECT" : "READY")
            font.pixelSize: 14 * fitScale; font.bold: true
            color: currentTurn === 1 ? "indianred" : (currentTurn === 2 ? "steelblue" : "darkgoldenrod")
        }
    }

    // 头像网格
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom; anchors.bottomMargin: 60 * fitScale
        spacing: 10 * fitScale

        Repeater {
            model: 6
            Rectangle {
                width: 100 * fitScale; height: 100 * fitScale
                color: root["ac" + index]; border.color: root["bc" + index]
                border.width: 2; radius: 4
                Image {
                    anchors.centerIn: parent
                    width: 88 * fitScale; height: 88 * fitScale
                    fillMode: Image.PreserveAspectFit
                    source: characters[index].avatar
                }
                MouseArea {
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (currentTurn === 1 && lockedP1 >= 0) return
                        if (currentTurn === 2 && lockedP2 >= 0) return
                        if (currentTurn === 1) previewP1 = index; else previewP2 = index
                    }
                }
            }
        }
    }

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom; anchors.bottomMargin: 16 * fitScale
        spacing: 20 * fitScale

        Button {
            id: btnConfirm
            text: "CONFIRM"; width: 120 * fitScale
            enabled: (currentTurn === 1 && previewP1 >= 0) || (currentTurn === 2 && previewP2 >= 0)
            onClicked: {
                if (currentTurn === 1) { lockedP1 = previewP1; previewP1 = -1; currentTurn = 2 }
                else if (currentTurn === 2) { lockedP2 = previewP2; previewP2 = -1; currentTurn = 0 }
            }
            contentItem: Text { text: btnConfirm.text; font.pixelSize: 16 * fitScale; font.bold: true; color: btnConfirm.enabled ? "silver" : "dimgray"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            background: Rectangle { color: btnConfirm.enabled ? (btnConfirm.hovered ? "black" : "black") : "black"; border.color: btnConfirm.enabled ? "darkgreen" : "black"; border.width: 2; radius: 4 }
        }

        Button {
            id: btnFight
            text: "FIGHT"; width: 160 * fitScale
            enabled: lockedP1 >= 0 && lockedP2 >= 0
            onClicked: {
                if (!stackViewRef) return
                var p1 = characters[lockedP1]
                var p2 = characters[lockedP2]
                var comp = Qt.createComponent("qrc:/qt/qml/FightGame/FightScreen.qml")
                if (comp.status === Component.Ready) {
                    stackViewRef.push(comp, {
                        "stackViewRef": stackViewRef,
                        "p1Name":     p1.name,
                        "p1Avatar":   p1.avatar,
                        "p1Portrait": p1.portrait,
                        "p1CharId":   p1.cid,
                        "p2Name":     p2.name,
                        "p2Avatar":   p2.avatar,
                        "p2Portrait": p2.portrait,
                        "p2CharId":   p2.cid
                    })
                }
            }

            contentItem: Text { text: btnFight.text; font.pixelSize: 18 * fitScale; font.bold: true; color: btnFight.enabled ? "silver" : "dimgray"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            background: Rectangle { color: btnFight.enabled ? (btnFight.hovered ? "black" : "black") : "black"; border.color: btnFight.enabled ? "darkred" : "black"; border.width: 2; radius: 4 }
        }

        Button {
            id: btnBack
            text: "BACK"; width: 120 * fitScale
            onClicked: { if (stackViewRef) stackViewRef.pop() }
            contentItem: Text { text: btnBack.text; font.pixelSize: 16 * fitScale; font.bold: true; color: "dimgray"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            background: Rectangle { color: "transparent"; border.color: btnBack.hovered ? "dimgray" : "dimgray"; border.width: 1; radius: 4 }
        }
    }
}
