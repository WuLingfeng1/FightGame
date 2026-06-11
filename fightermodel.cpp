#include "fightermodel.h"
#include <QDebug>

FighterModel::FighterModel(QObject *parent)
    : QObject(parent)
{
    m_frameTimer = new QTimer(this);
    m_frameTimer->setInterval(m_frameInterval);
    m_frameTimer->setTimerType(Qt::PreciseTimer);
    connect(m_frameTimer, &QTimer::timeout, this, &FighterModel::onFrameTick);

    qDebug() << "[FighterModel] created —"
             << m_totalFrames << "frames,"
             << m_frameInterval << "ms each";
}

FighterModel::~FighterModel()
{
    m_frameTimer->stop();
}

QString FighterModel::charId() const { return m_charId; }
int FighterModel::currentFrame() const { return m_currentFrame; }
int FighterModel::totalFrames() const { return m_totalFrames; }

void FighterModel::setTotalFrames(int n)
{
    if (n < 1 || n == m_totalFrames) return;
    m_totalFrames = n;
    m_currentFrame = 0;
    emit totalFramesChanged();
}

void FighterModel::setCharId(const QString &id)
{
    if (m_charId == id) return;
    m_charId = id;
    m_currentFrame = 0;
    m_frameTimer->stop();
    m_frameTimer->start();
    qDebug() << "[FighterModel] charId =" << m_charId << "totalFrames:" << m_totalFrames;
    emit charIdChanged();
    emit currentFrameChanged();
}

void FighterModel::onFrameTick()
{
    m_currentFrame = (m_currentFrame + 1) % m_totalFrames;
    emit currentFrameChanged();
}
