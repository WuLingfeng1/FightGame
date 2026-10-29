import QtQuick

Item {
    id: effect
    required property var defender
    required property var caster
    property real cameraOffset: 0
    property real fitScale: 1
    visible: defender && defender.sunshineGuarded
    readonly property real pulse: caster ? (caster.currentFrame % 4) / 4 : 0
    Canvas {
        id: shield
        width: 76 * effect.fitScale
        height: 108 * effect.fitScale
        x: parent.width * (effect.defender.posXRatio - effect.cameraOffset)
           + (effect.defender.facingLeft ? -65 : -11) * effect.fitScale
        y: parent.height - 290 * effect.fitScale
        opacity: 0.5 + effect.pulse * 0.5
        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.scale(width / 76, height / 108)
            ctx.strokeStyle = "#238bd1"
            ctx.lineWidth = 5
            ctx.beginPath()
            ctx.ellipse(10, 12, 56, 84)
            ctx.stroke()
            ctx.strokeStyle = "#d6faff"
            ctx.lineWidth = 2
            ctx.beginPath()
            ctx.ellipse(14, 15, 48, 78)
            ctx.stroke()
            ctx.fillStyle = "#efffff"
            ctx.fillRect(35, 0, 4, 15)
            ctx.fillRect(35, 94, 4, 14)
        }
    }
}
