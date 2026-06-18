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
    m_heavyStrike  = cfg.heavyStrike;
    m_hurt         = cfg.hurt;
    m_hurt1        = cfg.hurt1;
    m_hurt2        = cfg.hurt2;
    m_hurt3        = cfg.hurt3;
    m_crouch       = cfg.crouch;
    m_crouchAttack = cfg.crouchAttack;
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
    if (m_crouching) return;
    m_timer.stop();
    m_state = Stand;
    m_currentFrame = 2;          // 从第2帧开始(避开站立的起始过渡帧)
    m_loopAnim = true;
    m_visualScale = 1.0;         // 必须在 applyAnim 之前设置, setPosY 依赖此值
    m_animOffsetX = 0;           // 站立动画无水平偏移
    m_hitThisAttack = false;     // 重置命中标志
    applyAnim(m_stand);
    m_timer.setInterval(m_stand.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
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
    if (m_crouching) return;
    if (m_state == LightPunch && m_timer.isActive()) return;
    m_timer.stop();
    m_state = LightPunch;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_lightPunch.visualScale;
    m_animOffsetX = m_lightPunch.offsetX;
    m_currentAnim = m_lightPunch;
    m_hitThisAttack = false;  // 重置命中标志
    applyAnim(m_lightPunch);
    m_timer.setInterval(m_lightPunch.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 切换到轻腿攻击动画
void CharacterModel::playLightKick()
{
    if (m_lightKick.cols <= 0) return;
    if (m_crouching) return;
    if (m_state == LightKick && m_timer.isActive()) return;
    m_timer.stop();
    m_state = LightKick;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_lightKick.visualScale;
    m_animOffsetX = m_lightKick.offsetX;
    m_currentAnim = m_lightKick;
    m_hitThisAttack = false;
    applyAnim(m_lightKick);
    m_timer.setInterval(m_lightKick.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 切换到重拳攻击动画
void CharacterModel::playHeavyPunch()
{
    if (m_heavyPunch.cols <= 0) return;
    if (m_crouching) return;
    if (m_state == HeavyPunch && m_timer.isActive()) return;
    m_timer.stop();
    m_state = HeavyPunch;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_heavyPunch.visualScale;
    m_animOffsetX = m_heavyPunch.offsetX;
    m_currentAnim = m_heavyPunch;
    m_hitThisAttack = false;
    applyAnim(m_heavyPunch);
    m_timer.setInterval(m_heavyPunch.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 切换到重腿攻击动画
void CharacterModel::playHeavyKick()
{
    if (m_heavyKick.cols <= 0) return;
    if (m_crouching) return;
    if (m_state == HeavyKick && m_timer.isActive()) return;
    m_timer.stop();
    m_state = HeavyKick;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_heavyKick.visualScale;
    m_animOffsetX = m_heavyKick.offsetX;
    m_currentAnim = m_heavyKick;
    m_hitThisAttack = false;
    applyAnim(m_heavyKick);
    m_timer.setInterval(m_heavyKick.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 切换到超重击动画
void CharacterModel::playHeavyStrike()
{
    if (m_heavyStrike.cols <= 0) return;
    if (m_crouching) return;
    m_timer.stop();
    m_state = HeavyStrike;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_heavyStrike.visualScale;
    // 从配置读取 offsetX，实现视觉上的位移
    // QML 中会根据 facingLeft 自动翻转方向
    m_animOffsetX = m_heavyStrike.offsetX;
    m_currentAnim = m_heavyStrike;
    m_hitThisAttack = false;
    applyAnim(m_heavyStrike);
    m_timer.setInterval(m_heavyStrike.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 切换到受击动画
void CharacterModel::playHurt()
{
    if (m_hurt.cols <= 0) return;
    if (m_crouching) return;
    m_timer.stop();
    m_state = Hurt;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_hurt.visualScale;
    m_animOffsetX = 0;
    m_currentAnim = m_hurt;  // 设置当前动画参数用于位置计算
    applyAnim(m_hurt);
    applyKnockback();  // 被击中时应用击退
    m_timer.setInterval(m_hurt.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 切换到轻度受击动画
void CharacterModel::playHurt1()
{
    if (m_hurt1.cols <= 0) { playHurt(); return; }
    if (m_crouching) return;
    m_timer.stop();
    m_state = Hurt;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_hurt1.visualScale;
    m_animOffsetX = 0;
    m_currentAnim = m_hurt1;  // 设置当前动画参数用于位置计算
    applyAnim(m_hurt1);
    applyKnockback();  // 被击中时应用击退
    m_timer.setInterval(m_hurt1.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 切换到中度受击动画
void CharacterModel::playHurt2()
{
    if (m_hurt2.cols <= 0) { playHurt(); return; }
    if (m_crouching) return;
    m_timer.stop();
    m_state = Hurt;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_hurt2.visualScale;
    m_animOffsetX = 0;
    m_currentAnim = m_hurt2;  // 设置当前动画参数用于位置计算
    applyAnim(m_hurt2);
    applyKnockback();  // 被击中时应用击退
    m_timer.setInterval(m_hurt2.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 切换到重度受击动画
void CharacterModel::playHurt3()
{
    if (m_hurt3.cols <= 0) { playHurt(); return; }
    if (m_crouching) return;
    m_timer.stop();
    m_state = Hurt;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_hurt3.visualScale;
    m_animOffsetX = 0;
    m_currentAnim = m_hurt3;  // 设置当前动画参数用于位置计算
    applyAnim(m_hurt3);
    applyKnockback();  // 被击中时应用击退
    m_timer.setInterval(m_hurt3.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 切换到下蹲动画: Orochi冻结在第0帧(蹲姿), Yagami循环播放
void CharacterModel::playCrouch()
{
    if (m_crouch.cols <= 0) return;
    if (m_state == Crouch || m_state == CrouchAttack) return;
    m_timer.stop();
    m_state = Crouch;
    m_currentFrame = 0;
    m_crouching = true;
    m_loopAnim = m_crouch.loop;
    m_visualScale = m_crouch.visualScale;
    m_animOffsetX = m_crouch.offsetX;
    m_currentAnim = m_crouch;
    applyAnim(m_crouch);
    if (m_crouch.loop) {
        m_timer.setInterval(m_crouch.interval);
        m_timer.start();
    }
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 切换到下蹲攻击动画: 从第0帧开始播放完整动画, 播完回到蹲姿
void CharacterModel::playCrouchAttack()
{
    if (m_crouchAttack.cols <= 0) return;
    if (m_state == CrouchAttack && m_timer.isActive()) return;
    m_timer.stop();
    m_state = CrouchAttack;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_crouchAttack.visualScale;
    m_animOffsetX = m_crouchAttack.offsetX;
    m_currentAnim = m_crouchAttack;
    m_hitThisAttack = false;
    applyAnim(m_crouchAttack);
    m_timer.setInterval(m_crouchAttack.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 退出下蹲状态: 回到站立
void CharacterModel::stopCrouch()
{
    m_crouching = false;
    if (m_state == Crouch || m_state == CrouchAttack) {
        playStand();
    }
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
    m_crouching = false;
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
            case HeavyStrike:
                fb = m_heavyStrike.feetBottom;  fm = m_heavyStrike.feetMargin;  break;
            case Forward:
                fb = m_forward.feetBottom;    fm = m_forward.feetMargin;    break;
            case Backward:
                fb = m_backward.feetBottom;   fm = m_backward.feetMargin;   break;
            case Hurt:
                fb = m_currentAnim.feetBottom; fm = m_currentAnim.feetMargin; break;
            case Crouch:
                fb = m_crouch.feetBottom;      fm = m_crouch.feetMargin;      break;
            case CrouchAttack:
                fb = m_crouchAttack.feetBottom; fm = m_crouchAttack.feetMargin; break;
            default:
                fb = m_stand.feetBottom;      fm = m_stand.feetMargin;      break;
        }
        if (fb <= 0 || fm <= 0) { fb = m_stand.feetBottom; fm = m_stand.feetMargin; }
        if (fb > 0 && fm > 0) {
            // HeavyStrike 特殊处理：确保底部位置与 stand 一致
            if (m_state == HeavyStrike) {
                double standBottom = m_rootHeight - m_stand.feetMargin - m_stand.feetBottom + m_stand.fh;
                m_posY = standBottom - m_frameHeight;
            } else {
                m_posY = m_rootHeight - fm - fb;
                // 补偿 visualScale 从 Item 底部缩放导致的脚尖上移
                if (m_visualScale > 1.0)
                    m_posY += (m_frameHeight - fb) * (m_visualScale - 1.0);
                else if (m_visualScale < 1.0)
                    m_posY -= (m_frameHeight - fb) * (1.0 - m_visualScale);
            }
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
    if (m_state == HeavyStrike && m_currentFrame >= m_totalFrames) {
        m_currentFrame = m_totalFrames - 1;
        m_timer.stop();
        playStand();
        emit attackFinished();
        return;
    }
    if (m_state == Hurt && m_currentFrame >= m_totalFrames) {
        m_currentFrame = m_totalFrames - 1;
        m_timer.stop();
        playStand();
        emit hurtFinished();
        return;
    }
    if (m_state == Crouch && m_currentFrame >= m_totalFrames) {
        if (m_loopAnim) {
            m_currentFrame = 0;
        } else {
            m_currentFrame = m_totalFrames - 1;
            m_timer.stop();
            return;
        }
    }
    if (m_state == CrouchAttack && m_currentFrame >= m_totalFrames) {
        m_currentFrame = m_totalFrames - 1;
        m_timer.stop();
        // 回到蹲姿冻结在第0帧
        if (m_crouching && m_crouch.cols > 0) {
            m_state = Crouch;
            m_currentFrame = 0;
            m_loopAnim = m_crouch.loop;
            m_visualScale = m_crouch.visualScale;
            m_animOffsetX = m_crouch.offsetX;
            m_currentAnim = m_crouch;
            applyAnim(m_crouch);
            emit sourcePathChanged();
            emit sizeChanged();
            emit stateChanged();
        } else {
            playStand();
        }
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
    // 渐进式击退: 每帧应用一小部分击退距离
    if (m_state == Hurt && m_knockbackFrames > 0 && m_knockbackRemaining > 0 && m_opponent) {
        double step = m_knockbackRemaining / m_knockbackFrames;
        double oppX = m_opponent->posXRatio();
        
        if (m_cfgPosX > oppX) {
            m_cfgPosX += step;  // 向右退
        } else {
            m_cfgPosX -= step;  // 向左退
        }
        
        // 尊重舞台边界
        m_cfgPosX = std::max(0.0, std::min(m_cfgPosX, 3.0));
        m_knockbackRemaining -= step;
        m_knockbackFrames--;
        
        emit posXRatioChanged();
    }
    // HeavyStrike 动画播放期间触发镜头更新
    if (m_state == HeavyStrike) {
        emit posXRatioChanged();
    }
    emit frameChanged();
}

// 碰撞检测相关方法实现

bool CharacterModel::isAttacking() const
{
    return m_state == LightPunch || m_state == LightKick ||
           m_state == HeavyPunch || m_state == HeavyKick ||
           m_state == HeavyStrike || m_state == CrouchAttack;
}

bool CharacterModel::isAttackActive() const
{
    if (!isAttacking()) return false;
    if (m_hitThisAttack) return false;  // 本次攻击已命中，不再判定
    if (m_currentAnim.attackFrames.isEmpty()) return true;  // 未配置则整个动画都生效

    // 解析 attackFrames 格式: "3-5" 或 "3,5,7" 或 "3-5,8-10"
    QStringList parts = m_currentAnim.attackFrames.split(',');
    for (const QString &part : parts) {
        if (part.contains('-')) {
            QStringList range = part.split('-');
            int start = range[0].toInt();
            int end = range[1].toInt();
            if (m_currentFrame >= start && m_currentFrame <= end) return true;
        } else {
            if (m_currentFrame == part.toInt()) return true;
        }
    }
    return false;
}

double CharacterModel::hitboxX() const
{
    if (!isAttacking()) return 0;
    // 攻击点X = 角色中心X + 攻击点偏移(归一化) * 朝向
    double offsetX = m_currentAnim.attackPointX;  // 已经是归一化坐标
    return m_cfgPosX + (m_facingLeft ? -offsetX : offsetX);
}

double CharacterModel::hitboxY() const
{
    if (!isAttacking()) return 0;
    // 攻击点Y = 角色Y + 帧高度/2 + 攻击点Y偏移
    return m_posY + m_frameHeight / 2.0 + m_currentAnim.attackPointY;
}

int CharacterModel::hitboxRadius() const
{
    if (!isAttacking()) return 0;
    return m_currentAnim.attackRadius;
}

double CharacterModel::hurtboxX() const
{
    return m_cfgPosX;
}

double CharacterModel::hurtboxY() const
{
    return m_posY + m_frameHeight / 2.0;
}

// 被击中时应用击退效果(渐进式, 分散到受击动画的每一帧)
void CharacterModel::applyKnockback()
{
    if (!m_opponent || m_knockbackToApply <= 0) return;
    
    m_knockbackRemaining = m_knockbackToApply / 1000.0;
    m_knockbackFrames = m_currentAnim.cols;  // 分散到整个受击动画
    m_knockbackToApply = 0;
}
