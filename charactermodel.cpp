#include "charactermodel.h"
#include <algorithm>

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
    m_jump         = cfg.jump;
    m_diagonalJump = cfg.diagonalJump;
    m_lightPunch   = cfg.lightPunch;
    m_lightKick    = cfg.lightKick;
    m_heavyPunch   = cfg.heavyPunch;
    m_heavyKick    = cfg.heavyKick;
    m_jumpHeight   = cfg.jump.jumpHeight;
    m_refFrameWidth = cfg.stand.fw;
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
    m_loopAnim = true;
    m_visualScale = 1.0;         // 必须在 applyAnim 之前设置, setPosY 依赖此值
    m_animOffsetX = 0;           // 站立动画无水平偏移
    applyAnim(m_stand);
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
    if (m_state == Forward && m_timer.isActive()) return;  // 已在行走中则跳过
    m_timer.stop();
    m_state = Forward;
    m_currentFrame = 0;
    m_loopAnim = m_forward.loop;
    m_visualScale = 1.0;         // 必须在 applyAnim 之前设置
    m_animOffsetX = 0;
    applyAnim(m_forward);
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
    if (m_state == Backward && m_timer.isActive()) return;  // 已在后退中则跳过
    m_timer.stop();
    m_state = Backward;
    m_currentFrame = 0;
    m_loopAnim = m_backward.loop;
    m_visualScale = 1.0;         // 必须在 applyAnim 之前设置
    m_animOffsetX = 0;
    applyAnim(m_backward);
    m_timer.setInterval(m_backward.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
}

