#include "fightdirector.h"
#include "voicemanager.h"
#include "charactermodel.h"
#include "characterconfig.h"
#include "keybindingconfig.h"
#include <QDebug>
#include <algorithm>
#include <cmath>

// 构造函数: 创建P1/P2角色模型, 连接开场完成信号, 加载JSON角色配置
FightDirector::FightDirector(QObject *parent)
    : QObject(parent)
{
    m_p1Model = new CharacterModel(this);
    m_p2Model = new CharacterModel(this);
    m_keyBindings = new KeyBindingConfig(this);
    m_cameraTimer.setInterval(16);m_cameraTimer.setTimerType(Qt::PreciseTimer);
    connect(&m_cameraTimer,&QTimer::timeout,this,&FightDirector::onCameraTick);
    m_projectileTimer.setInterval(16);
    m_projectileTimer.setTimerType(Qt::PreciseTimer);
    connect(&m_projectileTimer,&QTimer::timeout,this,[this]() {
        advanceDarkParticles(int(m_projectileClock.restart()));
    });
    connect(m_p1Model,&CharacterModel::darkParticleReleased,this,[this]() {spawnDarkParticle(1);});
    connect(m_p2Model,&CharacterModel::darkParticleReleased,this,[this]() {spawnDarkParticle(2);});

    connect(m_p1Model, &CharacterModel::openingFinished, this, &FightDirector::onP1OpeningFinished);
    connect(m_p2Model, &CharacterModel::openingFinished, this, &FightDirector::onP2OpeningFinished);
    connect(m_p1Model, &CharacterModel::winFinished, this, &FightDirector::onP1WinFinished);
    connect(m_p2Model, &CharacterModel::winFinished, this, &FightDirector::onP2WinFinished);

    QFile file(":/config/characters.json");
    if (file.open(QIODevice::ReadOnly)) {
        m_jsonConfig = QJsonDocument::fromJson(file.readAll()).object();
        file.close();
    } else {
        qWarning() << "[FightDirector] Cannot load characters.json";
    }

    // 加载键位配置: 优先读外部文件, 回退到资源默认
    if (!m_keyBindings->loadFromFile("config/keybindings.json")) {
        m_keyBindings->loadFromFile(":/config/keybindings.json");
    }
}

// 重新加载键位配置
void FightDirector::reloadKeyBindings()
{
    if (m_keyBindings) {
        if (!m_keyBindings->loadFromFile("config/keybindings.json")) {
            m_keyBindings->loadFromFile(":/config/keybindings.json");
        }
    }
}

// QML 窗口高度变化时同步更新两个角色的Y坐标计算基线
void FightDirector::setRootHeight(double h)
{
    m_rootHeight = h;
    if (m_p1Model) m_p1Model->updateRootHeight(h);
    if (m_p2Model) m_p2Model->updateRootHeight(h);
    emit rootHeightChanged();
}

// 开始战斗: 加载双方角色配置, 先播放P1开场动画
void FightDirector::start(const QString &p1CharId, const QString &p2CharId)
{
    m_cameraTimer.stop();m_cameraTarget=0;m_cameraOffset=0;
    clearDarkParticles();
    qDebug() << "[FightDirector] Starting fight:" << p1CharId << "vs" << p2CharId;

    loadConfig(p1CharId, m_p1Model);
    loadConfig(p2CharId, m_p2Model);

    m_p1Model->setOpponent(m_p2Model);
    m_p2Model->setOpponent(m_p1Model);

    m_p1Model->m_rootHeight = m_rootHeight;
    m_p2Model->m_rootHeight = m_rootHeight;

    // P1始终在左侧朝右, P2始终在右侧朝左
    m_p1Model->setPosXRatio(0.22);
    m_p1Model->setFacingLeft(false);
    m_p2Model->setPosXRatio(0.78);
    m_p2Model->setFacingLeft(true);

    m_cameraTimer.stop();m_cameraTarget=0;m_cameraOffset=0;emit cameraOffsetChanged();
    m_phase = Opening_P1;
    emit phaseChanged();

    if (m_previewWin) {
        // 调试预览: 用胜利动画+语音替代开场, 方便开局查看胜行动作效果
        qDebug() << "[FightDirector] previewWin mode: playing win as opening";
        m_p1Model->playWin();
    } else {
        m_p1Model->playOpening(); // 先播放P1的开场
    }
}

