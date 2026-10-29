#include "charactermodel.h"
#include "voicemanager.h"
#include <algorithm>


CharacterModel::CharacterModel(QObject *parent)
    : QObject(parent)
{
    m_timer.setSingleShot(false);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &CharacterModel::onTick);
    m_sunshineCooldownTimer.setInterval(1000);
    connect(&m_sunshineCooldownTimer, &QTimer::timeout, this, [this]() {
        m_sunshineCooldown = std::max(0, m_sunshineCooldown - 1);
        emit sunshineCooldownChanged();
        if (m_sunshineCooldown == 0) m_sunshineCooldownTimer.stop();
    });
}

CharacterModel::~CharacterModel()
{
    m_timer.stop();
}

void CharacterModel::configure(const CharacterData &cfg)
{
    m_timer.stop();
    if (m_sunshineGuard) { m_sunshineGuard=false; emit sunshineGuardChanged(); }
    m_sunshineRecoil = false;
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
    m_airActive=false;m_airUsed=false;m_sunshineHitMask=0;
    m_crouchKick=cfg.crouchKick;
    m_airPunch=cfg.airPunch;
    m_airKick=cfg.airKick;
    m_run=cfg.run;
    m_backstep=cfg.backstep;
    m_darkParticle = cfg.darkParticle;
    m_darkParticleReleased = false;
    m_sunshine = cfg.sunshine;
    m_sunshineCooldownTimer.stop();
    m_sunshineCooldown = 0;
    emit sunshineCooldownChanged();
    m_hurt = cfg.hurt;
    m_hurt1 = cfg.hurt1;
    m_hurt2 = cfg.hurt2;
    m_hurt3 = cfg.hurt3;
    m_rise = cfg.rise;
    m_crouch = cfg.crouch;
    m_crouchAttack = cfg.crouchAttack;
    m_dodge = cfg.dodge;
    m_standBlock = cfg.standBlock;
    m_win = cfg.win;
    m_lose = cfg.lose;
    m_blockHoldFrame = cfg.standBlock.blockHoldFrame;
    m_crouchHoldFrame = cfg.crouch.crouchHoldFrame;
    m_jumpHeight = cfg.jump.jumpHeight;
    m_charId = cfg.id;
    m_refFrameWidth = cfg.stand.fw;
    m_openingVoiceMs = cfg.opening.openingVoiceMs;
    setFacingLeft(cfg.facingLeft);
    setPosXRatio(cfg.posX);
    m_state = Waiting;
    m_currentFrame = 0;
    m_crouching = false; // 重置下蹲状态
    m_stayDown = false;  // 重置被击倒保持倒地标志
    m_pendingLose = false; // 重置失败动画等待标志
    m_knockbackToApply = 0;
    m_visualScale=m_stand.visualScale;m_animOffsetX=m_stand.offsetX;applyAnim(m_stand);
    emit frameChanged();emit sourcePathChanged();emit sizeChanged();emit stateChanged();
    emit preloadSourcesChanged();
}

QStringList CharacterModel::preloadSources() const
{
    QStringList sources;
    for (const auto *anim : {&m_stand, &m_forward, &m_backward, &m_lightPunch,
                            &m_lightKick, &m_heavyPunch, &m_heavyKick, &m_hurt,
                            &m_hurt1, &m_hurt2, &m_hurt3, &m_dodge, &m_crouch,
                            &m_crouchAttack, &m_standBlock, &m_rise, &m_darkParticle, &m_crouchKick, &m_airPunch, &m_airKick, &m_run, &m_backstep}) {
        if (anim->cols > 0 && !anim->path.isEmpty() && !sources.contains(anim->path))
            sources.append(anim->path);
    }
    return sources;
}

void CharacterModel::setStayDown(bool on)
{
    m_stayDown = on;
}

QString CharacterModel::attackAction() const {
    switch (m_state) {
    case AirPunch:
    case CrouchAttack:
    case LightPunch: return QStringLiteral("punch");
    case AirKick:
    case CrouchKick:
    case LightKick: return QStringLiteral("kick");
    case HeavyPunch: return QStringLiteral("heavyPunch");
    case HeavyKick: return QStringLiteral("heavyKick");
    case HeavyStrike: return QStringLiteral("strike");
    case DarkParticle: return QStringLiteral("strike");
    case Sunshine: return QStringLiteral("strike");
    default: return QString();
    }
}

// 播放开场: 仅Waiting状态可进入, 播完自动切Stand
void CharacterModel::playOpening()
{
    if (m_state != Waiting) return;
    VoiceManager::instance().play(m_charId, QStringLiteral("opening"));
    m_state = Opening;
    m_currentFrame = 0;
    m_loopAnim=false;m_openingElapsed=0;m_openingVoiceMs=m_opening.openingVoiceMs;
    m_visualScale=m_opening.visualScale;m_animOffsetX=m_opening.offsetX;
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
    if(m_opponent) setFacingLeft(m_cfgPosX>m_opponent->posXRatio());
    m_state = Stand;
    m_currentFrame = 0; // 从第2帧开始(避开站立的起始过渡帧)
    m_loopAnim = true;
    m_visualScale = m_stand.visualScale;     // 必须在 applyAnim 之前设置, setPosY 依赖此值
    m_animOffsetX = 0;        // 站立动画无水平偏移
    m_hitThisAttack = false;  // 重置命中标志
    m_attackJumping = false;  // 重置重腿小跳标志(回合结束/重置等强制切站立时防止残留浮动)
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
    if (m_state == Hurt || m_state == Rise) return;       // 受击/起身中不可移动
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
    emit stateChanged();
}

