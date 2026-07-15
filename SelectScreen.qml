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
import FightGame

Item {
    id: root

    // 外部传入属性
    property var stackViewRef: null
    property bool isOnline: false
    property bool isHost: false
    property var networkMgr: null

    property real fitScale: Math.min(root.width / 900, root.height / 640)


    // 角色数据

    property var characters: [
        { cid: "Kusanagi", name: "草薙京", avatar: "qrc:/images/avatar/Kusanagi.jpg", portrait: "qrc:/images/portrait/Kusanagi.jpg" },
        { cid: "Kula", name: "库拉", avatar: "qrc:/images/avatar/Kula.jpg", portrait: "qrc:/images/portrait/Kula.jpg" },
        { cid: "Orochi", name: "大蛇", avatar: "qrc:/images/avatar/Orochi.jpg", portrait: "qrc:/images/portrait/Orochi.jpg" },
        { cid: "Yagami", name: "八神庵", avatar: "qrc:/images/avatar/Yagami.jpg", portrait: "qrc:/images/portrait/Yagami.jpg" },
        { cid: "Shiranui", name: "不知火舞", avatar: "qrc:/images/avatar/Shiranui.jpg", portrait: "qrc:/images/portrait/Shiranui.jpg" },
        { cid: "Kdash", name: "K", avatar: "qrc:/images/avatar/Kdash.jpg", portrait: "qrc:/images/portrait/Kdash.jpg" }
    ]


    // 选人状态

    property int  currentTurn: 1
    property int  previewP1: -1
    property int  previewP2: -1
    property int  lockedP1: -1
    property int  lockedP2: -1
    property bool fightReady: false
    property bool opponentFightReady: false

    // 头像网格边框颜色
    property string ac0: "black";   property string bc0: "dimgray"
    property string ac1: "black";   property string bc1: "dimgray"
    property string ac2: "black";   property string bc2: "dimgray"
    property string ac3: "black";   property string bc3: "dimgray"
    property string ac4: "black";   property string bc4: "dimgray"
    property string ac5: "black";   property string bc5: "dimgray"


    // 工具函数

    function findCharIdx(cid) {
        for (var i = 0; i < characters.length; i++) {
            if (characters[i].cid === cid)
                return i
        }
        return -1
    }

    function refresh() {
        var colors  = []
        var borders = []
        for (var i = 0; i < 6; i++) {
            if (i === lockedP1 || i === lockedP2) {
                colors[i]  = "black"
                borders[i] = "darkgoldenrod"
            } else if (i === previewP1 && currentTurn === 1) {
                colors[i]  = "black"
                borders[i] = "darkred"
            } else if (i === previewP2 && currentTurn === 2) {
                colors[i]  = "black"
                borders[i] = "midnightblue"
            } else {
                colors[i]  = "black"
                borders[i] = "dimgray"
            }
        }
        ac0 = colors[0]; bc0 = borders[0]
        ac1 = colors[1]; bc1 = borders[1]
        ac2 = colors[2]; bc2 = borders[2]
        ac3 = colors[3]; bc3 = borders[3]
        ac4 = colors[4]; bc4 = borders[4]
        ac5 = colors[5]; bc5 = borders[5]
    }


    // 战斗启动

    function startFightOnline() {
        if (!stackViewRef)
            return

        var p1   = characters[lockedP1]
        var p2   = characters[lockedP2]
        var comp = Qt.createComponent("qrc:/qt/qml/FightGame/StageSelectScreen.qml")

        if (comp.status !== Component.Ready)
            return

        stackViewRef.push(comp, {
            "stackViewRef": stackViewRef,
            "p1Name": p1.name,
            "p1Avatar": p1.avatar,
            "p1Portrait": p1.portrait,
            "p1CharId": p1.cid,
            "p2Name": p2.name,
            "p2Avatar": p2.avatar,
            "p2Portrait": p2.portrait,
            "p2CharId": p2.cid,
            "isOnline": true,
            "isHost": isHost,
            "networkMgr": networkMgr
        })
    }


    // 生命周期,信号和网络消息

    Component.onCompleted: {
        if (isOnline && !isHost)
            currentTurn = 0
        refresh()
    }

    onCurrentTurnChanged: refresh()
    onPreviewP1Changed: refresh()
    onPreviewP2Changed: refresh()
    onLockedP1Changed: refresh()
    onLockedP2Changed: refresh()

    Connections {
        target: networkMgr
        enabled: isOnline && networkMgr !== null

        function onMessageReceived(msg) {
            if (msg.type === "p1_select") {
                lockedP1 = findCharIdx(msg.cid)
                if (!isHost)
                    currentTurn = 2
            } else if (msg.type === "p2_select") {
                lockedP2 = findCharIdx(msg.cid)
                if (isHost)
                    currentTurn = 0
            } else if (msg.type === "fight_start") {
                opponentFightReady = true
                if (fightReady)
                    startFightOnline()
            }
        }
    }


    // 背景

    Rectangle {
        anchors.fill: parent
        color: "black"
    }


    // 标题

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 14 * fitScale
        text: "SELECT YOUR FIGHTER"
        font.pixelSize: 28 * fitScale
        font.bold: true
        color: "darkgoldenrod"
    }


    // P1 展示框

    Rectangle {
        x: parent.width * 0.07
        y: parent.height * 0.1
        width:  parent.width  * 0.34
        height: parent.height * 0.54
        color: "black"
        border.color: currentTurn === 1 && lockedP1 < 0 ? "darkred"
                     : (lockedP1 >= 0 ? "darkgoldenrod" : "black")
        border.width: 2
        radius: 6

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 8 * fitScale
            width:  parent.width  * 0.93
            height: parent.height * 0.78
            fillMode: Image.PreserveAspectFit
            source: (previewP1 >= 0 || lockedP1 >= 0)
                    ? characters[lockedP1 >= 0 ? lockedP1 : previewP1].portrait
                    : ""
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height * 0.85
            text: (lockedP1 >= 0 || previewP1 >= 0)
                  ? characters[lockedP1 >= 0 ? lockedP1 : previewP1].name
                  : ""
            font.pixelSize: 18 * fitScale
            font.bold: true
            color: "indianred"
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height * 0.92
            text: lockedP1 >= 0 ? "READY"
                  : (currentTurn === 1 ? "SELECTING" : "")
            font.pixelSize: 12 * fitScale
            color: lockedP1 >= 0 ? "darkgoldenrod" : "dimgray"
        }
    }


    // VS

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height * 0.33
        text: "VS"
        font.pixelSize: 40 * fitScale
        font.bold: true
        color: "darkgoldenrod"
    }


    // P2 展示框

    Rectangle {
        x: parent.width * 0.59
        y: parent.height * 0.1
        width:  parent.width  * 0.34
        height: parent.height * 0.54
        color: "black"
        border.color: currentTurn === 2 && lockedP2 < 0 ? "darkslateblue"
                     : (lockedP2 >= 0 ? "darkgoldenrod" : "midnightblue")
        border.width: 2
        radius: 6

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 8 * fitScale
            width:  parent.width  * 0.93
            height: parent.height * 0.78
            fillMode: Image.PreserveAspectFit
            source: (previewP2 >= 0 || lockedP2 >= 0)
                    ? characters[lockedP2 >= 0 ? lockedP2 : previewP2].portrait
                    : ""
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height * 0.85
            text: (lockedP2 >= 0 || previewP2 >= 0)
                  ? characters[lockedP2 >= 0 ? lockedP2 : previewP2].name
                  : ""
            font.pixelSize: 18 * fitScale
            font.bold: true
            color: "steelblue"
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height * 0.92
            text: lockedP2 >= 0 ? "READY"
                  : (currentTurn === 2 ? "SELECTING" : "")
            font.pixelSize: 12 * fitScale
            color: lockedP2 >= 0 ? "darkgoldenrod" : "dimgray"
        }
    }


    // 状态提示

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height * 0.67
        width:  200 * fitScale
        height: 30  * fitScale
        color: "black"
        radius: 4

        Text {
            id: statusHint
            anchors.centerIn: parent
            font.pixelSize: 14 * fitScale
            font.bold: true
            color: currentTurn === 1 ? "indianred"
                   : (currentTurn === 2 ? "steelblue" : "darkgoldenrod")

            text: {
                if (!isOnline)
                    return currentTurn === 1 ? "PLAYER 1 SELECT"
                         : (currentTurn === 2 ? "PLAYER 2 SELECT" : "READY")

                // 在线提示
                if (isHost) {
                    if (lockedP1 < 0)
                        return "请选择你的角色（P1）"
                    if (lockedP2 < 0)
                        return "等待对手选人…"
                    if (!fightReady)
                        return "点击 FIGHT 准备战斗"
                    if (!opponentFightReady)
                        return "等待对手准备…"
                    return "进入战斗！"
                } else {
                    if (lockedP1 < 0)
                        return "等待对手选人…"
                    if (lockedP2 < 0)
                        return "请选择你的角色（P2）"
                    if (!fightReady)
                        return "点击 FIGHT 准备战斗"
                    if (!opponentFightReady)
                        return "等待对手准备…"
                    return "进入战斗！"
                }
            }
        }
    }


    // 头像网格

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 60 * fitScale
        spacing: 10 * fitScale

        Repeater {
            model: 6

            Rectangle {
                width:  100 * fitScale
                height: 100 * fitScale
                color: root["ac" + index]
                border.color: root["bc" + index]
                border.width: 2
                radius: 4

                Image {
                    anchors.centerIn: parent
                    width:  88 * fitScale
                    height: 88 * fitScale
                    fillMode: Image.PreserveAspectFit
                    source: characters[index].avatar
                }

                TapHandler {
                    gesturePolicy: TapHandler.ReleaseWithinBounds
                    cursorShape: Qt.PointingHandCursor
                    onTapped: {
                        if (currentTurn === 0)
                            return
                        if (currentTurn === 1 && lockedP1 >= 0)
                            return
                        if (currentTurn === 2 && lockedP2 >= 0)
                            return
                        if (currentTurn === 1)
                            previewP1 = index
                        else
                            previewP2 = index
                    }
                }
            }
        }
    }


    // 底部按钮

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16 * fitScale
        spacing: 20 * fitScale

        // ---- CONFIRM ----
        Button {
            id: btnConfirm
            text: "CONFIRM"
            width: 120 * fitScale
            enabled: (currentTurn === 1 && previewP1 >= 0)
                  || (currentTurn === 2 && previewP2 >= 0)

            onClicked: {
                if (isOnline) {
                    if (isHost && currentTurn === 1) {
                        lockedP1 = previewP1
                        previewP1 = -1
                        currentTurn = 0
                        var p1 = characters[lockedP1]
                        networkMgr.sendMessage({
                            "type": "p1_select",
                            "cid": p1.cid,
                            "name": p1.name,
                            "avatar": p1.avatar,
                            "portrait": p1.portrait
                        })
                    } else if (!isHost && currentTurn === 2) {
                        lockedP2 = previewP2
                        previewP2 = -1
                        currentTurn = 0
                        var p2 = characters[lockedP2]
                        networkMgr.sendMessage({
                            "type": "p2_select",
                            "cid": p2.cid,
                            "name": p2.name,
                            "avatar": p2.avatar,
                            "portrait": p2.portrait
                        })
                    }
                } else {
                    if (currentTurn === 1) {
                        lockedP1 = previewP1
                        previewP1 = -1
                        currentTurn = 2
                    } else if (currentTurn === 2) {
                        lockedP2 = previewP2
                        previewP2 = -1
                        currentTurn = 0
                    }
                }
            }

            contentItem: Text {
                text: btnConfirm.text
                font.pixelSize: 16 * fitScale
                font.bold: true
                color: btnConfirm.enabled ? "silver" : "dimgray"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                color: "black"
                border.color: btnConfirm.enabled ? "darkgreen" : "black"
                border.width: 2
                radius: 4
            }
        }

        //  FIGHT
        Button {
            id: btnFight
            text: fightReady ? (opponentFightReady ? "FIGHT" : "WAITING...") : "FIGHT"
            width: 160 * fitScale
            enabled: lockedP1 >= 0 && lockedP2 >= 0 && !fightReady

            onClicked: {
                if (isOnline) {
                    fightReady = true
                    networkMgr.sendMessage({ "type": "fight_start" })
                    if (opponentFightReady)
                        startFightOnline()
                } else {
                    if (!stackViewRef)
                        return
                    var p1   = characters[lockedP1]
                    var p2   = characters[lockedP2]
                    var comp = Qt.createComponent("qrc:/qt/qml/FightGame/StageSelectScreen.qml")
                    if (comp.status !== Component.Ready)
                        return
                    stackViewRef.push(comp, {
                        "stackViewRef": stackViewRef,
                        "p1Name": p1.name,
                        "p1Avatar": p1.avatar,
                        "p1Portrait": p1.portrait,
                        "p1CharId": p1.cid,
                        "p2Name": p2.name,
                        "p2Avatar": p2.avatar,
                        "p2Portrait": p2.portrait,
                        "p2CharId": p2.cid
                    })
                }
            }

            contentItem: Text {
                text: btnFight.text
                font.pixelSize: 18 * fitScale
                font.bold: true
                color: (fightReady && !opponentFightReady) ? "darkgoldenrod"
                       : (btnFight.enabled ? "silver" : "dimgray")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                color: "black"
                border.color: btnFight.enabled ? "darkred"
                              : (fightReady ? "darkgoldenrod" : "black")
                border.width: 2
                radius: 4
            }
        }

        // BACK
        Button {
            id: btnBack
            text: "BACK"
            width: 120 * fitScale
            onClicked: {
                if (stackViewRef)
                    stackViewRef.pop()
            }

            contentItem: Text {
                text: btnBack.text
                font.pixelSize: 16 * fitScale
                font.bold: true
                color: "dimgray"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                color: "transparent"
                border.color: "dimgray"
                border.width: 1
                radius: 4
            }
        }
    }
}
