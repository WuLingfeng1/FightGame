// Module
// File: fightermodel.h   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-10 19:00:39
// Description:
//     The standing idle animation is finished
#pragma once

#include <QObject>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

class FighterModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString charId READ charId WRITE setCharId NOTIFY charIdChanged)
    Q_PROPERTY(int currentFrame READ currentFrame NOTIFY currentFrameChanged)

    QML_ELEMENT
public:
    explicit FighterModel(QObject *parent = nullptr);
    ~FighterModel() override;

    QString charId() const;
    int currentFrame() const;

public slots:
    void setCharId(const QString &id);

signals:
    void charIdChanged();
    void currentFrameChanged();

private slots:
    void onFrameTick();

private:
    QString m_charId;
    int     m_currentFrame = 0;
    QTimer *m_frameTimer = nullptr;

    // 当前 9 帧、111ms 保持一致
    const int m_totalFrames = 9;
    const int m_frameInterval = 111;
};
