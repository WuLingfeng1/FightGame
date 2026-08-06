// Module
// File: voicemanager.cpp   Version: 0.1.0   License: AGPLv3
// 播放后端: QMediaPlayer(FFmpeg) — 异步加载后在 LoadedMedia 回调中播放, 确保不丢失
#include "voicemanager.h"

#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDebug>
#include <QSet>

static qreal actionVolume(const QString &action)
{
    static const QHash<QString, qreal> vol = {
        {QStringLiteral("swing"), 0.5},{QStringLiteral("swingPunch"),0.5},{QStringLiteral("swingKick"),0.5},
        {QStringLiteral("swingHeavyPunch"),0.5},{QStringLiteral("swingHeavyKick"),0.5},{QStringLiteral("swingStrike"),0.5},
        {QStringLiteral("punch"),0.7},{QStringLiteral("kick"),0.7},{QStringLiteral("heavyPunch"),0.7},
        {QStringLiteral("heavyKick"),0.7},{QStringLiteral("strike"),0.8},{QStringLiteral("hitPunch"),1.0},
        {QStringLiteral("hitKick"),1.0},{QStringLiteral("hitHeavyPunch"),1.0},{QStringLiteral("hitHeavyKick"),1.0},
        {QStringLiteral("hitStrike"),1.0},{QStringLiteral("hurt"),0.7},{QStringLiteral("jump"),0.7},
        {QStringLiteral("dodge"),0.7},{QStringLiteral("step"),0.7},{QStringLiteral("opening"),1.0},{QStringLiteral("win"),1.0},
    };
    return vol.value(action, 1.0);
}

static QString unpackToLocal(const QString &path)
{
    if (!path.startsWith(QLatin1String(":/"))) return path;
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/voices");
    QDir().mkpath(dir);
    QString local = dir + QLatin1Char('/') + path.mid(2).replace(QLatin1Char('/'), QLatin1Char('_'));
    QFile in(path), out(local);
    if (in.open(QIODevice::ReadOnly) && out.open(QIODevice::WriteOnly | QIODevice::Truncate))
        out.write(in.readAll());
    else return QString();
    return local;
}

VoiceManager *VoiceManager::s_instance = nullptr;
VoiceManager &VoiceManager::instance() {
    if (!s_instance) s_instance = new VoiceManager;
    return *s_instance;
}
VoiceManager::VoiceManager(QObject *parent) : QObject(parent) {
    if (!s_instance) s_instance = this;
}

void VoiceManager::setVolume(qreal v) {
    v = qBound<qreal>(0.0, v, 1.0);
    if (qFuzzyCompare(m_volume, v)) return;
    m_volume = v;
    for (QMediaPlayer *p : std::as_const(m_pool))
        if (auto *ao = p->audioOutput()) ao->setVolume(v);
    emit volumeChanged();
}
void VoiceManager::setEnabled(bool on) {
    if (m_enabled == on) return;
    m_enabled = on;
    if (!on) for (QMediaPlayer *p : std::as_const(m_pool)) p->stop();
    emit enabledChanged();
}

QStringList VoiceManager::variants(const QString &charId, const QString &action) {
    const QString key = charId + QLatin1Char('/') + action;
    if (m_variantsCache.contains(key)) return m_variantsCache.value(key);
    QStringList files;
    const QStringList roots = {
        QStringLiteral(":/resources/voices/%1").arg(charId),
        QStringLiteral(":/voices/%1").arg(charId),
        QCoreApplication::applicationDirPath() + QStringLiteral("/voices/%1").arg(charId),
        QDir::currentPath() + QStringLiteral("/resources/voices/%1").arg(charId),
    };
    QSet<QString> seen;
    for (const QString &root : roots) {
        QDir dir(root);
        if (!dir.exists()) continue;
        for (const QString &entry : dir.entryList(QDir::Files | QDir::Readable, QDir::Name)) {
            if (!entry.startsWith(action) || !entry.endsWith(QLatin1String(".wav"))) continue;
            QString num = entry.mid(action.size());
            num = num.left(num.indexOf(QLatin1Char('.')));
            bool ok = false; num.toInt(&ok);
            if (!ok) continue;
            const QString path = root + QLatin1Char('/') + entry;
            if (!seen.contains(path)) { seen.insert(path); files.append(path); }
        }
    }
    if (files.isEmpty()) {
        static QSet<QString> s_warned;
        if (!s_warned.contains(key)) { s_warned.insert(key); qWarning() << "[VoiceManager] no voices for" << key; }
    }
    files.sort();
    m_variantsCache.insert(key, files);
    return files;
}

QStringList VoiceManager::localVariants(const QString &charId, const QString &action) {
    QStringList locals;
    for (const QString &p : variants(charId, action)) {
        const QString local = unpackToLocal(p);
        if (!local.isEmpty()) locals.append(local);
    }
    return locals;
}

QMediaPlayer *VoiceManager::acquireEffect() {
    for (QMediaPlayer *p : std::as_const(m_pool))
        if (!m_busy.contains(p)) return p;
    if (m_pool.size() >= 8) return m_pool.at(0);
    auto *p = new QMediaPlayer;
    auto *ao = new QAudioOutput;
    ao->setVolume(m_volume);
    p->setAudioOutput(ao);
    m_pool.append(p);
    return p;
}

bool VoiceManager::hasVoice(const QString &charId, const QString &action) {
    return !variants(charId, action).isEmpty();
}

QStringList VoiceManager::voiceFiles(const QString &charId) {
    QStringList all;
    for (const QString &a : {QStringLiteral("opening"),QStringLiteral("punch"),QStringLiteral("kick"),
                             QStringLiteral("heavyPunch"),QStringLiteral("heavyKick"),QStringLiteral("strike"),
                             QStringLiteral("special"),QStringLiteral("hurt"),QStringLiteral("jump"),
                             QStringLiteral("dodge"),QStringLiteral("win")}) {
        for (const QString &p : variants(charId, a)) all << p;
    }
    return all;
}

void VoiceManager::play(const QString &charId, const QString &action) {
    if (!m_enabled || charId.isEmpty() || action.isEmpty()) return;
    const QStringList files = localVariants(charId, action);
    if (files.isEmpty()) return;
    QMediaPlayer *p = acquireEffect();
    if (!p) return;
    m_busy.insert(p);
    p->stop();
    const QString file = files.at(QRandomGenerator::global()->bounded(files.size()));
    if (auto *ao = p->audioOutput())
        ao->setVolume(m_volume * actionVolume(action));
    // 等待加载完成再播放 —— 避免 play() 在加载中被忽略
    p->setSource(QUrl::fromLocalFile(file));
    QMetaObject::Connection *conn = new QMetaObject::Connection;
    *conn = connect(p, &QMediaPlayer::mediaStatusChanged, p, [this, p, conn](QMediaPlayer::MediaStatus s) {
        if (s == QMediaPlayer::LoadedMedia || s == QMediaPlayer::BufferedMedia) {
            p->play();
            disconnect(*conn); delete conn;
        } else if (s == QMediaPlayer::EndOfMedia) {
            m_busy.remove(p);
            p->stop();
        }
    });
}