// 切换到直跳动画: 从第0帧开始, 不循环, 播完自动切 Stand
void CharacterModel::playJump()
{
    if (m_jump.cols <= 0) return;
    m_timer.stop();
    m_state = Jump;
    m_currentFrame = 0;
    m_loopAnim = m_jump.loop;
    m_jumpHeight = m_jump.jumpHeight;
    m_visualScale = m_jump.visualScale;
    applyAnim(m_jump);
    m_timer.setInterval(m_jump.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
}

// 切换到对角跳动画: forward=true前跳(朝对手) false后跳(背向对手)
// 帧段由 divFrame 分割: 左半(0~divFrame-1)=右向跳跃 右半(divFrame~cols-1)=左向跳跃
// 朝向翻转由 QML Scale 处理, 因此前跳始终用左半段 后跳始终用右半段
void CharacterModel::playDiagonalJump(bool forward)
{
    if (m_diagonalJump.cols <= 0 || m_diagonalJump.divFrame <= 0) return;
    m_timer.stop();
    m_state = DiagonalJump;
    m_djIsForward = forward;
    m_djStartXRatio = m_cfgPosX;
    m_djInitialFacingLeft = m_facingLeft;
    if (forward) {
        m_djStartFrame = 0;
        m_djTotalFrames = m_diagonalJump.divFrame;
    } else {
        m_djStartFrame = m_diagonalJump.divFrame;
        m_djTotalFrames = m_diagonalJump.cols - m_diagonalJump.divFrame;
    }
    m_currentFrame = m_djStartFrame;
    m_loopAnim = false;
    m_jumpHeight = m_diagonalJump.jumpHeight;
    m_djDistance = m_diagonalJump.jumpDistance;
    applyAnim(m_diagonalJump);
    if (forward) {
        m_djOffsetFirst = m_diagonalJump.offsetXFwd;
        m_djOffsetLast = m_diagonalJump.offsetXLast;
    } else {
        m_djOffsetFirst = m_diagonalJump.offsetXBwd;
        m_djOffsetLast = m_diagonalJump.offsetXBwdLast;
    }
    m_animOffsetX = m_djOffsetFirst;
    m_visualScale = m_diagonalJump.visualScale;
    m_timer.setInterval(m_diagonalJump.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
}

// 切换到轻拳攻击动画: 从第0帧开始, 不循环, 播完自动切 Stand
void CharacterModel::playLightPunch()
{
    if (m_lightPunch.cols <= 0) return;
    if (m_state == LightPunch && m_timer.isActive()) return;
    m_timer.stop();
    m_state = LightPunch;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_lightPunch.visualScale;
    m_animOffsetX = m_lightPunch.offsetX;  // 轻拳水平偏移补偿
    applyAnim(m_lightPunch);
    m_timer.setInterval(m_lightPunch.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
}

// 切换到轻腿攻击动画
void CharacterModel::playLightKick()
{
    if (m_lightKick.cols <= 0) return;
    if (m_state == LightKick && m_timer.isActive()) return;
    m_timer.stop();
    m_state = LightKick;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_lightKick.visualScale;
    m_animOffsetX = m_lightKick.offsetX;
    applyAnim(m_lightKick);
    m_timer.setInterval(m_lightKick.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
}

// 切换到重拳攻击动画
void CharacterModel::playHeavyPunch()
{
    if (m_heavyPunch.cols <= 0) return;
    if (m_state == HeavyPunch && m_timer.isActive()) return;
    m_timer.stop();
    m_state = HeavyPunch;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_heavyPunch.visualScale;
    m_animOffsetX = m_heavyPunch.offsetX;
    applyAnim(m_heavyPunch);
    m_timer.setInterval(m_heavyPunch.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
}

// 切换到重腿攻击动画
void CharacterModel::playHeavyKick()
{
    if (m_heavyKick.cols <= 0) return;
    if (m_state == HeavyKick && m_timer.isActive()) return;
    m_timer.stop();
    m_state = HeavyKick;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_heavyKick.visualScale;
    m_animOffsetX = m_heavyKick.offsetX;
    applyAnim(m_heavyKick);
    m_timer.setInterval(m_heavyKick.interval);
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
    if (m_state == Jump || m_state == DiagonalJump) {
        int totalFrames = (m_state == DiagonalJump) ? m_djTotalFrames : m_totalFrames;
        int localFrame  = (m_state == DiagonalJump) ? (m_currentFrame - m_djStartFrame) : m_currentFrame;
        double t = (totalFrames > 1) ? (double)localFrame / (totalFrames - 1) : 0;
        double arcOffset = -m_jumpHeight * 4.0 * t * (1.0 - t);
        double groundY;
        if (m_stand.feetMargin > 0 && m_stand.feetBottom > 0) {
            groundY = m_rootHeight - m_stand.feetMargin - m_stand.feetBottom;
        } else {
            const AnimParams &curAnim = (m_state == DiagonalJump) ? m_diagonalJump : m_jump;
            if (curAnim.feetMargin > 0 && curAnim.feetBottom > 0) {
                groundY = m_rootHeight - curAnim.feetMargin - curAnim.feetBottom;
            } else {
                groundY = m_rootHeight - m_frameHeight - 60.0;
            }
        }
        m_posY = groundY + arcOffset;
    } else if (m_state == Opening || m_state == Waiting) {
        m_posY = m_rootHeight - m_frameHeight - 60.0;
    } else {
        // 优先使用当前动画自身的 feetBottom/feetMargin，回退到 stand 的值
        double fb = 0, fm = 0;
        switch (m_state) {
            case LightPunch:
                fb = m_lightPunch.feetBottom; fm = m_lightPunch.feetMargin; break;
            case LightKick:
                fb = m_lightKick.feetBottom;  fm = m_lightKick.feetMargin;  break;
            case HeavyPunch:
                fb = m_heavyPunch.feetBottom; fm = m_heavyPunch.feetMargin; break;
            case HeavyKick:
                fb = m_heavyKick.feetBottom;  fm = m_heavyKick.feetMargin;  break;
            case Forward:
                fb = m_forward.feetBottom;    fm = m_forward.feetMargin;    break;
            case Backward:
                fb = m_backward.feetBottom;   fm = m_backward.feetMargin;   break;
            default:
                fb = m_stand.feetBottom;      fm = m_stand.feetMargin;      break;
        }
        if (fb <= 0 || fm <= 0) { fb = m_stand.feetBottom; fm = m_stand.feetMargin; }
        if (fb > 0 && fm > 0) {
            m_posY = m_rootHeight - fm - fb;
            // 补偿 visualScale 从 Item 底部缩放导致的脚尖上移
            if (m_visualScale > 1.0)
                m_posY += (m_frameHeight - fb) * (m_visualScale - 1.0);
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
    if (m_state == Jump && m_currentFrame >= m_totalFrames) {
        m_currentFrame = m_totalFrames - 1;
        m_timer.stop();
        emit jumpFinished();
        return;
    }
    if (m_state == DiagonalJump && m_currentFrame >= m_djStartFrame + m_djTotalFrames) {
        m_currentFrame = m_djStartFrame + m_djTotalFrames - 1;
        m_timer.stop();
        emit jumpFinished();
        return;
    }
    if (m_state == LightPunch && m_currentFrame >= m_totalFrames) {
        m_currentFrame = m_totalFrames - 1;
        m_timer.stop();
        playStand();
        emit attackFinished();
        return;
    }
    if (m_state == LightKick && m_currentFrame >= m_totalFrames) {
        m_currentFrame = m_totalFrames - 1;
        m_timer.stop();
        playStand();
        emit attackFinished();
        return;
    }
    if (m_state == HeavyPunch && m_currentFrame >= m_totalFrames) {
        m_currentFrame = m_totalFrames - 1;
        m_timer.stop();
        playStand();
        emit attackFinished();
        return;
    }
    if (m_state == HeavyKick && m_currentFrame >= m_totalFrames) {
        m_currentFrame = m_totalFrames - 1;
        m_timer.stop();
        playStand();
        emit attackFinished();
        return;
    }
    if (m_state == Stand && m_currentFrame >= m_totalFrames) {
        m_currentFrame = 0;                   // 站立动画循环
    }
    if (m_state == Jump || m_state == DiagonalJump) {
        if (m_state == DiagonalJump) {
            int localFrame = m_currentFrame - m_djStartFrame;
            double t = (m_djTotalFrames > 1) ? (double)localFrame / (m_djTotalFrames - 1) : 0;
            double xSign = m_djInitialFacingLeft ? -1.0 : 1.0;
            double xDelta = xSign * (m_djIsForward ? 1.0 : -1.0);
            double newX = m_djStartXRatio + xDelta * m_djDistance * t;
            if (m_opponent) {
                double oppX = m_opponent->posXRatio();
                double minX = std::max(0.0, oppX - 0.95);
                double maxX = std::min(3.0, oppX + 0.95);
                newX = std::max(minX, std::min(newX, maxX));
            }
            if (m_opponent)
                m_facingLeft = (newX > m_opponent->posXRatio());
            m_cfgPosX = newX;
            m_animOffsetX = m_djOffsetFirst + (int)((m_djOffsetLast - m_djOffsetFirst) * t);
            emit posXRatioChanged();
        }
        if (m_state == Jump && m_opponent)
            m_facingLeft = (m_cfgPosX > m_opponent->posXRatio());
        setPosY();
    }
    emit frameChanged();
}
