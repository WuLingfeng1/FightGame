// Module
// File: keybindingconfig.h   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-24 19:47:21
// Description:
//     键位配置管理器，从外部 JSON 文件加载键位配置
#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QMap>
#include <QtQml/qqmlregistration.h>

// 键位配置管理器
class KeyBindingConfig : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString p1MoveLeft READ p1MoveLeft NOTIFY bindingsChanged)
    Q_PROPERTY(QString p1MoveRight READ p1MoveRight NOTIFY bindingsChanged)
    Q_PROPERTY(QString p1Jump READ p1Jump NOTIFY bindingsChanged)
    Q_PROPERTY(QString p1Crouch READ p1Crouch NOTIFY bindingsChanged)
    Q_PROPERTY(QString p1LightPunch READ p1LightPunch NOTIFY bindingsChanged)
    Q_PROPERTY(QString p1LightKick READ p1LightKick NOTIFY bindingsChanged)
    Q_PROPERTY(QString p1HeavyPunch READ p1HeavyPunch NOTIFY bindingsChanged)
    Q_PROPERTY(QString p1HeavyKick READ p1HeavyKick NOTIFY bindingsChanged)
    Q_PROPERTY(QString p1HeavyStrike READ p1HeavyStrike NOTIFY bindingsChanged)
    Q_PROPERTY(QString p1Block READ p1Block NOTIFY bindingsChanged)

    Q_PROPERTY(QString p2MoveLeft READ p2MoveLeft NOTIFY bindingsChanged)
    Q_PROPERTY(QString p2MoveRight READ p2MoveRight NOTIFY bindingsChanged)
    Q_PROPERTY(QString p2Jump READ p2Jump NOTIFY bindingsChanged)
    Q_PROPERTY(QString p2Crouch READ p2Crouch NOTIFY bindingsChanged)
    Q_PROPERTY(QString p2LightPunch READ p2LightPunch NOTIFY bindingsChanged)
    Q_PROPERTY(QString p2LightKick READ p2LightKick NOTIFY bindingsChanged)
    Q_PROPERTY(QString p2HeavyPunch READ p2HeavyPunch NOTIFY bindingsChanged)
    Q_PROPERTY(QString p2HeavyKick READ p2HeavyKick NOTIFY bindingsChanged)
    Q_PROPERTY(QString p2HeavyStrike READ p2HeavyStrike NOTIFY bindingsChanged)
    Q_PROPERTY(QString p2Block READ p2Block NOTIFY bindingsChanged)

public:
    explicit KeyBindingConfig(QObject *parent = nullptr);

    // 从 JSON 文件加载配置
    Q_INVOKABLE bool loadFromFile(const QString &filePath);

    // P1 键位访问器
    QString p1MoveLeft() const { return m_p1Bindings["moveLeft"]; }
    QString p1MoveRight() const { return m_p1Bindings["moveRight"]; }
    QString p1Jump() const { return m_p1Bindings["jump"]; }
    QString p1Crouch() const { return m_p1Bindings["crouch"]; }
    QString p1LightPunch() const { return m_p1Bindings["lightPunch"]; }
    QString p1LightKick() const { return m_p1Bindings["lightKick"]; }
    QString p1HeavyPunch() const { return m_p1Bindings["heavyPunch"]; }
    QString p1HeavyKick() const { return m_p1Bindings["heavyKick"]; }
    QString p1HeavyStrike() const { return m_p1Bindings["heavyStrike"]; }
    QString p1Block() const { return m_p1Bindings["block"]; }

    // P2 键位访问器
    QString p2MoveLeft() const { return m_p2Bindings["moveLeft"]; }
    QString p2MoveRight() const { return m_p2Bindings["moveRight"]; }
    QString p2Jump() const { return m_p2Bindings["jump"]; }
    QString p2Crouch() const { return m_p2Bindings["crouch"]; }
    QString p2LightPunch() const { return m_p2Bindings["lightPunch"]; }
    QString p2LightKick() const { return m_p2Bindings["lightKick"]; }
    QString p2HeavyPunch() const { return m_p2Bindings["heavyPunch"]; }
    QString p2HeavyKick() const { return m_p2Bindings["heavyKick"]; }
    QString p2HeavyStrike() const { return m_p2Bindings["heavyStrike"]; }
    QString p2Block() const { return m_p2Bindings["block"]; }

    // 获取指定玩家和动作的键位
    Q_INVOKABLE QString getBinding(const QString &player, const QString &action) const;

    // 设置指定玩家和动作的键位
    Q_INVOKABLE void setBinding(const QString &player, const QString &action, const QString &key);

    // 保存配置到文件
    Q_INVOKABLE bool saveToFile(const QString &filePath);

    // 重置为默认配置
    Q_INVOKABLE void resetToDefault();

signals:
    void bindingsChanged();

private:
    QMap<QString, QString> m_p1Bindings;
    QMap<QString, QString> m_p2Bindings;

    void setDefaultBindings();
    void parseJson(const QJsonObject &json);
};
