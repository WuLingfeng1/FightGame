// musicmanager.cpp — 背景音乐: 按 stageId 加载 qrc 资源, 解包到本地缓存后循环播放
#include "musicmanager.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

static QString unpackToLocal(const QString &stageId)
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QStringLiteral("/bgm");
    QDir().mkpath(dir);
    const QString local = dir + QLatin1Char('/') + stageId + QStringLiteral(".flac");
    if (QFileInfo::exists(local)) return local;

    const QString res = QStringLiteral(":/resources/BackGround/") + stageId + QStringLiteral(".flac");
    QFile in(res), out(local);
    if (in.open(QIODevice::ReadOnly) && out.open(QIODevice::WriteOnly | QIODevice::Truncate))
        out.write(in.readAll());
    else
        return QString();
    return local;
}

MusicManager::MusicManager(QObject *parent)
    : QObject(parent)
{
    m_player = new QMediaPlayer(this);
    m_audio = new QAudioOutput(this);
    m_audio->setVolume(m_volume);
    m_player->setAudioOutput(m_audio);
    m_player->setLoops(QMediaPlayer::Infinite);
}

MusicManager::~MusicManager() = default;

void MusicManager::setVolume(qreal v)
{
    v = qBound<qreal>(0.0, v, 1.0);
    if (qFuzzyCompare(m_volume, v)) return;
    m_volume = v;
    if (m_audio) m_audio->setVolume(m_volume);
    emit volumeChanged();
}

void MusicManager::setEnabled(bool on)
{
    if (m_enabled == on) return;
    m_enabled = on;
    if (!m_enabled) stop();
    emit enabledChanged();
}

void MusicManager::playStage(const QString &stageId)
{
    if (!m_enabled || stageId.isEmpty()) return;
    const QString res = QStringLiteral(":/resources/BackGround/") + stageId + QStringLiteral(".flac");
    if (!QFileInfo::exists(res)) {
        qInfo() << "[MusicManager] no bgm for stage" << stageId;
        return;
    }
    const QString local = unpackToLocal(stageId);
    if (local.isEmpty()) {
        qWarning() << "[MusicManager] failed to unpack bgm for stage" << stageId;
        return;
    }
    m_player->setSource(QUrl::fromLocalFile(local));
    m_player->play();
}

void MusicManager::stop()
{
    if (m_player) m_player->stop();
}