// 回合重置: 加载配置, 直接进入战斗阶段, 跳过开场动画
void FightDirector::resetForNewRound(const QString &p1CharId, const QString &p2CharId)
{
    m_cameraTimer.stop();m_cameraTarget=0;m_cameraOffset=0;
    clearDarkParticles();
    qDebug() << "[FightDirector] Reset for new round:" << p1CharId << "vs" << p2CharId;

    loadConfig(p1CharId, m_p1Model);
    loadConfig(p2CharId, m_p2Model);

    m_p1Model->setOpponent(m_p2Model);
    m_p2Model->setOpponent(m_p1Model);

    m_p1Model->m_rootHeight = m_rootHeight;
    m_p2Model->m_rootHeight = m_rootHeight;

    // P1始终在左侧朝右, P2始终在右侧朝左
    m_p1Model->setPosXRatio(0.22);
    m_p1Model->setFacingLeft(false);
    m_p2Model->setPosXRatio(0.78);
    m_p2Model->setFacingLeft(true);

    // 直接进入战斗阶段，跳过开场动画
    m_phase = Fighting;
    m_cameraTimer.stop();m_cameraTarget=0;
    m_cameraOffset = 0; // 重置镜头位置

    // 让角色进入站立状态
    m_p1Model->playStand();
    m_p2Model->playStand();

    emit phaseChanged();
    emit cameraOffsetChanged();
}

// 从 JSON 中查找角色ID对应的配置, 解析并注入到 CharacterModel
void FightDirector::loadConfig(const QString &charId, CharacterModel *model)
{
    QJsonObject obj = m_jsonConfig.value(charId).toObject();
    if (obj.isEmpty()) {
        qWarning() << "[FightDirector] Unknown character:" << charId;
        return;
    }
    CharacterData cfg = CharacterConfig::load(obj, charId);
    model->configure(cfg);
}

// P1开场结束回调: 进入P2开场阶段
void FightDirector::onP1OpeningFinished()
{
    qDebug() << "[FightDirector] P1 opening finished, starting P2 opening";
    m_phase = Opening_P2;
    emit phaseChanged();
    m_p2Model->playOpening();
}

// P2开场结束回调: 双方入场完毕, 进入自由战斗阶段
void FightDirector::onP2OpeningFinished()
{
    qDebug() << "[FightDirector] P2 opening finished, fight begins";
    m_phase = Fighting;
    emit phaseChanged();
}

// 预览模式: P1胜行动画播完, 播放P2胜行动画
void FightDirector::onP1WinFinished()
{
    if (!m_previewWin || m_phase != Opening_P1) return;
    qDebug() << "[FightDirector] P1 win preview finished, starting P2";
    m_phase = Opening_P2;
    emit phaseChanged();
    m_p2Model->playWin();
}

// 预览模式: P2胜行动画播完, 进入战斗阶段
void FightDirector::onP2WinFinished()
{
    if (!m_previewWin || m_phase != Opening_P2) return;
    qDebug() << "[FightDirector] P2 win preview finished, fight begins";
    m_phase = Fighting;
    emit phaseChanged();
    // 胜利姿态冻结了, 进入战斗前复位为站立
    m_p1Model->playStand();
    m_p2Model->playStand();
}

