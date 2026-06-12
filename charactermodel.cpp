#include "charactermodel.h"

// 构造函数: 创建重复触发的定时器, 绑定 onTick 回调
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

// 从配置初始化角色模型: 保存动画参数副本, 重置状态为 Waiting
void CharacterModel::configure(const CharacterConfig &cfg)
{
    m_opening      = cfg.opening;
    m_stand        = cfg.stand;
    m_forward      = cfg.forward;
    m_backward     = cfg.backward;
    setFacingLeft(cfg.facingLeft);
    setPosXRatio(cfg.posX);
    m_state        = Waiting;
    m_currentFrame = 0;
}

// 播放开场动画: 仅允许从 Waiting 状态进入, 播放完毕后自动切 Stand
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

// 切换到站立动画: 停止当前动画, 从第2帧开始播放(匹配参考实现), 循环模式
void CharacterModel::playStand()
{
    m_timer.stop();
    m_state = Stand;
    m_currentFrame = 2;          // 从第2帧开始(避开站立的起始过渡帧)
    applyAnim(m_stand);
    m_loopAnim = true;
    m_timer.setInterval(m_stand.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
}

// 切换到前进行走动画: 从第0帧开始, 循环与否由配置决定
void CharacterModel::playForward()
{
    if (m_forward.cols <= 0) return;   // 未配置行走动画则忽略
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

// 切换到后退行走动画: 从第0帧开始, 循环与否由配置决定
void CharacterModel::playBackward()
{
    if (m_backward.cols <= 0) return;   // 未配置后退动画则忽略
    m_timer.stop();
    m_state = Backward;
    m_currentFrame = 0;
    applyAnim(m_backward);
    m_loopAnim = m_backward.loop;
    m_timer.setInterval(m_backward.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
}

// 重置角色到初始状态
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

// 更新窗口高度并重新计算Y坐标
void CharacterModel::updateRootHeight(double h)
{
    m_rootHeight = h;
    setPosY();
}

// 将 AnimParams 中的元数据应用到当前角色的运行时属性
void CharacterModel::applyAnim(const AnimParams &p)
{
    m_sourcePath   = p.path;
    m_frameWidth   = p.fw;
    m_frameHeight  = p.fh;
    m_totalFrames  = p.cols;
    setPosY();
}

// 计算角色的像素Y坐标, 支持两种对齐模式:
// 1. 开场/Waiting: 底部对齐, 下方留60px边距
// 2. 站立/行走: 脚部对齐(如果配置了feetMargin/feetBottom), 否则底部对齐
void CharacterModel::setPosY()
{
    if (m_state == Opening || m_state == Waiting) {
        m_posY = m_rootHeight - m_frameHeight - 60.0;
    } else {
        if (m_stand.feetMargin > 0 && m_stand.feetBottom > 0) {
            m_posY = m_rootHeight - m_stand.feetMargin - m_stand.feetBottom;
        } else {
            m_posY = m_rootHeight - m_frameHeight - 60.0;
        }
    }
    emit positionChanged();
}

// 动画帧推进: 定时器回调, 每 interval 毫秒触发一次
// Opening状态: 播放完毕后冻结最后一帧, 自动切 Stand
// Forward/Backward状态: 循环则回到第0帧, 否则冻结并切 Stand
// Stand状态:  循环回到第0帧
void CharacterModel::onTick()
{
    if (m_state == Opening && m_opening.pauseFrame > 0) {
        if (m_currentFrame == m_opening.pauseFrame - 1) {
            m_currentFrame++;
            emit frameChanged();
            m_timer.setInterval(m_opening.pauseDuration);
            return;
        }
        if (m_currentFrame == m_opening.pauseFrame) {
            m_timer.setInterval(m_opening.interval);
        }
    }

    m_currentFrame++;
    if (m_state == Opening && m_currentFrame >= m_totalFrames) {
        m_currentFrame = m_totalFrames - 1;   // 冻结在最后一帧
        playStand();
        emit openingFinished();
        return;
    }
    if (m_state == Forward && m_currentFrame >= m_totalFrames) {
        if (m_loopAnim) {
            m_currentFrame = 0;               // 循环: 回到首帧
        } else {
            m_currentFrame = m_totalFrames - 1; // 不循环: 冻结末帧并切回站立
            playStand();
            return;
        }
    }
    if (m_state == Backward && m_currentFrame >= m_totalFrames) {
        if (m_loopAnim) {
            m_currentFrame = 0;               // 循环: 回到首帧
        } else {
            m_currentFrame = m_totalFrames - 1; // 不循环: 冻结末帧并切回站立
            playStand();
            return;
        }
    }
    if (m_state == Stand && m_currentFrame >= m_totalFrames) {
        m_currentFrame = 0;                   // 站立动画循环
    }
    emit frameChanged();
}
