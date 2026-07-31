#include "charactermodel.h"
#include <algorithm>

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

void CharacterModel::configure(const CharacterData &cfg)
{
    m_opening = cfg.opening;
    m_stand = cfg.stand;
    m_forward = cfg.forward;
    m_backward = cfg.backward;
    m_jump = cfg.jump;
    m_diagonalJump = cfg.diagonalJump;
    m_lightPunch = cfg.lightPunch;
    m_lightKick = cfg.lightKick;
    m_heavyPunch = cfg.heavyPunch;
    m_heavyKick = cfg.heavyKick;
    m_heavyStrike = cfg.heavyStrike;
    m_hurt = cfg.hurt;
    m_hurt1 = cfg.hurt1;
    m_hurt2 = cfg.hurt2;
    m_hurt3 = cfg.hurt3;
    m_crouch = cfg.crouch;
    m_crouchAttack = cfg.crouchAttack;
    m_dodge = cfg.dodge;
    m_standBlock = cfg.standBlock;
    m_blockHoldFrame = cfg.standBlock.blockHoldFrame;
    m_crouchHoldFrame = cfg.crouch.crouchHoldFrame;
    m_jumpHeight = cfg.jump.jumpHeight;
    m_refFrameWidth = cfg.stand.fw;
    setFacingLeft(cfg.facingLeft);
    setPosXRatio(cfg.posX);
    m_state = Waiting;
    m_currentFrame = 0;
    m_crouching = false; // 重置下蹲状态
}

// 播放开场: 仅Waiting状态可进入, 播完自动切Stand
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

