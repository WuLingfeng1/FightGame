#include "charactermodel.h"

CharacterModel::CharacterModel(QObject *parent)
    : QObject(parent)
{
    m_timer.setSingleShot(false);
    connect(&m_timer, &QTimer::timeout, this, &CharacterModel::onTick);
}

CharacterModel::~CharacterModel()
{
    m_timer.stop();
}

void CharacterModel::configure(const CharacterConfig &cfg)
{
    m_opening      = cfg.opening;
    m_stand        = cfg.stand;
    m_forward      = cfg.forward;
    m_facingLeft   = cfg.facingLeft;
    m_cfgPosX      = cfg.posX;
    m_state        = Waiting;
    m_currentFrame = 0;
}

void CharacterModel::playOpening()
{
    if (m_state != Waiting) return;
    m_state = Opening;
    m_currentFrame = 0;
    applyAnim(m_opening);
    m_timer.setInterval(m_opening.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
}

void CharacterModel::playStand()
{
    m_timer.stop();
    m_state = Stand;
    m_currentFrame = 2;          // start from frame 2 (matching reference)
    applyAnim(m_stand);
    m_loopAnim = true;
    m_timer.setInterval(m_stand.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
}

void CharacterModel::playForward()
{
    if (m_forward.cols <= 0) return;
    m_timer.stop();
    m_state = Forward;
    m_currentFrame = 0;
    applyAnim(m_forward);
    m_loopAnim = m_forward.loop;
    m_timer.setInterval(m_forward.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
}

void CharacterModel::reset()
{
    m_timer.stop();
    m_state = Waiting;
    m_currentFrame = 0;
    m_frameWidth = 0;
    m_frameHeight = 0;
    m_totalFrames = 0;
    m_sourcePath.clear();
}

void CharacterModel::updateRootHeight(double h)
{
    m_rootHeight = h;
    setPosY();
}

void CharacterModel::applyAnim(const AnimParams &p)
{
    m_sourcePath   = p.path;
    m_frameWidth   = p.fw;
    m_frameHeight  = p.fh;
    m_totalFrames  = p.cols;
    setPosY();
}

void CharacterModel::setPosY()
{
    if (m_state == Opening || m_state == Waiting) {
        // opening: bottom-aligned with 60px margin
        m_posY = m_rootHeight - m_frameHeight - 60.0;
    } else {
        // stand: feet-aligned
        if (m_stand.feetMargin > 0 && m_stand.feetBottom > 0) {
            m_posY = m_rootHeight - m_stand.feetMargin - m_stand.feetBottom;
        } else {
            m_posY = m_rootHeight - m_frameHeight - 60.0;
        }
    }
    emit positionChanged();
}

void CharacterModel::onTick()
{
    m_currentFrame++;
    if (m_state == Opening && m_currentFrame >= m_totalFrames) {
        m_currentFrame = m_totalFrames - 1;
        playStand();
        emit openingFinished();
        return;
    }
    if (m_state == Forward && m_currentFrame >= m_totalFrames) {
        if (m_loopAnim) {
            m_currentFrame = 0;
        } else {
            m_currentFrame = m_totalFrames - 1;
            playStand();
            return;
        }
    }
    if (m_state == Stand && m_currentFrame >= m_totalFrames) {
        m_currentFrame = 0;
    }
    emit frameChanged();
}
