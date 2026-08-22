// musicmanager.cpp — 背景音乐单例: 按轨道名加载 qrc 资源, 解包到本地缓存后播放
#include "musicmanager.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QQmlEngine>
#include <QJSEngine>
#include <QDebug>

static QString unpackToLocal(const QString &resPath, const QString &cacheName)
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QStringLiteral("/bgm");
    QDir().mkpath(dir);
    const QString local = dir + QLatin1Char('/') + cacheName + QStringLiteral(".flac");
    if (QFileInfo::exists(local)) return local;

    QFile in(resPath), out(local);
    if (in.open(QIODevice::ReadOnly) && out.open(QIODevice::WriteOnly | QIODevice::Truncate))
        out.write(in.readAll());
    else
        return QString();
    return local;
}

MusicManager *MusicManager::s_instance = nullptr;

MusicManager *MusicManager::instance()
{
    if (!s_instance) s_instance = new MusicManager;
    return s_instance;
}

MusicManager *MusicManager::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine);
    Q_UNUSED(jsEngine);
    MusicManager *m = instance();
    QJSEngine::setObjectOwnership(m, QJSEngine::CppOwnership);
    return m;
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

void MusicManager::playTrack(const QString &resPath, const QString &cacheName, bool loop)
{
    if (!m_enabled || resPath.isEmpty()) return;
    if (m_currentTrack == cacheName) return;
    if (!QFileInfo::exists(resPath)) {
        qInfo() << "[MusicManager] no track" << resPath;
        return;
    }
    const QString local = unpackToLocal(resPath, cacheName);
    if (local.isEmpty()) {
        qWarning() << "[MusicManager] failed to unpack" << resPath;
        return;
    }
    m_currentTrack = cacheName;
    m_player->setLoops(loop ? QMediaPlayer::Infinite : 1);
    m_player->setSource(QUrl::fromLocalFile(local));
    m_player->play();
}

void MusicManager::playBegin()
{
    playTrack(QStringLiteral(":/resources/select/begin.flac"), QStringLiteral("begin"), true);
}

void MusicManager::playSelect()
{
    playTrack(QStringLiteral(":/resources/select/select.flac"), QStringLiteral("select"), true);
}

void MusicManager::playStage(const QString &stageId)
{
    if (stageId.isEmpty()) return;
    playTrack(QStringLiteral(":/resources/BackGround/") + stageId + QStringLiteral(".flac"), stageId, true);
}

void MusicManager::playVictory()
{
    playTrack(QStringLiteral(":/resources/select/victory.flac"), QStringLiteral("victory"), false);
}

void MusicManager::stop()
{
    m_currentTrack.clear();
    if (m_player) m_player->stop();
}