// 站立: 从第2帧开始(避开起始过渡帧), 循环播放
void CharacterModel::playStand()
{
    if (m_crouching) return;
    m_timer.stop();
    m_state = Stand;
    m_currentFrame = 0; // 从第2帧开始(避开站立的起始过渡帧)
    m_loopAnim = true;
    m_visualScale = m_stand.visualScale;     // 必须在 applyAnim 之前设置, setPosY 依赖此值
    m_animOffsetX = 0;        // 站立动画无水平偏移
    m_hitThisAttack = false;  // 重置命中标志
    applyAnim(m_stand);
    m_timer.setInterval(m_stand.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 前进: 循环与否由配置决定
void CharacterModel::playForward()
{
    if (m_forward.cols <= 0) return;                      // 未配置行走动画则忽略
    if (m_state == Forward && m_timer.isActive()) return; // 已在行走中则跳过
    m_timer.stop();
    m_state = Forward;
    m_currentFrame = 0;
    m_loopAnim = m_forward.loop;
    m_visualScale = m_forward.visualScale; // 必须在 applyAnim 之前设置
    m_animOffsetX = 0;
    applyAnim(m_forward);
    m_timer.setInterval(m_forward.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
}

// 后退: 循环与否由配置决定
void CharacterModel::playBackward()
{
    if (m_backward.cols <= 0) return;                      // 未配置后退动画则忽略
    if (m_state == Backward && m_timer.isActive()) return; // 已在后退中则跳过
    m_timer.stop();
    m_state = Backward;
    m_currentFrame = 0;
    m_loopAnim = m_backward.loop;
    m_visualScale = m_backward.visualScale; // 必须在 applyAnim 之前设置
    m_animOffsetX = 0;
    applyAnim(m_backward);
    m_timer.setInterval(m_backward.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
}

// 直跳: 不循环, 播完触发jumpFinished
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

// 对角跳: forward=true前跳(朝对手) false后跳(背向对手)
// 帧段由 divFrame 分割: 左半=右向跳跃 右半=左向跳跃
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

// 轻拳
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
    m_hitThisAttack = false; // 重置命中标志
    applyAnim(m_lightPunch);
    m_timer.setInterval(m_lightPunch.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 轻腿
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

// 重拳
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

// 重腿
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
    if (m_heavyKick.heavyKickJumpHeight > 0) {
        m_jumpHeight = m_heavyKick.heavyKickJumpHeight;
        m_attackJumping = true;
    }
    applyAnim(m_heavyKick);
    m_timer.setInterval(m_heavyKick.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 超重击
void CharacterModel::playHeavyStrike()
{
    if (m_heavyStrike.cols <= 0) return;
    if (m_crouching) return;
    m_timer.stop();
    m_state = HeavyStrike;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_heavyStrike.visualScale;
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

// 闪避
void CharacterModel::playDodge(bool forward)
{
    if (m_dodge.cols <= 0) return;
    m_timer.stop();
    m_state = Dodge;
    m_loopAnim = false;
    m_visualScale = m_dodge.visualScale;
    if (m_dodge.dodgeStartFrame > 0)
        m_animOffsetX = m_dodge.offsetX;
    else if (forward || m_dodge.dodgeBackFrames.isEmpty())
        m_animOffsetX = m_dodge.offsetX;
    else
        m_animOffsetX = -m_dodge.offsetX;
    m_currentAnim = m_dodge;
    m_dodgeForward = forward;
    m_dodgeStandPending = false;
    m_totalFrames = m_dodge.cols;

    // 帧范围：Yagami 用 dodgeFrames/dodgeBackFrames，Orochi 用 dodgeStartFrame/dodgeEndFrame
    m_dodgeEndFrame = 0;
    if (m_dodge.dodgeFrames.length() > 0) {
        QStringList fwdRange = m_dodge.dodgeFrames.split('-');
        QStringList bwdRange = m_dodge.dodgeBackFrames.split('-');
        if (forward && fwdRange.size() == 2) {
            m_currentFrame = fwdRange[0].toInt();
            m_dodgeEndFrame = fwdRange[1].toInt() + 1;
        } else if (!forward && bwdRange.size() == 2) {
            m_currentFrame = bwdRange[0].toInt();
            m_dodgeEndFrame = bwdRange[1].toInt() + 1;
        } else {
            m_currentFrame = 0;
        }
    } else {
        // Orochi：用 dodgeStartFrame/dodgeEndFrame
        m_currentFrame = 0;
        if (m_dodge.dodgeEndFrame > 0 && m_dodge.dodgeEndFrame < m_dodge.cols)
            m_dodgeEndFrame = m_dodge.dodgeEndFrame + 1;
        else
            m_dodgeEndFrame = m_dodge.cols;
    }

    m_sourcePath = m_dodge.path;
    m_frameWidth = m_dodge.fw;
    m_frameHeight = m_dodge.fh;
    if (m_dodge.dodgeStartFrame > 0) {
        m_dodgeOffsetXPost = m_dodge.offsetXPost;
        m_dodgeOffsetXEnd = m_dodge.offsetXEnd;
        if (m_dodge.dodgeSwitchFrame > 0)
            m_dodgeSwitchFrame = m_dodge.dodgeSwitchFrame;
        else
            m_dodgeSwitchFrame = m_dodge.dodgeStartFrame;
    }
    setPosY();

    m_timer.setInterval(m_dodge.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 受击(通用)
void CharacterModel::playHurt()
{
    if (m_hurt.cols <= 0) return;
    if (m_crouching) {
        if (m_opponent && m_knockbackToApply > 0) {
            double dist = m_knockbackToApply / 1000.0;
            if (m_cfgPosX > m_opponent->posXRatio())
                m_cfgPosX += dist;
            else
                m_cfgPosX -= dist;
            m_cfgPosX = std::max(0.0, std::min(m_cfgPosX, 3.0));
            emit posXRatioChanged();
            m_knockbackToApply = 0;
        }
        return;
    }
    m_timer.stop();
    m_state = Hurt;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_hurt.visualScale;
    m_animOffsetX = 0;
    m_currentAnim = m_hurt; // 设置当前动画参数用于位置计算
    applyAnim(m_hurt);
    applyKnockback(); // 被击中时应用击退
    m_timer.setInterval(m_hurt.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 轻度受击
void CharacterModel::playHurt1()
{
    if (m_hurt1.cols <= 0) {
        playHurt();
        return;
    }
    if (m_crouching) {
        if (m_opponent && m_knockbackToApply > 0) {
            double dist = m_knockbackToApply / 1000.0;
            if (m_cfgPosX > m_opponent->posXRatio())
                m_cfgPosX += dist;
            else
                m_cfgPosX -= dist;
            m_cfgPosX = std::max(0.0, std::min(m_cfgPosX, 3.0));
            emit posXRatioChanged();
            m_knockbackToApply = 0;
        }
        return;
    }
    m_timer.stop();
    m_state = Hurt;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_hurt1.visualScale;
    m_animOffsetX = 0;
    m_currentAnim = m_hurt1; // 设置当前动画参数用于位置计算
    applyAnim(m_hurt1);
    applyKnockback(); // 被击中时应用击退
    m_timer.setInterval(m_hurt1.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 中度受击
void CharacterModel::playHurt2()
{
    if (m_hurt2.cols <= 0) {
        playHurt();
        return;
    }
    if (m_crouching) {
        if (m_opponent && m_knockbackToApply > 0) {
            double dist = m_knockbackToApply / 1000.0;
            if (m_cfgPosX > m_opponent->posXRatio())
                m_cfgPosX += dist;
            else
                m_cfgPosX -= dist;
            m_cfgPosX = std::max(0.0, std::min(m_cfgPosX, 3.0));
            emit posXRatioChanged();
            m_knockbackToApply = 0;
        }
        return;
    }
    m_timer.stop();
    m_state = Hurt;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_hurt2.visualScale;
    m_animOffsetX = 0;
    m_currentAnim = m_hurt2; // 设置当前动画参数用于位置计算
    applyAnim(m_hurt2);
    applyKnockback(); // 被击中时应用击退
    m_timer.setInterval(m_hurt2.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 重度受击
void CharacterModel::playHurt3()
{
    if (m_hurt3.cols <= 0) {
        playHurt();
        return;
    }
    if (m_crouching) {
        if (m_opponent && m_knockbackToApply > 0) {
            double dist = m_knockbackToApply / 1000.0;
            if (m_cfgPosX > m_opponent->posXRatio())
                m_cfgPosX += dist;
            else
                m_cfgPosX -= dist;
            m_cfgPosX = std::max(0.0, std::min(m_cfgPosX, 3.0));
            emit posXRatioChanged();
            m_knockbackToApply = 0;
        }
        return;
    }
    m_timer.stop();
    m_state = Hurt;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_hurt3.visualScale;
    m_animOffsetX = 0;
    m_currentAnim = m_hurt3; // 设置当前动画参数用于位置计算
    applyAnim(m_hurt3);
    applyKnockback(); // 被击中时应用击退
    m_timer.setInterval(m_hurt3.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 下蹲: Orochi冻结在第0帧(蹲姿), Yagami循环播放
// crouchHoldFrame>0 时: 播放至停顿帧后冻结, 松手后播放起身帧->切Stand
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
    m_crouchReleased = false;
    applyAnim(m_crouch);
    if (m_crouchHoldFrame > 0) {
        m_loopAnim = false;
        m_timer.setInterval(m_crouch.interval);
        m_timer.start();
    } else if (m_crouch.loop) {
        m_timer.setInterval(m_crouch.interval);
        m_timer.start();
    }
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 下蹲攻击: 播完回到蹲姿
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

// 退出下蹲: crouchHoldFrame>0 时从停顿帧下一帧播放起身动画, 播完切Stand
void CharacterModel::stopCrouch()
{
    m_crouching = false;
    if (m_state == CrouchAttack) { playStand(); return; }
    if (m_state == Crouch && m_crouchHoldFrame > 0 && !m_crouchReleased) {
        m_crouchReleased = true;
        m_currentFrame = m_crouchHoldFrame + 1;
        if (m_currentFrame >= m_totalFrames) {
            playStand();
            return;
        }
        m_timer.setInterval(m_crouch.interval);
        m_timer.start();
        emit frameChanged();
        return;
    }
    if (m_state == Crouch || m_state == CrouchAttack) { playStand(); }
}

// 站立防御: 循环播放防御动画
void CharacterModel::playStandBlock()
{
    if (m_standBlock.cols <= 0) return;
    m_timer.stop();
    m_state = StandBlock;
    m_currentFrame = 3;
    m_loopAnim = false;
    m_visualScale = m_standBlock.visualScale;
    m_animOffsetX = m_standBlock.offsetX;
    m_hitThisAttack = false;
    m_blockReleased = false;
    applyAnim(m_standBlock);
    m_timer.setInterval(m_standBlock.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 松开防御键: 从停顿帧的下一帧继续播放, 播完切回站立
void CharacterModel::releaseStandBlock()
{
    if (m_state != StandBlock) return;
    m_blockReleased = true;
    if (m_blockHoldFrame > 0 && m_currentFrame >= m_blockHoldFrame) {
        m_currentFrame = m_blockHoldFrame + 1;
        if (m_currentFrame >= m_totalFrames) {
            // 没有剩余帧, 直接切回站立
            playStand();
            return;
        }
        m_timer.setInterval(m_standBlock.interval);
        m_timer.start();
        emit frameChanged();
    }
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
    m_crouching = false;
}

void CharacterModel::updateRootHeight(double h)
{
    m_rootHeight = h;
    setPosY();
}

void CharacterModel::applyAnim(const AnimParams &p)
{
    m_sourcePath = p.path;
    m_frameWidth = p.fw;
    m_frameHeight = p.fh;
    m_totalFrames = p.cols;
    setPosY();
}

// 计算角色Y坐标: 跳跃用抛物线, 其他用脚部/底部对齐
void CharacterModel::setPosY()
{
    if (m_state == Jump || m_state == DiagonalJump) {
        int totalFrames, localFrame;
        if (m_state == DiagonalJump) {
            totalFrames = m_djTotalFrames;
            localFrame = m_currentFrame - m_djStartFrame;
        } else {
            totalFrames = m_totalFrames;
            localFrame = m_currentFrame;
        }
        double t = 0;
        if (totalFrames > 1) t = (double) localFrame / (totalFrames - 1);
        double arcOffset = -m_jumpHeight * 4.0 * t * (1.0 - t);
        double groundY;
        if (m_stand.feetMargin > 0 && m_stand.feetBottom > 0) {
            groundY = m_rootHeight - m_stand.feetMargin - m_stand.feetBottom;
        } else {
            const AnimParams *curAnim;
            if (m_state == DiagonalJump)
                curAnim = &m_diagonalJump;
            else
                curAnim = &m_jump;
            if (curAnim->feetMargin > 0 && curAnim->feetBottom > 0) {
                groundY = m_rootHeight - curAnim->feetMargin - curAnim->feetBottom;
            } else {
                groundY = m_rootHeight - m_frameHeight - 60.0;
            }
        }
        m_posY = groundY + arcOffset;
    } else if (m_attackJumping) {
        double t = 0;
        if (m_totalFrames > 1) t = (double) m_currentFrame / (m_totalFrames - 1);
        double arcOffset = -m_jumpHeight * 4.0 * t * (1.0 - t);
        double fb = m_heavyKick.feetBottom, fm = m_heavyKick.feetMargin;
        if (fb <= 0 || fm <= 0) {
            fb = m_stand.feetBottom;
            fm = m_stand.feetMargin;
        }
        double groundY = m_rootHeight - fm - fb;
        if (m_visualScale > 1.0)
            groundY += (m_frameHeight - fb) * (m_visualScale - 1.0);
        else if (m_visualScale < 1.0)
            groundY -= (m_frameHeight - fb) * (1.0 - m_visualScale);
        m_posY = groundY + arcOffset;
    } else if (m_state == Opening || m_state == Waiting) {
        m_posY = m_rootHeight - m_frameHeight - 60.0;
    } else {
        // 优先用当前动画的 feetBottom/feetMargin，回退到 stand
        double fb = 0, fm = 0;
        switch (m_state) {
        case LightPunch:
            fb = m_lightPunch.feetBottom;
            fm = m_lightPunch.feetMargin;
            break;
        case LightKick:
            fb = m_lightKick.feetBottom;
            fm = m_lightKick.feetMargin;
            break;
        case HeavyPunch:
            fb = m_heavyPunch.feetBottom;
            fm = m_heavyPunch.feetMargin;
            break;
        case HeavyKick:
            fb = m_heavyKick.feetBottom;
            fm = m_heavyKick.feetMargin;
            break;
        case HeavyStrike:
            fb = m_heavyStrike.feetBottom;
            fm = m_heavyStrike.feetMargin;
            break;
        case Dodge:
            fb = m_dodge.feetBottom;
            fm = m_dodge.feetMargin;
            break;
        case Forward:
            fb = m_forward.feetBottom;
            fm = m_forward.feetMargin;
            break;
        case Backward:
            fb = m_backward.feetBottom;
            fm = m_backward.feetMargin;
            break;
        case Hurt:
            fb = m_currentAnim.feetBottom;
            fm = m_currentAnim.feetMargin;
            break;
        case Crouch:
            fb = m_crouch.feetBottom;
            fm = m_crouch.feetMargin;
            break;
        case CrouchAttack:
            fb = m_crouchAttack.feetBottom;
            fm = m_crouchAttack.feetMargin;
            break;
        case StandBlock:
            fb = m_standBlock.feetBottom;
            fm = m_standBlock.feetMargin;
            break;
        default:
            fb = m_stand.feetBottom;
            fm = m_stand.feetMargin;
            break;
        }
        if (fb <= 0 || fm <= 0) {
            fb = m_stand.feetBottom;
            fm = m_stand.feetMargin;
        }
        if (fb > 0 && fm > 0) {
            // HeavyStrike/Dodge: 确保底部位置与 stand 一致
            if ((m_state == HeavyStrike && m_heavyStrike.fh != m_stand.fh) || (m_state == Dodge && m_dodge.dodgeStartFrame > 0)) {
                double standBottom = m_rootHeight - m_stand.feetMargin - m_stand.feetBottom + m_stand.fh;
                if (m_state == Dodge) {
                    m_posY = standBottom - m_stand.feetBottom - m_frameHeight + (int) (fb * m_visualScale);
                } else {
                    m_posY = standBottom - m_frameHeight;
                }
            } else {
                m_posY = m_rootHeight - fm - fb;
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

// 尝试结束当前状态: 帧播完时停止定时器并切回站立, 返回true表示已处理
// 跳跃/对角线跳/闪避状态不在此处理, 由onTick中的专用逻辑触发各自完成信号
bool CharacterModel::tryFinishState()
{
    if (m_state == Jump || m_state == DiagonalJump || m_state == Dodge) return false;

    if (m_currentFrame < m_totalFrames) return false;

    m_currentFrame = m_totalFrames - 1;

    // 防御动画: 松开键后播完切站立, 未松开则停住
    if (m_state == StandBlock) {
        if (m_blockReleased) {
            m_timer.stop();
            playStand();
        } else {
            m_timer.stop();
        }
        return true;
    }

    m_timer.stop();

    State prevState = m_state;
    playStand();

    switch (prevState) {
    case Opening:
        emit openingFinished();
        break;
    case Hurt:
        emit hurtFinished();
        break;
    case LightPunch:
    case LightKick:
    case HeavyPunch:
    case HeavyKick:
    case HeavyStrike:
        if (m_attackJumping) m_attackJumping = false;
        emit attackFinished();
        break;
    default:
        break;
    }
    return true;
}

void CharacterModel::onTick()
{
    // 开场暂停帧处理
    if (m_state == Opening && m_opening.pauseFrame > 0) {
        if (m_currentFrame == m_opening.pauseFrame - 1) {
            m_currentFrame++;
            emit frameChanged();
            m_timer.setInterval(m_opening.pauseDuration);
            return;
        }
        if (m_currentFrame == m_opening.pauseFrame) { m_timer.setInterval(m_opening.interval); }
    }

    // 防御停顿: 到达停顿帧且未松开防御键时暂停定时器
    if (m_state == StandBlock && !m_blockReleased && m_blockHoldFrame > 0 && m_currentFrame >= m_blockHoldFrame) {
        m_currentFrame = m_blockHoldFrame;
        m_timer.stop();
        emit frameChanged();
        return;
    }

    // 帧推进
    m_currentFrame++;

    // 行走循环
    if ((m_state == Forward || m_state == Backward) && m_currentFrame >= m_totalFrames) {
        if (m_loopAnim) {
            m_currentFrame = 0;
        } else {
            m_currentFrame = m_totalFrames - 1;
            m_timer.stop();
            playStand();
            return;
        }
    }

    // 尝试结束当前状态(攻击/受击/开场等)
    if (tryFinishState()) return;

    // 跳跃结束
    if (m_state == Jump && m_currentFrame >= m_totalFrames) {
        m_currentFrame = m_totalFrames - 1;
        m_timer.stop();
        emit jumpFinished();
        return;
    }
    if (m_state == DiagonalJump && m_currentFrame >= m_djStartFrame + m_djTotalFrames) {
        m_currentFrame = m_djStartFrame + m_djTotalFrames - 1;
        m_timer.stop();
        if (m_opponent) m_facingLeft = (m_cfgPosX > m_opponent->posXRatio());
        emit jumpFinished();
        return;
    }

    // 下蹲停顿/起身: crouchHoldFrame>0 时, 未松手冻在停顿帧, 松手播放起身
    if (m_state == Crouch && m_crouchHoldFrame > 0) {
        if (!m_crouchReleased) {
            if (m_currentFrame >= m_crouchHoldFrame) {
                m_currentFrame = m_crouchHoldFrame;
                m_timer.stop();
                emit frameChanged();
                return;
            }
        } else {
            if (m_currentFrame >= m_totalFrames) {
                m_currentFrame = m_totalFrames - 1;
                m_timer.stop();
                playStand();
                return;
            }
        }
    }
    // 下蹲循环/冻结
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
        // 回到蹲姿
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
    // --- Dodge ---
    if (m_state == Dodge) {
        bool reachedEnd = (m_dodgeEndFrame > 0 && m_currentFrame >= m_dodgeEndFrame);
        if (!reachedEnd) {
            // 闪避方向: 对手在右则往右闪, 否则往左; 后闪取反
            auto xSign = [this]() {
                double s;
                if (m_opponent && m_cfgPosX > m_opponent->posXRatio())
                    s = -1.0;
                else
                    s = 1.0;
                if (!m_dodgeForward) s = -s;
                return s;
            };
            double dodgeDist;
            if (m_dodge.dodgeDistance > 0)
                dodgeDist = m_dodge.dodgeDistance;
            else
                dodgeDist = 0.3;

            if (m_dodge.dodgeStartFrame > 0) {
                // Orochi: 瞬移
                if (m_currentFrame == m_dodge.dodgeStartFrame) {
                    double newX = m_cfgPosX + xSign() * dodgeDist;
                    if (m_opponent) newX = std::max(0.0, std::min(newX, 3.0));
                    m_cfgPosX = newX;
                    emit posXRatioChanged();
                    if (m_opponent) m_facingLeft = (newX > m_opponent->posXRatio());
                }
                if (m_currentFrame == m_dodgeSwitchFrame) m_animOffsetX = m_dodgeOffsetXPost;
                if (m_dodgeOffsetXPost != m_dodgeOffsetXEnd && m_currentFrame >= m_dodgeEndFrame - 3
                    && m_currentFrame < m_dodgeEndFrame) {
                    int stepsLeft = m_dodgeEndFrame - m_currentFrame;
                    double t = (stepsLeft - 1) / 3.0;
                    m_animOffsetX = (int) (m_dodgeOffsetXEnd + (m_dodgeOffsetXPost - m_dodgeOffsetXEnd) * t);
                }
            } else if (m_dodge.dodgeStartFrame == 0 && m_dodge.dodgeFrames.length() > 0) {
                // Yagami: 均匀位移
                int dodgeFrameSteps = 0;
                QStringList range = m_dodgeForward ? m_dodge.dodgeFrames.split('-')
                                                   : m_dodge.dodgeBackFrames.split('-');
                if (range.size() == 2) dodgeFrameSteps = range[1].toInt() - range[0].toInt();
                if (dodgeFrameSteps > 0) {
                    double newX = m_cfgPosX + xSign() * dodgeDist / dodgeFrameSteps;
                    if (m_opponent) {
                        double oppX = m_opponent->posXRatio();
                        double bodyDist = 0.08;
                        if (xSign() > 0 && m_cfgPosX < oppX)
                            newX = std::min(newX, oppX - bodyDist);
                        else if (xSign() < 0 && m_cfgPosX > oppX)
                            newX = std::max(newX, oppX + bodyDist);
                        newX = std::max(std::max(0.0, oppX - 0.95), std::min(newX, std::min(3.0, oppX + 0.95)));
                    }
                    m_cfgPosX = newX;
                    emit posXRatioChanged();
                }
            }
        }
    }
    // Dodge 结束
    if (m_state == Dodge) {
        bool shouldFinish = (m_dodgeEndFrame > 0 && m_currentFrame >= m_dodgeEndFrame)
                            || (!m_dodgeForward && m_dodge.dodgeBackFrames.isEmpty() && m_currentFrame >= m_totalFrames);
        if (shouldFinish) {
            if (m_dodgeEndFrame > 0)
                m_currentFrame = m_dodgeEndFrame - 1;
            else
                m_currentFrame = m_totalFrames - 1;
            m_timer.stop();
            if (m_dodge.dodgeStartFrame > 0) m_animOffsetX = 0;
            m_dodgeStandPending = true;
            QTimer::singleShot(0, this, [this]() {
                if (m_dodgeStandPending) {
                    m_dodgeStandPending = false;
                    playStand();
                    emit dodgeFinished();
                }
            });
            return;
        }
    }

    if (m_state == Stand && m_currentFrame >= m_totalFrames) m_currentFrame = 0;

    // 跳跃位移+朝向
    if (m_state == DiagonalJump) {
        int localFrame = m_currentFrame - m_djStartFrame;
        double t = 0;
        if (m_djTotalFrames > 1) t = (double) localFrame / (m_djTotalFrames - 1);
        double xSign;
        if (m_djInitialFacingLeft)
            xSign = -1.0;
        else
            xSign = 1.0;
        double xDelta;
        if (m_djIsForward)
            xDelta = xSign;
        else
            xDelta = -xSign;
        double newX = m_djStartXRatio + xDelta * m_djDistance * t;
        if (m_opponent) {
            double oppX = m_opponent->posXRatio();
            newX = std::max(std::max(0.0, oppX - 0.95), std::min(newX, std::min(3.0, oppX + 0.95)));
        }
        m_cfgPosX = newX;
        m_animOffsetX = m_djOffsetFirst + (int) ((m_djOffsetLast - m_djOffsetFirst) * t);
        emit posXRatioChanged();
    } else if (m_state == Jump && m_opponent) {
        m_facingLeft = (m_cfgPosX > m_opponent->posXRatio());
    }
    if (m_state == Jump || m_state == DiagonalJump) setPosY();

    // 渐进式击退
    if (m_state == Hurt && m_knockbackFrames > 0 && m_knockbackRemaining > 0 && m_opponent) {
        double step = m_knockbackRemaining / m_knockbackFrames;
        double oppX = m_opponent->posXRatio();
        if (m_cfgPosX > oppX)
            m_cfgPosX += step;
        else
            m_cfgPosX -= step;
        m_cfgPosX = std::max(0.0, std::min(m_cfgPosX, 3.0));
        m_knockbackRemaining -= step;
        m_knockbackFrames--;
        emit posXRatioChanged();
    }

    if (m_state == HeavyStrike) emit posXRatioChanged();

    if (m_attackJumping) setPosY();

    emit frameChanged();
}

bool CharacterModel::isAttacking() const
{
    return m_state == LightPunch || m_state == LightKick || m_state == HeavyPunch || m_state == HeavyKick
           || m_state == HeavyStrike || m_state == CrouchAttack;
}

bool CharacterModel::isAttackActive() const
{
    if (!isAttacking()) return false;
    if (m_hitThisAttack) return false;
    if (m_currentAnim.attackFrames.isEmpty()) return true;

    // 解析 attackFrames: "3-5" 或 "3,5,7" 或 "3-5,8-10"
    const QStringList parts = m_currentAnim.attackFrames.split(',');
    for (const QString &part : parts) {
        if (part.contains('-')) {
            const QStringList range = part.split('-');
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
    double offsetX = m_currentAnim.attackPointX;
    if (m_facingLeft)
        return m_cfgPosX - offsetX;
    else
        return m_cfgPosX + offsetX;
}

double CharacterModel::hitboxY() const
{
    if (!isAttacking()) return 0;
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

// 被击中时应用击退(渐进式, 分散到受击动画每一帧)
void CharacterModel::applyKnockback()
{
    if (!m_opponent || m_knockbackToApply <= 0) return;

    m_knockbackRemaining = m_knockbackToApply / 1000.0;
    m_knockbackFrames = m_currentAnim.cols; // 分散到整个受击动画
    m_knockbackToApply = 0;
}
