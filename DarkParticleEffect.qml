import QtQuick

Item {
    id: effect
    required property var wave
    property real fitScale: 1
    property real cameraOffset: 0
    readonly property int frame: Math.floor(wave.age / 40) % 16
    width: 128 * 1.85 * fitScale
    height: 192 * 1.85 * fitScale
    x: parent.width * (wave.x - cameraOffset) - width / 2
    y: parent.height - (82 + wave.height) * fitScale - height / 2
    opacity: wave.dying > 0 ? wave.dying / 120 : 1

    Item {
        anchors.fill: parent
        clip: true
        transform: Scale {
            origin.x: effect.width / 2
            xScale: effect.wave.direction < 0 ? -1 : 1
        }
        Image {
            source: "qrc:/images/character/Orochi/DarkParticleFlight.png"
            width: parent.width * 8
            height: parent.height * 2
            x: -(effect.frame % 8) * parent.width
            y: -Math.floor(effect.frame / 8) * parent.height
            smooth: false
            cache: true
        }
    }
    Image {
        anchors.centerIn: parent
        width: 80 * effect.fitScale
        height: 80 * effect.fitScale
        visible: effect.wave.hits > 0 && effect.wave.pulse > 20
        source: "qrc:/images/Attack/" + (effect.wave.pulse > 75 ? "1" : effect.wave.pulse > 40 ? "2" : "3") + ".png"
        smooth: false
    }
}