// 格斗运镜: 始终确保两角色在屏内, 容不下时回退中点跟随
// 小幅抖动通过平滑插值消除, 大幅移动直接跟随
void FightDirector::updateCamera()
{
    if(!m_p1Model || !m_p2Model) return;
    const double left=std::min(m_p1Model->posXRatio(),m_p2Model->posXRatio());
    const double right=std::max(m_p1Model->posXRatio(),m_p2Model->posXRatio());
    const double lo=right-.92,hi=left-.08;
    // Keep the existing composition while both fighters are safely visible.
    // Teleporting inside the viewport must not recenter the stage in two jolts.
    double target=hi>=lo ? std::clamp(m_cameraOffset,lo,hi) : (left+right)/2-.5;
    target=std::clamp(target,0.0,kStageWidth-1.0);
    m_cameraTarget=target;
    if(std::abs(target-m_cameraOffset)<.5/900.0) return;
    if(!m_cameraTimer.isActive())m_cameraTimer.start();
}

void FightDirector::onCameraTick()
{
    const double diff=m_cameraTarget-m_cameraOffset;
    if(std::abs(diff)<.5/900.0) {
        m_cameraTimer.stop();
        if(m_cameraOffset!=m_cameraTarget) {m_cameraOffset=m_cameraTarget;emit cameraOffsetChanged();}
        return;
    }
    m_cameraOffset+=diff*.28;
    emit cameraOffsetChanged();
}

// 碰撞检测: 检查攻击框是否命中对手受击框
bool FightDirector::checkCollision()
{
    // 窗口尺寸
    const double windowW = 900.0;
    const double windowH = 640.0;
    bool anyHit = false;

    // 检查 P1 是否攻击 P2 (defender击飞中或起身中免疫)
    if (m_p1Model->isAttacking() && m_p1Model->isAttackActive()
        && m_p2Model->state() != CharacterModel::Dodge
        && !(m_p2Model->m_flyActive || m_p2Model->state() == CharacterModel::Rise)) {
        double hitNormX = m_p1Model->hitboxX();
        double hitY = m_p1Model->hitboxY();
        double hitR = m_p1Model->hitboxRadius();

        double hurtNormX = m_p2Model->hurtboxX();
        double hurtY = m_p2Model->hurtboxY();
        double hurtW = m_p2Model->hurtboxW();
        double hurtH = m_p2Model->hurtboxH();

        double hitPx = hitNormX * windowW;
        double hurtPx = hurtNormX * windowW;

        const bool fullScreen = m_p1Model->state() == CharacterModel::Sunshine;
        bool hitX = fullScreen || ((hitPx + hitR >= hurtPx - hurtW / 2) && (hitPx - hitR <= hurtPx + hurtW / 2));
        bool hitYRange = fullScreen || ((hitY + hitR >= hurtY - hurtH / 2) && (hitY - hitR <= hurtY + hurtH / 2));

        if (hitX && hitYRange) {
            const int part = fullScreen ? m_p1Model->sunshineHitPart() : 0;
            if (fullScreen) m_p1Model->m_sunshineHitMask |= 1 << (part-1);
            else m_p1Model->m_hitThisAttack = true;
            const int totalDamage = m_p1Model->m_currentAnim.damage;
            int finalDamage = fullScreen ? totalDamage/6 + (part==6 ? totalDamage%6 : 0) : totalDamage;
            int finalKnockback = fullScreen ? 0 : m_p1Model->m_currentAnim.knockbackDistance;
            if (m_p2Model->isBlocking()) {
                finalDamage = std::max(1, finalDamage / 2); // 50%减伤
                finalKnockback /= 2;
            }
            m_p2Model->m_knockbackToApply = finalKnockback;
            // 命中/格挡音效在 emit 之前播放, 确保攻击者仍处于攻击状态, attackAction() 有效
            if (m_p2Model->isBlocking()) {
                VoiceManager::instance().play(m_p1Model->charId(), QStringLiteral("step"));
            } else {
                VoiceManager::instance().play(m_p1Model->charId(), QStringLiteral("hit") + m_p1Model->attackAction());
            }
            setLastHitContext(m_p1Model->state(),part);
            emit hitDetected(1, finalDamage);
            anyHit = true;
        }
    }

    // 检查 P2 是否攻击 P1 (defender击飞中或起身中免疫)
    if (m_p2Model->isAttacking() && m_p2Model->isAttackActive()
        && m_p1Model->state() != CharacterModel::Dodge
        && !(m_p1Model->m_flyActive || m_p1Model->state() == CharacterModel::Rise)) {
        double hitNormX = m_p2Model->hitboxX();
        double hitY = m_p2Model->hitboxY();
        double hitR = m_p2Model->hitboxRadius();

        double hurtNormX = m_p1Model->hurtboxX();
        double hurtY = m_p1Model->hurtboxY();
        double hurtW = m_p1Model->hurtboxW();
        double hurtH = m_p1Model->hurtboxH();

        double hitPx = hitNormX * windowW;
        double hurtPx = hurtNormX * windowW;

        const bool fullScreen = m_p2Model->state() == CharacterModel::Sunshine;
        bool hitX = fullScreen || ((hitPx + hitR >= hurtPx - hurtW / 2) && (hitPx - hitR <= hurtPx + hurtW / 2));
        bool hitYRange = fullScreen || ((hitY + hitR >= hurtY - hurtH / 2) && (hitY - hitR <= hurtY + hurtH / 2));

        if (hitX && hitYRange) {
            const int part = fullScreen ? m_p2Model->sunshineHitPart() : 0;
            if (fullScreen) m_p2Model->m_sunshineHitMask |= 1 << (part-1);
            else m_p2Model->m_hitThisAttack = true;
            const int totalDamage = m_p2Model->m_currentAnim.damage;
            int finalDamage = fullScreen ? totalDamage/6 + (part==6 ? totalDamage%6 : 0) : totalDamage;
            int finalKnockback = fullScreen ? 0 : m_p2Model->m_currentAnim.knockbackDistance;
            if (m_p1Model->isBlocking()) {
                finalDamage = std::max(1, finalDamage / 2); // 50%减伤
                finalKnockback /= 2;
            }
            m_p1Model->m_knockbackToApply = finalKnockback;
            // 命中/格挡音效在 emit 之前播放, 确保攻击者仍处于攻击状态, attackAction() 有效
            if (m_p1Model->isBlocking()) {
                VoiceManager::instance().play(m_p2Model->charId(), QStringLiteral("step"));
            } else {
                VoiceManager::instance().play(m_p2Model->charId(), QStringLiteral("hit") + m_p2Model->attackAction());
            }
            setLastHitContext(m_p2Model->state(),part);
            emit hitDetected(2, finalDamage);
            anyHit = true;
        }
    }

    return anyHit;
}

