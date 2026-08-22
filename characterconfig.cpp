#include "characterconfig.h"
#include <QJsonArray>

// 从 JSON 对象解析单个动画参数, 空对象返回默认值
static AnimParams parseAnim(const QJsonObject &obj)
{
    AnimParams a;
    if (obj.isEmpty()) return a;
    a.cols = obj.value("cols").toInt(0);
    a.fw = obj.value("fw").toInt(0);
    a.fh = obj.value("fh").toInt(0);
    a.interval = obj.value("interval").toInt(50);
    a.path = obj.value("path").toString();
    a.loop = obj.value("loop").toBool(true);
    a.pauseFrame = obj.value("pauseFrame").toInt(-1);
    a.pauseDuration = obj.value("pauseDuration").toInt(0);
    a.feetBottom = obj.value("feetBottom").toInt(0);
    a.feetMargin = obj.value("feetMargin").toInt(0);
    a.jumpHeight = obj.value("jumpHeight").toInt(200);
    a.divFrame = obj.value("divFrame").toInt(0);
    a.jumpDistance = obj.value("jumpDistance").toDouble(0.15);
    a.offsetXFwd = obj.value("offsetXFwd").toInt(0);
    a.offsetXLast = obj.value("offsetXLast").toInt(0);
    a.offsetXBwd = obj.value("offsetXBwd").toInt(0);
    a.offsetXBwdLast = obj.value("offsetXBwdLast").toInt(0);
    a.visualScale = obj.value("visualScale").toDouble(1.0);
    a.offsetX = obj.value("offsetX").toInt(0);
    a.attackPointX = obj.value("attackPointX").toDouble(0.0);
    a.attackPointY = obj.value("attackPointY").toInt(0);
    a.attackRadius = obj.value("attackRadius").toInt(60);
    a.attackFrames = obj.value("attackFrames").toString();
    a.knockbackDistance = obj.value("knockbackDistance").toInt(0);
    a.damage = obj.value("damage").toInt(0);
    a.dodgeDistance = obj.value("dodgeDistance").toDouble(0);
    a.dodgeFrames = obj.value("dodgeFrames").toString();
    a.dodgeBackFrames = obj.value("dodgeBackFrames").toString();
    a.dodgeStartFrame = obj.value("dodgeStartFrame").toInt(0);
    a.dodgeEndFrame = obj.value("dodgeEndFrame").toInt(0);
    a.dodgeSwitchFrame = obj.value("dodgeSwitchFrame").toInt(0);
    a.offsetXPost = obj.value("offsetXPost").toInt(0);
    a.offsetXEnd = obj.value("offsetXEnd").toInt(0);
    a.blockHoldFrame = obj.value("blockHoldFrame").toInt(0);
    a.crouchHoldFrame = obj.value("crouchHoldFrame").toInt(0);
    a.heavyKickJumpHeight = obj.value("heavyKickJumpHeight").toInt(0);
    a.flyHeight = obj.value("flyHeight").toInt(0);
    a.flyPeakT = obj.value("flyPeakT").toDouble(0.2);
    a.flyLandT = obj.value("flyLandT").toDouble(0.4);
    a.flyDistance = obj.value("flyDistance").toDouble(0);
    a.riseFollowUp = obj.value("riseFollowUp").toInt(0);
    a.openingVoiceMs = obj.value("openingVoiceMs").toInt(0);
    return a;
}

// 从 JSON 对象加载完整角色配置: 基础属性 + 三个动画段
CharacterData CharacterConfig::load(const QJsonObject &json, const QString &id)
{
    CharacterData cfg;
    cfg.id = id;
    cfg.facingLeft = json.value("facingLeft").toBool(false);
    cfg.posX = json.value("posX").toDouble(0.5);
    cfg.opening = parseAnim(json.value("opening").toObject());
    cfg.stand = parseAnim(json.value("stand").toObject());
    cfg.forward = parseAnim(json.value("forward").toObject());
    cfg.backward = parseAnim(json.value("backward").toObject());
    cfg.jump = parseAnim(json.value("jump").toObject());
    cfg.diagonalJump = parseAnim(json.value("diagonalJump").toObject());
    cfg.lightPunch = parseAnim(json.value("lightPunch").toObject());
    cfg.lightKick = parseAnim(json.value("lightKick").toObject());
    cfg.heavyPunch = parseAnim(json.value("heavyPunch").toObject());
    cfg.heavyKick = parseAnim(json.value("heavyKick").toObject());
    cfg.heavyStrike = parseAnim(json.value("heavyStrike").toObject());
    cfg.hurt = parseAnim(json.value("hurt").toObject());
    cfg.hurt1 = parseAnim(json.value("hurt1").toObject());
    cfg.hurt2 = parseAnim(json.value("hurt2").toObject());
    cfg.hurt3 = parseAnim(json.value("hurt3").toObject());
    cfg.rise = parseAnim(json.value("rise").toObject());
    cfg.crouch = parseAnim(json.value("crouch").toObject());
    cfg.crouchAttack = parseAnim(json.value("crouchAttack").toObject());
    cfg.dodge = parseAnim(json.value("dodge").toObject());
    cfg.standBlock = parseAnim(json.value("standBlock").toObject());
    cfg.win = parseAnim(json.value("win").toObject());
    cfg.lose = parseAnim(json.value("lose").toObject());
    return cfg;
}
