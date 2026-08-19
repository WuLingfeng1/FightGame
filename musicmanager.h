// musicmanager.h — 背景音乐播放, 按 stageId 循环播放对应 flac
#pragma once
#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

class QMediaPlayer;
class QAudioOutput;

class MusicManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    QML_ELEMENT
public:
    explicit MusicManager(QObject *parent = nullptr);
    ~MusicManager() override;

    Q_INVOKABLE void playStage(const QString &stageId);
    Q_INVOKABLE void stop();

    qreal volume() const { return m_volume; }
    void setVolume(qreal v);
    bool enabled() const { return m_enabled; }
    void setEnabled(bool on);
signals:
    void volumeChanged();
    void enabledChanged();
private:
    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audio = nullptr;
    qreal m_volume = 0.5;
    bool m_enabled = true;
};
