// musicmanager.h — 背景音乐单例: 选人/场景/胜利音乐
#pragma once
#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

class QMediaPlayer;
class QAudioOutput;
class QQmlEngine;
class QJSEngine;

class MusicManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    QML_ELEMENT
    QML_SINGLETON
public:
    static MusicManager *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static MusicManager *instance();

    Q_INVOKABLE void playSelect();                       // 选人/选图界面音乐(循环)
    Q_INVOKABLE void playStage(const QString &stageId);   // 战斗场景音乐(循环)
    Q_INVOKABLE void playVictory();                       // 胜利音乐(单次)
    Q_INVOKABLE void stop();

    qreal volume() const { return m_volume; }
    void setVolume(qreal v);
    bool enabled() const { return m_enabled; }
    void setEnabled(bool on);
signals:
    void volumeChanged();
    void enabledChanged();
private:
    explicit MusicManager(QObject *parent = nullptr);
    ~MusicManager() override;
    void playTrack(const QString &resPath, const QString &cacheName, bool loop);

    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audio = nullptr;
    qreal m_volume = 0.5;
    bool m_enabled = true;
    QString m_currentTrack;
    static MusicManager *s_instance;
};
