// Module
// File: keybindingconfig.cpp   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-24 20:00:03
// Description:
//     键位配置管理器实现
#include "keybindingconfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QDebug>

KeyBindingConfig::KeyBindingConfig(QObject *parent)
    : QObject(parent)
{
    setDefaultBindings();
}

// 设置默认键位配置
void KeyBindingConfig::setDefaultBindings()
{
    // P1 默认键位
    m_p1Bindings["moveLeft"] = "A";
    m_p1Bindings["moveRight"] = "D";
    m_p1Bindings["jump"] = "W";
    m_p1Bindings["crouch"] = "S";
    m_p1Bindings["lightPunch"] = "J";
    m_p1Bindings["lightKick"] = "K";
    m_p1Bindings["heavyPunch"] = "U";
    m_p1Bindings["heavyKick"] = "L";
    m_p1Bindings["heavyStrike"] = "I";
    m_p1Bindings["block"] = "G";

    // P2 默认键位
    m_p2Bindings["moveLeft"] = "Left";
    m_p2Bindings["moveRight"] = "Right";
    m_p2Bindings["jump"] = "Up";
    m_p2Bindings["crouch"] = "Down";
    m_p2Bindings["lightPunch"] = "NumPad1";
    m_p2Bindings["lightKick"] = "NumPad2";
    m_p2Bindings["heavyPunch"] = "NumPad3";
    m_p2Bindings["heavyKick"] = "NumPad0";
    m_p2Bindings["heavyStrike"] = "NumPad5";
    m_p2Bindings["block"] = "NumPad4";
}

// 从 JSON 文件加载配置
bool KeyBindingConfig::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[KeyBindingConfig] Cannot open file:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "[KeyBindingConfig] JSON parse error:" << error.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "[KeyBindingConfig] JSON root is not an object";
        return false;
    }

    parseJson(doc.object());
    emit bindingsChanged();
    return true;
}

// 解析 JSON 对象
void KeyBindingConfig::parseJson(const QJsonObject &json)
{
    // 解析 P1 配置
    QJsonObject p1Obj = json.value("P1").toObject();
    if (!p1Obj.isEmpty()) {
        for (auto it = p1Obj.begin(); it != p1Obj.end(); ++it) {
            m_p1Bindings[it.key()] = it.value().toString();
        }
    }

    // 解析 P2 配置
    QJsonObject p2Obj = json.value("P2").toObject();
    if (!p2Obj.isEmpty()) {
        for (auto it = p2Obj.begin(); it != p2Obj.end(); ++it) {
            m_p2Bindings[it.key()] = it.value().toString();
        }
    }
}

// 获取指定玩家和动作的键位
QString KeyBindingConfig::getBinding(const QString &player, const QString &action) const
{
    if (player == "P1") {
        return m_p1Bindings.value(action, "");
    } else if (player == "P2") {
        return m_p2Bindings.value(action, "");
    }
    return "";
}

// 设置指定玩家和动作的键位
void KeyBindingConfig::setBinding(const QString &player, const QString &action, const QString &key)
{
    if (player == "P1") {
        m_p1Bindings[action] = key;
    } else if (player == "P2") {
        m_p2Bindings[action] = key;
    }
    emit bindingsChanged();
}

// 保存配置到文件
bool KeyBindingConfig::saveToFile(const QString &filePath)
{
    QJsonObject root;

    // 构建 P1 配置
    QJsonObject p1Obj;
    for (auto it = m_p1Bindings.begin(); it != m_p1Bindings.end(); ++it) {
        p1Obj[it.key()] = it.value();
    }
    root["P1"] = p1Obj;

    // 构建 P2 配置
    QJsonObject p2Obj;
    for (auto it = m_p2Bindings.begin(); it != m_p2Bindings.end(); ++it) {
        p2Obj[it.key()] = it.value();
    }
    root["P2"] = p2Obj;

    QJsonDocument doc(root);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[KeyBindingConfig] Cannot write file:" << filePath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    qDebug() << "[KeyBindingConfig] Saved to:" << filePath;
    return true;
}

// 重置为默认配置
void KeyBindingConfig::resetToDefault()
{
    setDefaultBindings();
    emit bindingsChanged();
}
