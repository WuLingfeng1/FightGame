// voicemanager.h — QMediaPlayer, 每次play独立建player(无竞争)
#pragma once
#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QStringList>
#include <QHash>
#include <QRandomGenerator>
#include <QtQml/qqmlregistration.h>

class VoiceManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    QML_ELEMENT
public:
    explicit VoiceManager(QObject *parent = nullptr);
    static VoiceManager &instance();
    Q_INVOKABLE void play(const QString &charId, const QString &action);
    Q_INVOKABLE bool hasVoice(const QString &charId, const QString &action);
    Q_INVOKABLE QStringList voiceFiles(const QString &charId);
    qreal volume() const { return m_volume; }
    void setVolume(qreal v);
    bool enabled() const { return m_enabled; }
    void setEnabled(bool on);
signals: void volumeChanged(); void enabledChanged();
private:
    QStringList variants(const QString &charId, const QString &action);
    QStringList localVariants(const QString &charId, const QString &action);
    QHash<QString, QStringList> m_variantsCache;
    qreal m_volume = 1.0;
    bool m_enabled = true;
    static VoiceManager *s_instance;
};
