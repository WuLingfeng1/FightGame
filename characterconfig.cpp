#include "characterconfig.h"
#include <QJsonArray>

// 从 JSON 对象解析单个动画参数, 空对象返回默认值
static AnimParams parseAnim(const QJsonObject &obj)
{
    AnimParams a;
    if (obj.isEmpty()) return a;
    a.cols     = obj.value("cols").toInt(0);
    a.fw       = obj.value("fw").toInt(0);
    a.fh       = obj.value("fh").toInt(0);
    a.interval = obj.value("interval").toInt(50);
    a.path     = obj.value("path").toString();
    a.loop     = obj.value("loop").toBool(true);
    a.pauseFrame   = obj.value("pauseFrame").toInt(-1);
    a.pauseDuration = obj.value("pauseDuration").toInt(0);
    a.feetBottom = obj.value("feetBottom").toInt(0);
    a.feetMargin = obj.value("feetMargin").toInt(0);
    a.jumpHeight = obj.value("jumpHeight").toInt(200);
    return a;
}

// 从 JSON 对象加载完整角色配置: 基础属性 + 三个动画段
CharacterConfig CharacterConfigLoader::load(const QJsonObject &json, const QString &id)
{
    CharacterConfig cfg;
    cfg.id = id;
    cfg.facingLeft = json.value("facingLeft").toBool(false);
    cfg.posX       = json.value("posX").toDouble(0.5);
    cfg.opening    = parseAnim(json.value("opening").toObject());
    cfg.stand      = parseAnim(json.value("stand").toObject());
    cfg.forward    = parseAnim(json.value("forward").toObject());
    cfg.backward   = parseAnim(json.value("backward").toObject());
    cfg.jump       = parseAnim(json.value("jump").toObject());
    return cfg;
}
