// Module
// File: fightermodel.h   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-10 19:00:39
// Description:
//
#pragma once

#include <QObject>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

class FighterModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString charId READ charId WRITE setCharId NOTIFY charIdChanged)
    Q_PROPERTY(int currentFrame READ currentFrame NOTIFY currentFrameChanged)
    Q_PROPERTY(int totalFrames READ totalFrames WRITE setTotalFrames NOTIFY totalFramesChanged)

    QML_ELEMENT
public:
    explicit FighterModel(QObject *parent = nullptr);
    ~FighterModel() override;

    QString charId() const;
    int currentFrame() const;
    int totalFrames() const;
    void setTotalFrames(int n);

public slots:
    void setCharId(const QString &id);

signals:
    void charIdChanged();
    void currentFrameChanged();
    void totalFramesChanged();

private slots:
    void onFrameTick();

private:
    QString m_charId;
    int     m_currentFrame = 0;
    int     m_totalFrames = 9;
    QTimer *m_frameTimer = nullptr;

    static constexpr int m_frameInterval = 111;
};
