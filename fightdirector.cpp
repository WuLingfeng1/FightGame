#include "fightdirector.h"
#include "charactermodel.h"
#include "characterconfig.h"
#include <QDebug>
#include <algorithm>
#include <cmath>

// 构造函数: 创建P1/P2角色模型, 连接开场完成信号, 加载JSON角色配置
FightDirector::FightDirector(QObject *parent)
    : QObject(parent)
{
    m_p1Model = new CharacterModel(this);
    m_p2Model = new CharacterModel(this);

    connect(m_p1Model, &CharacterModel::openingFinished,
            this, &FightDirector::onP1OpeningFinished);
    connect(m_p2Model, &CharacterModel::openingFinished,
            this, &FightDirector::onP2OpeningFinished);

    QFile file("/wlf/FightGame/config/characters.json");
    if (file.open(QIODevice::ReadOnly)) {
        m_jsonConfig = QJsonDocument::fromJson(file.readAll()).object();
        file.close();
    } else {
        qWarning() << "[FightDirector] Cannot load characters.json from: /wlf/FightGame/config/characters.json";
    }
}

FightDirector::~FightDirector()
{
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
    qDebug() << "[FightDirector] Starting fight:" << p1CharId << "vs" << p2CharId;

    loadConfig(p1CharId, m_p1Model);
    loadConfig(p2CharId, m_p2Model);

    m_p1Model->setOpponent(m_p2Model);
    m_p2Model->setOpponent(m_p1Model);

    m_p1Model->m_rootHeight = m_rootHeight;
    m_p2Model->m_rootHeight = m_rootHeight;

    m_phase = Opening_P1;
    emit phaseChanged();

    m_p1Model->playOpening();   // 先播放P1的开场
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

// 格斗运镜: 始终确保两角色在屏内, 容不下时回退中点跟随
// 小幅抖动通过平滑插值消除, 大幅移动直接跟随
void FightDirector::updateCamera()
{
    if (!m_p1Model || !m_p2Model) return;

    double p1 = m_p1Model->posXRatio();
    double p2 = m_p2Model->posXRatio();
    double maxPos = std::max(p1, p2);
    double minPos = std::min(p1, p2);

    double target = (p1 + p2) / 2.0 - 0.5;

    double keepRight = maxPos - 0.95;
    double keepLeft  = minPos - 0.05;

    if (keepLeft >= keepRight) {
        target = std::max(target, keepRight);
        target = std::min(target, keepLeft);
    }

    target = std::max(0.0, std::min(target, kStageWidth - 1.0));

    double diff = target - m_cameraOffset;
    double factor = (std::abs(diff) < 0.015) ? 0.2 : 0.7;
    m_cameraOffset += diff * factor;

    emit cameraOffsetChanged();
}

// 碰撞检测: 检查攻击框是否命中对手受击框
bool FightDirector::checkCollision()
{
    // 窗口尺寸
    const double windowW = 900.0;
    const double windowH = 640.0;
    bool anyHit = false;

    // 检查 P1 是否攻击 P2
    if (m_p1Model->isAttacking() && m_p1Model->isAttackActive()
        && m_p2Model->state() != CharacterModel::Dodge) {
        double hitNormX = m_p1Model->hitboxX();
        double hitY = m_p1Model->hitboxY();
        double hitR = m_p1Model->hitboxRadius();

        double hurtNormX = m_p2Model->hurtboxX();
        double hurtY = m_p2Model->hurtboxY();
        double hurtW = m_p2Model->hurtboxW();
        double hurtH = m_p2Model->hurtboxH();

        double hitPx = hitNormX * windowW;
        double hurtPx = hurtNormX * windowW;

        bool hitX = (hitPx + hitR >= hurtPx - hurtW/2) && (hitPx - hitR <= hurtPx + hurtW/2);
        bool hitYRange = (hitY + hitR >= hurtY - hurtH/2) && (hitY - hitR <= hurtY + hurtH/2);

        if (hitX && hitYRange) {
            m_p1Model->m_hitThisAttack = true;
            m_p2Model->m_knockbackToApply = m_p1Model->m_currentAnim.knockbackDistance;
            emit hitDetected(1, 10);
            anyHit = true;
        }
    }

    // 检查 P2 是否攻击 P1
    if (m_p2Model->isAttacking() && m_p2Model->isAttackActive()
        && m_p1Model->state() != CharacterModel::Dodge) {
        double hitNormX = m_p2Model->hitboxX();
        double hitY = m_p2Model->hitboxY();
        double hitR = m_p2Model->hitboxRadius();

        double hurtNormX = m_p1Model->hurtboxX();
        double hurtY = m_p1Model->hurtboxY();
        double hurtW = m_p1Model->hurtboxW();
        double hurtH = m_p1Model->hurtboxH();

        double hitPx = hitNormX * windowW;
        double hurtPx = hurtNormX * windowW;

        bool hitX = (hitPx + hitR >= hurtPx - hurtW/2) && (hitPx - hitR <= hurtPx + hurtW/2);
        bool hitYRange = (hitY + hitR >= hurtY - hurtH/2) && (hitY - hitR <= hurtY + hurtH/2);

        if (hitX && hitYRange) {
            m_p2Model->m_hitThisAttack = true;
            m_p1Model->m_knockbackToApply = m_p2Model->m_currentAnim.knockbackDistance;
            emit hitDetected(2, 10);
            anyHit = true;
        }
    }

    return anyHit;
}
