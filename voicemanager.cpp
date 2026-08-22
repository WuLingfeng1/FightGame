// Module
// File: voicemanager.cpp   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-08-22 17:02:24
// Description:
//       每次play新建独立QMediaPlayer, 不竞争
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
        {"swing",          0.5},
        {"swingPunch",     0.5},
        {"swingKick",      0.5},
        {"swingHeavyPunch", 0.5},
        {"swingHeavyKick", 0.5},
        {"swingStrike",    0.5},
        {"punch",          0.7},
        {"kick",           0.7},
        {"heavyPunch",     0.7},
        {"heavyKick",      0.7},
        {"strike",         0.8},
        {"hitPunch",       1.0},
        {"hitKick",        1.0},
        {"hitHeavyPunch",  1.0},
        {"hitHeavyKick",   1.0},
        {"hitStrike",      1.0},
        {"hurt",           0.7},
        {"jump",           0.7},
        {"dodge",          0.7},
        {"step",           0.7},
        {"opening",        1.0},
        {"win",            1.0},
    };
    return vol.value(action, 1.0);
}

static QString unpackToLocal(const QString &path)
{
    if (!path.startsWith(QLatin1String(":/")))
        return path;

    const QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/voices";
    QDir().mkpath(dir);

    const QString local = dir + QLatin1Char('/') + path.mid(2).replace('/', '_');

    QFile in(path), out(local);
    if (in.open(QIODevice::ReadOnly) && out.open(QIODevice::WriteOnly | QIODevice::Truncate))
        out.write(in.readAll());
    else
        return QString();

    return local;
}

VoiceManager *VoiceManager::s_instance = nullptr;

VoiceManager &VoiceManager::instance()
{
    if (!s_instance)
        s_instance = new VoiceManager;
    return *s_instance;
}

VoiceManager::VoiceManager(QObject *parent)
    : QObject(parent)
{
    if (!s_instance)
        s_instance = this;
}

void VoiceManager::setVolume(qreal v)
{
    v = qBound<qreal>(0.0, v, 1.0);
    if (qFuzzyCompare(m_volume, v))
        return;
    m_volume = v;
    emit volumeChanged();
}

void VoiceManager::setEnabled(bool on)
{
    m_enabled = on;
    emit enabledChanged();
}

QStringList VoiceManager::variants(const QString &charId, const QString &action)
{
    const QString key = charId + '/' + action;
    if (m_variantsCache.contains(key))
        return m_variantsCache.value(key);

    QStringList files;
    const QStringList roots = {
        ":/resources/voices/" + charId,
        ":/voices/" + charId,
        QCoreApplication::applicationDirPath() + "/voices/" + charId,
        QDir::currentPath() + "/resources/voices/" + charId,
    };

    QSet<QString> seen;
    for (const QString &root : roots) {
        QDir dir(root);
        if (!dir.exists())
            continue;

        for (const QString &entry : dir.entryList(QDir::Files | QDir::Readable, QDir::Name)) {
            if (!entry.startsWith(action, Qt::CaseInsensitive) || !(entry.endsWith(".wav") || entry.endsWith(".mp3")))
                continue;

            QString num = entry.mid(action.size());
            num = num.left(num.indexOf('.'));

            // 接受无数字后缀的基础文件(如 win.wav / win.mp3), 也接受带数字的多变体(如 win1.wav)
            bool ok = num.isEmpty();
            if (!ok) {
                num.toInt(&ok);
                if (!ok)
                    continue;
            }

            const QString path = root + '/' + entry;
            if (!seen.contains(path)) {
                seen.insert(path);
                files.append(path);
            }
        }
    }

    if (files.isEmpty()) {
        static QSet<QString> warned;
        if (!warned.contains(key)) {
            warned.insert(key);
            qWarning() << "[VoiceManager] no voices for" << key;
        }
    }

    files.sort();
    m_variantsCache.insert(key, files);
    return files;
}

QStringList VoiceManager::localVariants(const QString &charId, const QString &action)
{
    QStringList locals;
    for (const QString &p : variants(charId, action)) {
        const QString l = unpackToLocal(p);
        if (!l.isEmpty())
            locals.append(l);
    }
    return locals;
}

bool VoiceManager::hasVoice(const QString &c, const QString &a)
{
    return !variants(c, a).isEmpty();
}

QStringList VoiceManager::voiceFiles(const QString &c)
{
    QStringList r;
    for (const char *a : {"opening", "punch", "kick", "heavyPunch", "heavyKick", "strike", "special", "hurt", "jump", "dodge", "win"})
        for (const QString &p : variants(c, a))
            r << p;
    return r;
}

void VoiceManager::play(const QString &charId, const QString &action)
{
    if (!m_enabled || charId.isEmpty() || action.isEmpty())
        return;

    const QStringList files = localVariants(charId, action);
    if (files.isEmpty())
        return;

    QMediaPlayer *p = new QMediaPlayer;
    QAudioOutput *a = new QAudioOutput(p);
    a->setVolume(m_volume * actionVolume(action));
    p->setAudioOutput(a);

    connect(p, &QMediaPlayer::mediaStatusChanged, [p](QMediaPlayer::MediaStatus s) {
        if (s == QMediaPlayer::EndOfMedia)
            p->deleteLater();
    });

    p->setSource(QUrl::fromLocalFile(files.at(QRandomGenerator::global()->bounded(files.size()))));
    p->play();
}