QVariantList FightDirector::darkParticles() const
{
    QVariantList result;
    for (const auto &p : m_darkParticles)
        result.append(QVariantMap{{"id",p.id},{"owner",p.owner},{"x",p.x},
            {"height",p.height},{"direction",p.direction},{"age",p.age},
            {"hits",p.hits},{"pulse",p.pulse},{"dying",p.dying}});
    return result;
}

void FightDirector::setLastHitContext(int state, int part)
{
    m_lastHitState = state;
    m_lastHitPart = part;
    emit hitContextChanged();
}

void FightDirector::clearDarkParticles()
{
    m_darkParticles.clear();m_projectileTimer.stop();
    emit darkParticlesChanged();
}

void FightDirector::spawnDarkParticle(int owner)
{
    if (!m_authoritative || m_phase != Fighting) return;
    auto caster = owner == 1 ? m_p1Model : m_p2Model;
    if (caster->state() != CharacterModel::DarkParticle) return;
    // At most one wave per player. A later cast can start after recovery,
    // but its release waits for the existing wave to leave the arena.
    for (const auto &p : m_darkParticles) if (p.owner == owner) return;
    DarkWave wave;
    wave.id = ++m_nextProjectileId;wave.owner = owner;
    wave.direction = caster->facingLeft() ? -1.0 : 1.0;
    wave.x = caster->posXRatio() + wave.direction * .18;
    m_darkParticles.append(wave);
    if (!m_projectileTimer.isActive()) { m_projectileClock.restart();m_projectileTimer.start(); }
    emit darkParticlesChanged();
}

