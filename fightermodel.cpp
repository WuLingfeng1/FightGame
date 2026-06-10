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
    if (m_frameTimer)
    {
        m_frameTimer->stop();
        m_frameTimer->deleteLater();
    }
}

QString FighterModel::charId() const
{
    return m_charId;
}

int FighterModel::currentFrame() const
{
    return m_currentFrame;
}

void FighterModel::setCharId(const QString &id)
{
    if (m_charId == id)
        return;

    m_charId = id;
    m_currentFrame = 0; // 切换角色重置帧

    m_frameTimer->stop();
    m_frameTimer->start(); // 启动动画

    qDebug() << "[FighterModel] charId =" << m_charId;

    emit charIdChanged();
    emit currentFrameChanged();
}

void FighterModel::onFrameTick()
{
    // C++ 统一做帧循环（核心业务逻辑）
    m_currentFrame = (m_currentFrame + 1) % m_totalFrames;
    emit currentFrameChanged();
}
