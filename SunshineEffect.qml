import QtQuick
import FightGame

// A separate original light-field layer fills every viewport size. The actor
// uses an independent transparent sheet, so resizing cannot stretch the body.
Item {
    required property var caster
    property real cameraOffset: 0
    property real fitScale: 1
    readonly property int frame: caster ? caster.currentFrame : 0
    visible: caster && caster.state === CharacterModel.Sunshine
             && frame >= 20 && frame <= 56
    clip: true

    Image {
        source: "qrc:/images/character/Orochi/SunshineBackground.png"
        width: parent.width * 8
        height: parent.height * 8
        x: -(parent.frame % 8) * parent.width
        y: -Math.floor(parent.frame / 8) * parent.height
        fillMode: Image.Stretch
        smooth: false
        mipmap: false
        asynchronous: true
        cache: true
    }
}