// 后退: 循环与否由配置决定
void CharacterModel::playBackward()
{
    if (m_backward.cols <= 0) return;                      // 未配置后退动画则忽略
    if (m_state == Backward && m_timer.isActive()) return; // 已在后退中则跳过
    if (m_state == Hurt || m_state == Rise) return;       // 受击/起身中不可移动
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
    emit stateChanged();
}

// 直跳: 不循环, 播完触发jumpFinished
void CharacterModel::playJump()
{
    VoiceManager::instance().play(m_charId, QStringLiteral("jump"));
    VoiceManager::instance().play(m_charId, QStringLiteral("step"));
    if (m_state == Hurt || m_state == Rise) return; // 受击/起身中锁定操作
    if (m_jump.cols <= 0) return;
    const bool highJump=m_state==Run;
    m_jumpStartX=m_cfgPosX;m_jumpLandingX=planLanding(m_cfgPosX);
    m_airUsed=false;m_jumpHadAttack=false;
    m_timer.stop();
    m_state = Jump;
    m_currentFrame = 0;
    m_loopAnim = m_jump.loop;
    m_jumpHeight = highJump ? m_jump.highJumpHeight : m_jump.jumpHeight;
    m_visualScale = m_jump.visualScale;
    m_animOffsetX = m_jump.offsetX;
    applyAnim(m_jump);
    beginJumpMotion(Jump,m_jump,m_jump.cols);
    m_timer.start(16);
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 对角跳: forward=true前跳(朝对手) false后跳(背向对手)
// 帧段由 divFrame 分割: 左半=右向跳跃 右半=左向跳跃
void CharacterModel::playDiagonalJump(bool forward)
{
    if (m_state == Hurt || m_state == Rise) return; // 受击/起身中锁定操作
    if (m_diagonalJump.cols <= 0 || m_diagonalJump.divFrame <= 0) return;
    const bool highJump=m_state==Run;
    m_airUsed=false;m_jumpHadAttack=false;
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
    m_jumpHeight = highJump ? m_diagonalJump.highJumpHeight : m_diagonalJump.jumpHeight;
    m_djDistance = m_diagonalJump.jumpDistance;
    m_jumpStartX=m_cfgPosX;
    const double dir=(m_djInitialFacingLeft ? -1.0 : 1.0)*(forward ? 1.0 : -1.0);
    m_jumpLandingX=planLanding(m_jumpStartX+dir*m_djDistance);
    m_visualScale = m_diagonalJump.visualScale;
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
    beginJumpMotion(DiagonalJump,m_diagonalJump,m_djTotalFrames);
    m_timer.start(16);
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 轻拳
void CharacterModel::playLightPunch()
{
    VoiceManager::instance().play(m_charId, QStringLiteral("punch"));
    if (m_state == Hurt || m_state == Rise) return; // 受击/起身中锁定操作
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
    VoiceManager::instance().play(m_charId, QStringLiteral("kick"));
    if (m_state == Hurt || m_state == Rise) return; // 受击/起身中锁定操作
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
    VoiceManager::instance().play(m_charId, QStringLiteral("heavyPunch"));
    if (m_state == Hurt || m_state == Rise) return; // 受击/起身中锁定操作
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
    VoiceManager::instance().play(m_charId, QStringLiteral("heavyKick"));
    if (m_state == Hurt || m_state == Rise) return; // 受击/起身中锁定操作
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
    VoiceManager::instance().play(m_charId, QStringLiteral("strike"));
    if (m_state == Hurt || m_state == Rise) return; // 受击/起身中锁定操作
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


bool CharacterModel::playDarkParticle()
{
    if (!hasDarkParticle() || (m_state != Stand && m_state != Forward && m_state != Backward && m_state != Run))
        return false;
    m_timer.stop();
    m_state = DarkParticle;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_crouching = false;
    m_darkParticleReleased = false;
    m_visualScale = m_darkParticle.visualScale;
    m_animOffsetX = m_darkParticle.offsetX;
    applyAnim(m_darkParticle);
    m_timer.start(m_darkParticle.interval);
    VoiceManager::instance().play(m_charId, QStringLiteral("strike"));
    emit frameChanged();emit sourcePathChanged();emit sizeChanged();emit stateChanged();
    return true;
}

void CharacterModel::playDarkParticleHurt(int hit)
{
    const bool airborne = isAirborne();
    const double fb = m_currentAnim.feetBottom > 0 ? m_currentAnim.feetBottom : m_stand.feetBottom;
    const double feet = m_posY + m_frameHeight + (fb - m_frameHeight) * m_visualScale;
    const double elevation = std::max(0.0, m_rootHeight - m_stand.feetMargin - feet);
    m_crouching = false;
    m_knockbackToApply = hit >= 3 ? 65 : 18;
    if (airborne) {
        launchSunshineHurt();
        m_flyStartHeight = elevation;
        setPosY();
        return;
    }
    if (hit >= 3) playHurt2();
    else playHurt1();
    if (m_state != Hurt) return;
    m_currentFrame = std::min(hit >= 3 ? (m_totalFrames >= 7 ? 2 : 0) : 1, m_totalFrames - 1);
    if (m_knockbackRemaining > 0) m_knockbackFrames = std::max(1, m_totalFrames - 1 - m_currentFrame);
    m_timer.start(std::max(65, m_currentAnim.interval));
    setPosY();emit frameChanged();
}

// Sunshine is a grounded, non-retriggerable super with a single active window.
bool CharacterModel::playSunshine()
{
    if (!hasSunshine() || m_sunshineCooldown > 0
        || (m_state != Stand && m_state != Forward && m_state != Backward && m_state != Run)) return false;
    m_timer.stop();
    m_state = Sunshine;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_crouching = false;
    m_attackJumping = false;
    m_hitThisAttack = false;
    m_visualScale = m_sunshine.visualScale;
    m_animOffsetX = m_sunshine.offsetX;
    m_currentAnim = m_sunshine;
    applyAnim(m_sunshine);
    m_sunshineHitMask=0;
    m_sunshineCooldown = 8;
    m_sunshineCooldownTimer.start();
    emit sunshineCooldownChanged();
    m_timer.start(m_sunshine.interval);
    VoiceManager::instance().play(m_charId, QStringLiteral("strike"));
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit stateChanged();
    return true;
}

void CharacterModel::playSunshineBlock()
{
    if (m_state != StandBlock) return;
    if(m_sunshineGuard) {m_currentFrame=std::clamp(m_blockHoldFrame,0,m_totalFrames-1);emit frameChanged();return;}
    m_sunshineGuard = true;
    m_sunshineRecoil = false;
    m_sunshineRecoilElapsed = 0;
    m_sunshineTracksCaster = m_opponent && m_opponent->state() == Sunshine;
    m_sunshineRecoilDuration = m_sunshineTracksCaster
        ? std::max(1, 50 - m_opponent->currentFrame()) * m_opponent->m_sunshine.interval
        : 1100;
    m_sunshineRecoilStartX = m_cfgPosX;
    m_sunshineRecoilDirection = m_opponent ? (m_cfgPosX > m_opponent->posXRatio() ? 1.0 : -1.0)
                                         : (m_facingLeft ? 1.0 : -1.0);
    m_sunshineRecoilDistance = .028;
    if (m_opponent)
        m_sunshineRecoilDistance = std::min(m_sunshineRecoilDistance,
            std::max(0.0, .75 - std::abs(m_cfgPosX - m_opponent->posXRatio())));
    m_knockbackToApply = 0;
    m_knockbackRemaining = 0;
    m_knockbackFrames = 0;
    m_currentFrame = std::clamp(m_blockHoldFrame, 0, m_totalFrames - 1);
    m_timer.start(30);
    emit frameChanged();
    emit sunshineGuardChanged();
}

bool CharacterModel::tickSunshineBlock()
{
    if (!m_sunshineGuard) return false;
    if (m_state != StandBlock) {
        m_sunshineGuard = false;emit sunshineGuardChanged();return false;
    }
    m_sunshineRecoilElapsed += m_timer.interval();
    const bool release = m_sunshineTracksCaster
        ? (!m_opponent || m_opponent->state() != Sunshine || m_opponent->currentFrame() >= 50)
        : m_sunshineRecoilElapsed >= m_sunshineRecoilDuration;
    const double t=std::min(1.0,double(m_sunshineRecoilElapsed)/m_sunshineRecoilDuration);
    m_cfgPosX=std::clamp(m_sunshineRecoilStartX+m_sunshineRecoilDirection
                       *m_sunshineRecoilDistance*t*t*(3.0-2.0*t),0.0,3.0);
    // Recoil through the two authored bracing poses, staying in block state.
    const int hold=std::clamp(m_blockHoldFrame,0,m_totalFrames-1);
    m_currentFrame=release ? hold : std::max(0,hold-((m_sunshineRecoilElapsed/120)%2));
    setPosY();emit posXRatioChanged();emit frameChanged();
    if (release) {
        m_sunshineGuard=false;emit sunshineGuardChanged();
        m_timer.stop();m_timer.setInterval(m_standBlock.interval);
        if (m_blockReleased) releaseStandBlock();
    }
    return true;
}

void CharacterModel::playSunshineHurt()
{
    if(m_sunshineRecoil && m_state==Hurt) {
        m_currentFrame=std::min(1,m_totalFrames-1);emit frameChanged();return;
    }
    m_crouching = false;
    m_attackJumping = false;
    m_knockbackToApply = 0;
    m_knockbackRemaining = 0;
    m_knockbackFrames = 0;
    // Stay in the light recoil sheet while the original white field is active.
    playHurt1();
    if (m_state != Hurt) return;
    m_sunshineRecoil = true;
    m_sunshineRecoilElapsed = 0;
    m_sunshineTracksCaster = m_opponent && m_opponent->state() == Sunshine;
    m_sunshineRecoilDuration = m_sunshineTracksCaster
        ? std::max(1, 50 - m_opponent->currentFrame()) * m_opponent->m_sunshine.interval
        : 1100;
    m_sunshineRecoilStartX = m_cfgPosX;
    m_sunshineRecoilDirection = m_opponent ? (m_cfgPosX > m_opponent->posXRatio() ? 1.0 : -1.0)
                                         : (m_facingLeft ? 1.0 : -1.0);
    m_sunshineRecoilDistance = 0.04;
    if (m_opponent)
        m_sunshineRecoilDistance = std::min(m_sunshineRecoilDistance,
            std::max(0.0, 0.75 - std::abs(m_cfgPosX - m_opponent->posXRatio())));
    m_timer.start(30);
}

bool CharacterModel::tickSunshineRecoil()
{
    if (!m_sunshineRecoil || m_state != Hurt) return false;
    m_sunshineRecoilElapsed += m_timer.interval();
    const bool release = m_sunshineTracksCaster
        ? (!m_opponent || m_opponent->state() != Sunshine || m_opponent->currentFrame() >= 50)
        : m_sunshineRecoilElapsed >= m_sunshineRecoilDuration;
    if (release) {
        m_sunshineRecoil = false;
        launchSunshineHurt();
        return true;
    }
    const double t = std::min(1.0, double(m_sunshineRecoilElapsed) / m_sunshineRecoilDuration);
    const double eased = t * t * (3.0 - 2.0 * t);
    m_cfgPosX = std::clamp(m_sunshineRecoilStartX + m_sunshineRecoilDirection
                         * m_sunshineRecoilDistance * eased, 0.0, 3.0);
    // Use the first backward-leaning poses, never the recovery/standing tail.
    const int poses[] = {1, 2, 3, 2};
    m_currentFrame = std::min(poses[(m_sunshineRecoilElapsed / 120) % 4], std::max(0, m_totalFrames - 2));
    setPosY();
    emit posXRatioChanged();
    emit frameChanged();
    return true;
}

void CharacterModel::launchSunshineHurt()
{
    // Full hit breaks crouching. Blocking is handled before this call.
    m_crouching = false;
    m_attackJumping = false;
    if (m_hurt3.cols > 0) {
        playHurt3();
        m_flyHeight = std::min(m_flyHeight, 180);
        m_flyDistance = std::min(m_flyDistance, 0.18);
        if (m_opponent)
            m_flyDistance = std::min(m_flyDistance,
                std::max(0.0, 0.75 - std::abs(m_cfgPosX - m_opponent->posXRatio())));
        return;
    }
    // Characters without a knockdown sheet still recoil, lift and slide back.
    playHurt();
    if (m_state != Hurt) return;
    m_flyActive = true;
    m_flyHeight = 120;
    m_flyPeakT = 0.3;
    m_flyLandT = 0.85;
    m_flyDistance = 0.18;
    if (m_opponent)
        m_flyDistance = std::min(m_flyDistance,
            std::max(0.0, 0.75 - std::abs(m_cfgPosX - m_opponent->posXRatio())));
    m_flyStartX = m_cfgPosX;
    m_flyDir = m_opponent ? (m_cfgPosX > m_opponent->posXRatio() ? 1.0 : -1.0)
                         : (m_facingLeft ? 1.0 : -1.0);
    m_knockbackRemaining = 0;
    m_knockbackFrames = 0;
    m_currentAnim.interval=std::max(120,m_hurt.interval);
    m_timer.start(16);
    setPosY();
}

// 闪避
void CharacterModel::playDodge(bool forward)
{
    if (m_state != Stand && m_state != Forward && m_state != Backward && m_state != Run) return;
    VoiceManager::instance().play(m_charId, QStringLiteral("dodge")); // 受击/起身中锁定操作
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
    m_sheetColumns = m_dodge.sheetColumns;
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
    VoiceManager::instance().play(m_charId, QStringLiteral("hurt"));
    if (m_hurt.cols <= 0) return;
    m_crouching = false;
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
    VoiceManager::instance().play(m_charId, QStringLiteral("hurt"));
    if (m_hurt1.cols <= 0) {
        playHurt();
        return;
    }
    m_crouching = false;
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
    VoiceManager::instance().play(m_charId, QStringLiteral("hurt"));
    if (m_hurt2.cols <= 0) {
        playHurt();
        return;
    }
    m_crouching = false;
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
    VoiceManager::instance().play(m_charId, QStringLiteral("hurt"));
    if (m_hurt3.cols <= 0) {
        playHurt();
        return;
    }
    m_crouching = false;
    m_timer.stop();
    m_state = Hurt;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_hurt3.visualScale;
    m_animOffsetX = 0;
    m_currentAnim = m_hurt3; // 设置当前动画参数用于位置计算
    applyAnim(m_hurt3);
    applyKnockback(); // 被击中时应用击退
    m_timer.setInterval(m_flyActive ? 16 : m_hurt3.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 起身: 击飞倒地后播放, 播完自动切Stand
void CharacterModel::playRise()
{
    if (m_rise.cols <= 0) {
        playStand();
        return;
    }
    m_timer.stop();
    m_state = Rise;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_rise.visualScale;
    m_animOffsetX = m_rise.offsetX;
    m_currentAnim = m_rise;
    applyAnim(m_rise);
    m_timer.setInterval(m_rise.interval);
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
    if (m_state == Hurt || m_state == Rise) return; // 受击/起身中锁定操作
    if (m_crouch.cols <= 0) return;
    if (m_state == Crouch || m_state == CrouchAttack || m_state == CrouchKick) return;
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
    VoiceManager::instance().play(m_charId, QStringLiteral("punch"));
    if (m_state == Hurt || m_state == Rise) return; // 受击/起身中锁定操作
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

void CharacterModel::playCrouchKick()
{
    VoiceManager::instance().play(m_charId, QStringLiteral("punch"));
    if (m_state == Hurt || m_state == Rise) return; // 受击/起身中锁定操作
    if (m_crouchKick.cols <= 0) return;
    if (m_state == CrouchKick && m_timer.isActive()) return;
    m_timer.stop();
    m_state = CrouchKick;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_crouchKick.visualScale;
    m_animOffsetX = m_crouchKick.offsetX;
    m_currentAnim = m_crouchKick;
    m_hitThisAttack = false;
    applyAnim(m_crouchKick);
    m_timer.setInterval(m_crouchKick.interval);
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
    if (m_state == CrouchAttack || m_state == CrouchKick) return; // Finish the committed strike, then stand.
    if (m_state == Crouch && m_crouchHoldFrame > 0 && !m_crouchReleased) {
        m_crouchReleased = true;
        m_currentFrame = m_crouchHoldFrame + 1;
        if (m_currentFrame >= m_totalFrames) {
            playStand();
            emit controlRecovered();
            return;
        }
        m_timer.setInterval(m_crouch.interval);
        m_timer.start();
        emit frameChanged();
        return;
    }
    if (m_state == Crouch) { playStand(); emit controlRecovered(); }
}

// 站立防御: 循环播放防御动画
void CharacterModel::playStandBlock()
{
    if (m_state == Hurt || m_state == Rise) return; // 受击/起身中锁定操作
    if (m_standBlock.cols <= 0) return;
    m_timer.stop();
    m_state = StandBlock;
    m_currentFrame = 0;
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
    if (m_sunshineGuard) return; // Finish guard stun before releasing the pose.
    if (m_blockHoldFrame > 0 && m_currentFrame >= m_blockHoldFrame) {
        m_currentFrame = m_blockHoldFrame + 1;
        if (m_currentFrame >= m_totalFrames) {
            // 没有剩余帧, 直接切回站立
            playStand();
            emit controlRecovered();
            return;
        }
        m_timer.setInterval(m_standBlock.interval);
        m_timer.start();
        emit frameChanged();
    }
}

// 标记击飞败者: 落地起身后自动播放失败动画
void CharacterModel::markPendingLose()
{
    m_pendingLose = true;
}

// 胜利动画: 三回合结束后胜者播放, 带胜利语音, 语音播完自动切站立
void CharacterModel::playWin()
{
    if (m_win.cols <= 0) {
        playStand();
        return;
    }
    VoiceManager::instance().play(m_charId, QStringLiteral("win"));
    m_timer.stop();
    m_state = Win;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_win.visualScale;
    m_animOffsetX = m_win.offsetX;
    m_currentAnim = m_win;
    m_openingVoiceMs = m_win.openingVoiceMs;
    m_openingElapsed = 0;
    m_attackJumping = false;
    applyAnim(m_win);
    m_timer.setInterval(m_win.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
}

// 失败动画: 三回合结束后败者播放, 全程沉默
void CharacterModel::playLose()
{
    if (m_lose.cols <= 0) {
        playStand();
        return;
    }
    m_timer.stop();
    m_state = Lose;
    m_currentFrame = 0;
    m_loopAnim = false;
    m_visualScale = m_lose.visualScale;
    m_animOffsetX = m_lose.offsetX;
    m_currentAnim = m_lose;
    m_openingVoiceMs = 0;
    m_attackJumping = false;
    applyAnim(m_lose);
    m_timer.setInterval(m_lose.interval);
    m_timer.start();
    emit frameChanged();
    emit sourcePathChanged();
    emit sizeChanged();
    emit positionChanged();
    emit stateChanged();
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
    if (m_sunshineGuard) { m_sunshineGuard=false;emit sunshineGuardChanged(); }
    m_sunshineRecoil = false;
    if (m_state!=AirPunch && m_state!=AirKick) m_airActive=false;
    m_currentAnim = p;
    if (m_state != HeavyKick) m_attackJumping = false;
    m_knockbackRemaining = 0;
    m_knockbackFrames = 0;
    m_sourcePath = p.path;
    m_frameWidth = p.fw;
    m_frameHeight = p.fh;
    m_totalFrames = p.cols;
    m_sheetColumns = p.sheetColumns;
    m_flyStartHeight = 0;
    m_flyElapsedMs=0;m_flyProgress=0;m_flyLastTickMs=0;m_flyClock.restart();
    m_flyActive = (p.flyHeight > 0);
    m_flyHeight = p.flyHeight;
    m_flyPeakT = p.flyPeakT;
    m_flyLandT = p.flyLandT;
    m_flyDistance = p.flyDistance;
    if (m_flyActive) {
        m_flyStartX = m_cfgPosX;
        m_flyDir = (m_opponent && m_cfgPosX > m_opponent->posXRatio()) ? 1.0 : -1.0;
    }
    setPosY();
}

// 计算角色Y坐标: 跳跃用抛物线, 其他用脚部/底部对齐
void CharacterModel::setPosY()
{
    if(m_airActive || m_state==Jump || m_state==DiagonalJump) {
        const double fb=m_currentAnim.feetBottom>0 ? m_currentAnim.feetBottom : m_stand.feetBottom;
        // One ground plane and one trajectory survive all sprite switches.
        const double fm=m_stand.feetMargin;
        const double groundY=m_rootHeight-fm-fb+(m_frameHeight-fb)*(m_visualScale-1);
        m_posY=groundY-m_jumpHeight*4*m_jumpProgress*(1-m_jumpProgress);
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
    } else if (m_flyActive) {
        // Elapsed-time motion is independent of the number of drawn poses.
        const double t=m_flyProgress;
        double arcOffset = 0;
        double peakT = m_flyPeakT, landT = m_flyLandT;
        if (t < landT) {
            if (t < peakT) {
                double u = t / peakT; // 上升: 0→1(峰)
                arcOffset = -m_flyHeight * u * (2.0 - u);
            } else {
                double u = (t - peakT) / (landT - peakT); // 下落: 0→1
                double v = 1.0 - u;
                arcOffset = -m_flyHeight * v * v;
            }
        }
        arcOffset -= m_flyStartHeight * std::max(0.0, 1.0 - t / landT);
        double fb = m_currentAnim.feetBottom, fm = m_currentAnim.feetMargin;
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
    } else if (m_state == Opening) {
        if(m_opening.feetBottom>0 && m_opening.feetMargin>0) {
            const double fb=m_opening.feetBottom;
            m_posY=m_rootHeight-m_opening.feetMargin-fb+(m_frameHeight-fb)*(m_visualScale-1);
        } else m_posY=m_rootHeight-m_frameHeight-60.0;
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
        case DarkParticle:
            fb = m_darkParticle.feetBottom;
            fm = m_darkParticle.feetMargin;
            break;
        case Sunshine:
            fb = m_sunshine.feetBottom;
            fm = m_sunshine.feetMargin;
            break;
        case HeavyStrike:
            fb = m_heavyStrike.feetBottom;
            fm = m_heavyStrike.feetMargin;
            break;
        case Dodge:
            fb = m_dodge.feetBottom;
            fm = m_dodge.feetMargin;
            break;
        case Run:
        case Backstep:
            fb=m_currentAnim.feetBottom;fm=m_currentAnim.feetMargin;break;
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
        case CrouchKick:
        case CrouchAttack:
            fb = m_currentAnim.feetBottom;
            fm = m_currentAnim.feetMargin;
            break;
        case StandBlock:
            fb = m_standBlock.feetBottom;
            fm = m_standBlock.feetMargin;
            break;
        case Rise:
            fb = m_rise.feetBottom;
            fm = m_rise.feetMargin;
            break;
        case Win:
            fb = m_win.feetBottom;
            fm = m_win.feetMargin;
            break;
        case Lose:
            fb = m_lose.feetBottom;
            fm = m_lose.feetMargin;
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
            m_posY = m_rootHeight - fm - fb
                     + (m_frameHeight - fb) * (m_visualScale - 1.0);
        } else {
            m_posY = m_rootHeight - m_frameHeight - 60.0;
        }
    }
    if(m_state==Backstep) {
        const double t=double(m_currentFrame)/std::max(1,m_totalFrames-1);
        m_posY-=45*4*t*(1-t);
    }
    emit positionChanged();
}

// 尝试结束当前状态: 帧播完时停止定时器并切回站立, 返回true表示已处理
// 跳跃/对角线跳/闪避状态不在此处理, 由onTick中的专用逻辑触发各自完成信号
bool CharacterModel::tryFinishState()
{
    if (m_state == Jump || m_state == DiagonalJump || m_state == Dodge
        || m_state == Crouch || m_state == CrouchAttack || m_state == CrouchKick || m_state == Stand || m_state==Run) return false;

    if (m_currentFrame < m_totalFrames) return false;

    m_currentFrame = m_totalFrames - 1;

    // 防御动画: 松开键后播完切站立, 未松开则停住
    if (m_state == StandBlock) {
        if (m_blockReleased) {
            m_timer.stop();
            playStand();
            emit controlRecovered();
        } else {
            m_timer.stop();
        }
        return true;
    }

    m_timer.stop();

    State prevState = m_state;
    // 胜利语音持帧: 帧播完后保持最后一帧直到语音播完
    if (prevState == Win && m_openingVoiceMs > 0 && m_openingElapsed < m_openingVoiceMs) {
        m_timer.start();
        return true;
    }
    if (prevState == Opening && m_openingVoiceMs > 0 && m_openingElapsed < m_openingVoiceMs) {
        m_timer.start();
        return true;
    }
    if (m_stayDown && prevState == Hurt && m_flyActive) {
        emit hurtFinished();
        return true;
    }
    if (prevState == Hurt && m_currentAnim.riseFollowUp > 0 && m_rise.cols > 0) {
        playRise();
        return true;
    }
    // 击飞败者落地起身后自动播放失败动画
    if (m_pendingLose && (prevState == Hurt || prevState == Rise)) {
        m_pendingLose = false;
        playLose();
        return true;
    }
    // 胜利/失败动画: 保持最终姿态定格, 不切回站立 (由战斗画面停留后退出)
    if (prevState == Win) {
        m_timer.stop();
        emit winFinished();
        return true;
    }
    if (prevState == Lose) {
        m_timer.stop();
        emit loseFinished();
        return true;
    }
    playStand();

    switch (prevState) {
    case Opening:
        emit openingFinished();
        break;
    case Backstep:
        emit dodgeFinished();
        break;
    case Hurt:
    case Rise:
        emit hurtFinished();
        break;
    case LightPunch:
    case LightKick:
    case HeavyPunch:
    case HeavyKick:
    case HeavyStrike:
    case Sunshine:
    case DarkParticle:
        if (m_attackJumping) m_attackJumping = false;
        emit attackFinished();
        break;
    default:
        break;
    }
    return true;
}

void CharacterModel::advanceKnockdown(int elapsedMs)
{
    if(!hasKnockdownMotion() || elapsedMs<=0)return;
    const int interval=std::max(1,m_currentAnim.interval);
    const int duration=std::max(1,m_totalFrames)*interval;
    if(m_flyElapsedMs>=duration)return;
    m_flyElapsedMs=std::min(duration,m_flyElapsedMs+elapsedMs);
    m_flyProgress=std::min(1.0,double(m_flyElapsedMs)/std::max(1,(m_totalFrames-1)*interval));
    m_currentFrame=std::min(m_totalFrames-1,m_flyElapsedMs/interval);
    const double slide=std::min(1.0,m_flyProgress/std::max(.001,m_flyLandT));
    if(m_flyDistance>0 && m_opponent)
        setPosXRatio(std::clamp(m_flyStartX+m_flyDir*m_flyDistance*slide,0.0,3.0));
    setPosY();emit frameChanged();
    if(m_flyElapsedMs>=duration){m_currentFrame=m_totalFrames;tryFinishState();}
}

void CharacterModel::onTick()
{
    if(m_airActive || m_state==Jump || m_state==DiagonalJump) {
        const qint64 now=m_jumpClock.elapsed();
        const int dt=int(now-m_jumpLastTickMs);m_jumpLastTickMs=now;
        advanceJump(std::clamp(dt,1,50));return;
    }
    if(hasKnockdownMotion()) {
        const qint64 now=m_flyClock.elapsed();
        const int dt=int(now-m_flyLastTickMs);m_flyLastTickMs=now;
        advanceKnockdown(std::clamp(dt,1,50));return;
    }
    if (tickSunshineBlock()) return;
    if (tickSunshineRecoil()) return;
    if (m_state == Opening || m_state == Win)
        m_openingElapsed += m_timer.interval();
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

    if (m_state == DarkParticle && m_currentFrame >= 14 && !m_darkParticleReleased) {
        m_darkParticleReleased = true;
        emit darkParticleReleased();
    }

    // 行走循环
    if ((m_state == Forward || m_state == Backward || m_state==Run) && m_currentFrame >= m_totalFrames) {
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
                emit controlRecovered();
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
    if ((m_state == CrouchAttack || m_state==CrouchKick) && m_currentFrame >= m_totalFrames) {
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
            m_crouchReleased = false;
            if (m_crouchHoldFrame > 0) m_currentFrame = m_crouchHoldFrame;
            if (m_crouch.loop) m_timer.start(m_crouch.interval);
            else m_timer.stop();
            emit frameChanged();
            emit sourcePathChanged();
            emit sizeChanged();
            emit stateChanged();
        } else {
            playStand();
        }
        emit attackFinished();
        return;
    }
    if(m_state==Backstep) {
        const double t=double(m_currentFrame)/std::max(1,m_totalFrames-1);
        double x=m_backstepStartX+m_backstepDirection*.18*std::min(1.0,t);
        if(m_opponent)x=std::clamp(x,std::max(0.0,m_opponent->posXRatio()-.9),std::min(3.0,m_opponent->posXRatio()+.9));
        setPosXRatio(x);setPosY();
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
                    if (m_opponent) setFacingLeft(newX > m_opponent->posXRatio());
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
            playStand();
            emit dodgeFinished();
            return;
        }
    }

    if (m_state == Stand && m_currentFrame >= m_totalFrames) m_currentFrame = 0;


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
    if (m_flyActive) setPosY();

    emit frameChanged();
}

bool CharacterModel::isAttacking() const
{
    return m_state == LightPunch || m_state == LightKick || m_state == HeavyPunch || m_state == HeavyKick
           || m_state == HeavyStrike || m_state == CrouchAttack || m_state == Sunshine || m_state == DarkParticle || m_state==CrouchKick || m_state==AirPunch || m_state==AirKick;
}

bool CharacterModel::isAttackActive() const
{
    if (m_state == DarkParticle) return false;
    if(m_state==Sunshine) {
        const int part=sunshineHitPart();
        return part>0 && !(m_sunshineHitMask & (1<<(part-1)));
    }
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
    double offsetX=m_currentAnim.attackPointX;
    if(m_currentFrame<m_currentAnim.attackContacts.size())
        offsetX=m_currentAnim.attackContacts[m_currentFrame].x()/900.0;
    if (m_facingLeft)
        return m_cfgPosX - offsetX;
    else
        return m_cfgPosX + offsetX;
}

double CharacterModel::hitboxY() const
{
    if (!isAttacking()) return 0;
    const double fb=m_currentAnim.feetBottom>0 ? m_currentAnim.feetBottom : m_stand.feetBottom;
    const double feet=m_posY+m_frameHeight+(fb-m_frameHeight)*m_visualScale;
    if(m_currentFrame<m_currentAnim.attackContacts.size())
        return feet-m_currentAnim.attackContacts[m_currentFrame].y();
    return feet-145+m_currentAnim.attackPointY;
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
    const double fb=m_currentAnim.feetBottom>0 ? m_currentAnim.feetBottom : m_stand.feetBottom;
    return m_posY+m_frameHeight+(fb-m_frameHeight)*m_visualScale-m_hurtboxH/2.0;
}

// 被击中时应用击退(渐进式, 分散到受击动画每一帧)
void CharacterModel::applyKnockback()
{
    if (!m_opponent || m_knockbackToApply <= 0) return;
    // 击飞动画水平位移由flyDistance控制, 不叠加普通击退
    if (m_currentAnim.flyDistance > 0) {
        m_knockbackToApply = 0;
        return;
    }

    m_knockbackRemaining = m_knockbackToApply / 1000.0;
    m_knockbackFrames = std::max(1, m_currentAnim.cols - 1); // 分散到整个受击动画
    m_knockbackToApply = 0;
}

bool CharacterModel::playRun()
{
    if (m_run.cols<=0 || (m_state!=Stand && m_state!=Forward && m_state!=Backward && m_state!=Run)) return false;
    if(m_state==Run) return true;
    m_timer.stop();m_state=Run;m_currentFrame=0;m_loopAnim=true;
    m_visualScale=m_run.visualScale;m_animOffsetX=m_run.offsetX;applyAnim(m_run);m_timer.start(m_run.interval);
    emit frameChanged();emit sourcePathChanged();emit sizeChanged();emit stateChanged();return true;
}

bool CharacterModel::playBackstep()
{
    if(m_backstep.cols<=0 || (m_state!=Stand && m_state!=Forward && m_state!=Backward && m_state!=Run)) return false;
    m_timer.stop();m_state=Backstep;m_currentFrame=0;m_loopAnim=false;
    m_backstepStartX=m_cfgPosX;m_backstepDirection=m_facingLeft ? 1.0 : -1.0;
    m_visualScale=m_backstep.visualScale;m_animOffsetX=m_backstep.offsetX;applyAnim(m_backstep);m_timer.start(m_backstep.interval);
    emit frameChanged();emit sourcePathChanged();emit sizeChanged();emit stateChanged();return true;
}

bool CharacterModel::playContextAttack(bool kick,bool heavy)
{
    if(m_state==Jump || m_state==DiagonalJump) return playAirAttack(kick,heavy);
    if(m_state!=Crouch) return false;
    if(kick) playCrouchKick();else playCrouchAttack();
    if(m_state!=CrouchAttack && m_state!=CrouchKick) return false;
    if(heavy) {m_currentAnim.damage+=3;m_currentAnim.knockbackDistance+=15;}
    return true;
}

void CharacterModel::finishLanding(bool attacked)
{
    m_timer.stop();m_airActive=false;m_attackJumping=false;m_crouching=false;
    // Complete the precomputed trajectory; never shove the jumper on landing.
    setPosXRatio(m_jumpLandingX);
    if(m_opponent)setFacingLeft(m_cfgPosX>m_opponent->posXRatio());
    playStand();
    if(attacked)emit attackFinished();
    emit jumpFinished();
}

void CharacterModel::beginJumpMotion(State kind,const AnimParams &anim,int count)
{
    m_airJumpState=kind;m_airJumpAnim=anim;m_airJumpInterval=anim.interval;
    m_jumpElapsedMs=0;m_jumpProgress=0;m_airAttackElapsedMs=0;
    m_jumpClock.restart();m_jumpLastTickMs=0;
    m_jumpDurationMs=std::max(1,(count-1)*anim.interval);
    m_airJumpFrame=kind==DiagonalJump ? m_djStartFrame : 0;
    setPosY();
}

bool CharacterModel::playAirAttack(bool kick,bool heavy)
{
    const auto &anim=kick ? m_airKick : m_airPunch;
    if(anim.cols<=0 || !canAirAttack())return false;
    m_airUsed=true;m_jumpHadAttack=true;m_airAttackElapsedMs=0;
    m_timer.stop();m_state=kick ? AirKick : AirPunch;m_currentFrame=0;m_loopAnim=false;
    m_visualScale=anim.visualScale;m_animOffsetX=anim.offsetX;m_hitThisAttack=false;
    applyAnim(anim);m_airActive=true;
    if(heavy){m_currentAnim.damage+=3;m_currentAnim.knockbackDistance+=15;}
    setPosY();m_timer.start(16);
    VoiceManager::instance().play(m_charId,kick ? QStringLiteral("kick") : QStringLiteral("punch"));
    emit frameChanged();emit sourcePathChanged();emit sizeChanged();emit stateChanged();return true;
}

void CharacterModel::recoverAirAttack()
{
    m_airActive=false;m_state=m_airJumpState;
    m_visualScale=m_airJumpAnim.visualScale;m_animOffsetX=m_airJumpAnim.offsetX;
    applyAnim(m_airJumpAnim);m_currentFrame=m_airJumpFrame;
    m_hitThisAttack=true;
    emit sourcePathChanged();emit sizeChanged();emit stateChanged();
}

void CharacterModel::advanceJump(int elapsedMs)
{
    if(!(m_airActive || m_state==Jump || m_state==DiagonalJump) || elapsedMs<=0)return;
    // Physics is independent of attack frames and their playback interval.
    m_jumpElapsedMs=std::min(m_jumpDurationMs,m_jumpElapsedMs+elapsedMs);
    m_jumpProgress=double(m_jumpElapsedMs)/m_jumpDurationMs;
    const int count=m_airJumpState==DiagonalJump ? m_djTotalFrames : m_airJumpAnim.cols;
    const int start=m_airJumpState==DiagonalJump ? m_djStartFrame : 0;
    m_airJumpFrame=start+std::min(count-1,m_jumpElapsedMs/std::max(1,m_airJumpInterval));
    if(m_jumpElapsedMs>=m_jumpDurationMs) {
        m_jumpProgress=1;finishLanding(m_jumpHadAttack);return;
    }
    if(m_airActive) {
        m_airAttackElapsedMs+=elapsedMs;
        if(m_airAttackElapsedMs>=m_totalFrames*m_currentAnim.interval)recoverAirAttack();
        else m_currentFrame=m_airAttackElapsedMs/std::max(1,m_currentAnim.interval);
    } else m_currentFrame=m_airJumpFrame;
    updateJumpPosition();setPosY();
    emit positionChanged();emit frameChanged();
}

bool CharacterModel::canAirAttack() const
{
    // Accept ascending, apex and descending attacks. A press immediately after
    // takeoff is valid; landing cancels unfinished attacks without ground hits.
    return !m_airUsed && (m_state==Jump || m_state==DiagonalJump)
        && (m_airPunch.cols>0 || m_airKick.cols>0) && m_jumpProgress<1;
}

double CharacterModel::planLanding(double desired) const
{
    return std::clamp(desired,0.0,3.0);
}

void CharacterModel::updateJumpPosition()
{
    setPosXRatio(m_jumpStartX+(m_jumpLandingX-m_jumpStartX)*m_jumpProgress);
    if(!m_airActive && m_airJumpState==DiagonalJump)
        m_animOffsetX=m_djOffsetFirst+int((m_djOffsetLast-m_djOffsetFirst)*m_jumpProgress);
}

int CharacterModel::sunshineHitPart() const
{
    if(m_state!=Sunshine || m_currentFrame<28 || m_currentFrame>49)return 0;
    const int frames[]={28,32,36,40,44,49};int part=0;
    for(int i=0;i<6;++i)if(m_currentFrame>=frames[i])part=i+1;
    return part;
}