void FightDirector::syncDarkParticles(const QVariantList &snapshot)
{
    if (m_authoritative) return;
    m_darkParticles.clear();
    for (const auto &value : snapshot) {
        const auto data = value.toMap();DarkWave p;
        p.id=data.value("id").toInt();p.owner=data.value("owner").toInt();
        if (p.owner != 1 && p.owner != 2) continue;
        p.x=data.value("x").toDouble();p.height=data.value("height").toDouble();
        p.direction=data.value("direction").toDouble();p.age=data.value("age").toInt();
        p.hits=data.value("hits").toInt();p.pulse=data.value("pulse").toInt();
        p.dying=data.value("dying").toInt();m_darkParticles.append(p);
    }
    if (m_darkParticles.isEmpty()) m_projectileTimer.stop();
    else if (!m_projectileTimer.isActive()) { m_projectileClock.restart();m_projectileTimer.start(); }
    emit darkParticlesChanged();
}

void FightDirector::advanceDarkParticles(int elapsedMs)
{
    // Fixed upper step prevents tunnelling after a delayed frame. Tests and
    // network clients use the same simulation, only the host applies damage.
    const int dt=std::clamp(elapsedMs,1,50);
    for (int index=0;index<m_darkParticles.size();) {
        auto &p=m_darkParticles[index];p.age+=dt;p.pulse-=dt;
        auto target=p.owner==1 ? m_p2Model : m_p1Model;
        if (p.dying>0) {
            p.dying-=dt;
            if(p.dying<=0) {m_darkParticles.removeAt(index);continue;}
        } else {
            const double oldX=p.x;
            if(p.hits==0) p.x+=p.direction*.72*dt/1000.0;
            else p.x=target->posXRatio()-p.direction*.045;
            const double fb=target->m_currentAnim.feetBottom>0
                ? target->m_currentAnim.feetBottom : target->m_stand.feetBottom;
            const double feet=target->m_posY+target->m_frameHeight
                +(fb-target->m_frameHeight)*target->m_visualScale;
            const double centerHeight=target->m_rootHeight-feet+target->m_hurtboxH/2.0;
            const double waveHeight=target->m_stand.feetMargin+p.height;
            const double reach=(32.0+target->m_hurtboxW/2.0)/900.0;
            const bool hitX=target->posXRatio()>=std::min(oldX,p.x)-reach
                && target->posXRatio()<=std::max(oldX,p.x)+reach;
            const bool hitY=std::abs(centerHeight-waveHeight)<=145.0+target->m_hurtboxH/2.0;
            const bool immune=target->state()==CharacterModel::Dodge
                ||target->state()==CharacterModel::Rise||target->m_flyActive;
            if(m_authoritative && m_phase==Fighting && p.pulse<=0 && hitX && hitY && !immune) {
                const bool air=target->isAirborne();
                const bool blocked=target->isBlocking();
                p.hits=air ? 3 : p.hits+1;p.pulse=110;
                if(p.hits>=3) p.dying=120;
                const int owner=p.owner;const int part=p.hits;
                // All wave state is committed before emitting: QML may reset
                // the round synchronously in response to lethal damage.
                setLastHitContext(CharacterModel::DarkParticle,part);
                if(blocked) {
                    target->setPosXRatio(std::clamp(target->posXRatio()+p.direction*.015,0.0,3.0));
                    VoiceManager::instance().play(target->charId(),QStringLiteral("step"));
                }
                emit hitDetected(owner,blocked ? 2 : (air ? 24 : 8));
                emit darkParticlesChanged();
                return;
            }
        }
        if(p.age>2600 || p.x<-.2 || p.x>kStageWidth+.2) {m_darkParticles.removeAt(index);continue;}
        ++index;
    }
    if(m_darkParticles.isEmpty()) m_projectileTimer.stop();
    emit darkParticlesChanged();
}
