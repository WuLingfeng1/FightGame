// Module
// File: KeyBindingScreen.qml   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-24 19:20:12
// Description:
//     键位设置界面，允许用户自定义 P1 和 P2 的键位
import QtQuick
import QtQuick.Controls
import FightGame

Item {
    id: root

    property var stackViewRef: null
    property var keyBindingConfig: null  // 由外部传入
    property bool waitingForKey: false
    property string waitingPlayer: ""
    property string waitingAction: ""

    // 刷新计数器，用于触发 UI 更新
    property int refreshCounter: 0

    Component.onCompleted: {
        if (keyBindingConfig) {
            keyBindingConfig.bindingsChanged.connect(function() { root.refreshCounter++ })
            root.refreshCounter++  // 手动触发一次刷新
        }
    }

    // 功能名称映射
    property var actionNames: {
        "moveLeft": "左移", "moveRight": "右移", "jump": "跳跃", "crouch": "下蹲",
        "lightPunch": "轻拳", "lightKick": "轻腿", "heavyPunch": "重拳", "heavyKick": "重腿",
        "heavyStrike": "超重击", "block": "防御"
    }

    // 功能列表
    property var actionList: ["moveLeft", "moveRight", "jump", "crouch", "lightPunch", "lightKick", "heavyPunch", "heavyKick", "heavyStrike", "block"]

    // 键位显示名称映射
    function keyDisplayName(key) {
        var nameMap = {
            "Left": "←", "Right": "→", "Up": "↑", "Down": "↓",
            "NumPad0": "小键盘0", "NumPad1": "小键盘1", "NumPad2": "小键盘2",
            "NumPad3": "小键盘3", "NumPad4": "小键盘4", "NumPad5": "小键盘5",
            "NumPad6": "小键盘6", "NumPad7": "小键盘7", "NumPad8": "小键盘8", "NumPad9": "小键盘9",
            "Space": "空格", "Return": "回车", "Escape": "Esc",
            "Shift": "Shift", "Control": "Ctrl", "Alt": "Alt"
        }
        return nameMap[key] || key
    }

    // 开始等待按键
    function startWaitingKey(player, action) {
        waitingForKey = true
        waitingPlayer = player
        waitingAction = action
        focus = true
    }

    // 停止等待按键
    function stopWaitingKey() {
        waitingForKey = false
        waitingPlayer = ""
        waitingAction = ""
    }

    // 返回按钮
    Button {
        id: btnBack
        text: "返回"
        width: 80; height: 30
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20

        contentItem: Text {
            text: btnBack.text; font.pixelSize: 14; color: "black"
            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            color: btnBack.hovered ? "lightgray" : "white"
            border.color: btnBack.hovered ? "dimgray" : "gray"
            border.width: 1; radius: 2
        }

        onClicked: {
            if (stackViewRef) stackViewRef.pop()
        }
    }

    // 标题
    Text {
        text: "键位设置"
        font.pixelSize: 28; font.bold: true; color: "black"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 20
    }

    // 提示文本
    Text {
        id: hintText
        text: waitingForKey ? "请按下新的键位..." : "点击键位按钮进行修改"
        font.pixelSize: 14; color: waitingForKey ? "red" : "dimgray"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 60
    }

    // P1 键位配置区域
    Column {
        id: p1Column
        anchors.left: parent.left
        anchors.leftMargin: 80
        anchors.top: parent.top
        anchors.topMargin: 100
        spacing: 10

        Text {
            text: "P1 键位"
            font.pixelSize: 18; font.bold: true; color: "darkred"
        }

        Repeater {
            model: actionList

            Row {
                spacing: 10

                Text {
                    text: actionNames[modelData]
                    font.pixelSize: 14; color: "black"
                    width: 80
                    verticalAlignment: Text.AlignVCenter
                    height: 30
                }

                Button {
                    id: p1Btn
                    text: { refreshCounter; return keyDisplayName(keyBindingConfig.getBinding("P1", modelData)) }
                    width: 100; height: 30

                    contentItem: Text {
                        text: p1Btn.text; font.pixelSize: 12; color: "black"
                        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: p1Btn.hovered ? "lightgray" : "white"
                        border.color: p1Btn.hovered ? "dimgray" : "gray"
                        border.width: 1; radius: 2
                    }

                    onClicked: {
                        startWaitingKey("P1", modelData)
                    }
                }
            }
        }
    }

    // P2 键位配置区域
    Column {
        id: p2Column
        anchors.right: parent.right
        anchors.rightMargin: 80
        anchors.top: parent.top
        anchors.topMargin: 100
        spacing: 10

        Text {
            text: "P2 键位"
            font.pixelSize: 18; font.bold: true; color: "darkblue"
        }

        Repeater {
            model: actionList

            Row {
                spacing: 10

                Text {
                    text: actionNames[modelData]
                    font.pixelSize: 14; color: "black"
                    width: 80
                    verticalAlignment: Text.AlignVCenter
                    height: 30
                }

                Button {
                    id: p2Btn
                    text: { refreshCounter; return keyDisplayName(keyBindingConfig.getBinding("P2", modelData)) }
                    width: 100; height: 30

                    contentItem: Text {
                        text: p2Btn.text; font.pixelSize: 12; color: "black"
                        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: p2Btn.hovered ? "lightgray" : "white"
                        border.color: p2Btn.hovered ? "dimgray" : "gray"
                        border.width: 1; radius: 2
                    }

                    onClicked: {
                        startWaitingKey("P2", modelData)
                    }
                }
            }
        }
    }

    // 底部按钮区域
    Row {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 20

        Button {
            id: btnReset
            text: "重置为默认"
            width: 120; height: 35

            contentItem: Text {
                text: btnReset.text; font.pixelSize: 14; color: "black"
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                color: btnReset.hovered ? "lightgray" : "white"
                border.color: btnReset.hovered ? "dimgray" : "gray"
                border.width: 1; radius: 2
            }

            onClicked: {
                keyBindingConfig.resetToDefault()
                keyBindingConfig.saveToFile("/wlf/FightGame/config/keybindings.json")
            }
        }

        Button {
            id: btnSave
            text: "保存"
            width: 100; height: 35

            contentItem: Text {
                text: btnSave.text; font.pixelSize: 14; color: "black"
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                color: btnSave.hovered ? "lightgray" : "white"
                border.color: btnSave.hovered ? "dimgray" : "gray"
                border.width: 1; radius: 2
            }

            onClicked: {
                keyBindingConfig.saveToFile("/wlf/FightGame/config/keybindings.json")
                if (stackViewRef) stackViewRef.pop()
            }
        }
    }

    // 键盘事件处理
    Keys.onPressed: (event) => {
        if (!waitingForKey) return

        var keyName = ""
        if (event.key >= Qt.Key_A && event.key <= Qt.Key_Z) {
            keyName = String.fromCharCode(event.key)
        } else if (event.key >= Qt.Key_0 && event.key <= Qt.Key_9) {
            keyName = String.fromCharCode(event.key)
        } else if (event.key === Qt.Key_Left) {
            keyName = "Left"
        } else if (event.key === Qt.Key_Right) {
            keyName = "Right"
        } else if (event.key === Qt.Key_Up) {
            keyName = "Up"
        } else if (event.key === Qt.Key_Down) {
            keyName = "Down"
        } else if (event.key === Qt.Key_Space) {
            keyName = "Space"
        } else if (event.key === Qt.Key_Return) {
            keyName = "Return"
        } else if (event.key === Qt.Key_Escape) {
            keyName = "Escape"
        } else if (event.key === Qt.Key_Shift) {
            keyName = "Shift"
        } else if (event.key === Qt.Key_Control) {
            keyName = "Control"
        } else if (event.key === Qt.Key_Alt) {
            keyName = "Alt"
        } else if (event.modifiers & Qt.KeypadModifier) {
            // 小键盘数字键
            if (event.key >= Qt.Key_0 && event.key <= Qt.Key_9) {
                keyName = "NumPad" + String.fromCharCode(event.key)
            }
        }

        if (keyName !== "") {
            keyBindingConfig.setBinding(waitingPlayer, waitingAction, keyName)
            stopWaitingKey()
        }
    }
}
