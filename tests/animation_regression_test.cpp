#include "charactermodel.h"
#include "characterconfig.h"
#include "fightdirector.h"
#include "keybindingconfig.h"
#include "voicemanager.h"
#include <QtTest>
#include <QFile>
#include <QJsonDocument>
#include <QJSEngine>
#include <QImage>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <memory>
#include <cmath>

class AnimationRegressionTest : public QObject
{
    Q_OBJECT
    QJsonObject characters;
    CharacterData character(const QString &name) {
        return CharacterConfig::load(characters.value(name).toObject(), name);
    }
    void tick(CharacterModel &model, int count = 1) {
        for (int i = 0; i < count; ++i) {
            if(model.state()==CharacterModel::Jump || model.state()==CharacterModel::DiagonalJump
               || model.state()==CharacterModel::AirPunch || model.state()==CharacterModel::AirKick)
                model.advanceJump(model.animationInterval());
            else if(model.hasKnockdownMotion())model.advanceKnockdown(model.animationInterval());
            else QVERIFY(QMetaObject::invokeMethod(&model, "onTick", Qt::DirectConnection));
        }
    }
private slots:
    void initTestCase() {
        QFile file(QStringLiteral(FIGHTGAME_SOURCE_DIR "/config/characters.json"));
        QVERIFY(file.open(QIODevice::ReadOnly));
        characters = QJsonDocument::fromJson(file.readAll()).object();
        QVERIFY(characters.contains("Orochi"));
        VoiceManager::instance().setEnabled(false);
    }

    void orochiDodgePublishesOffsetsAndFacing() {
        CharacterModel model, opponent;
        model.configure(character("Orochi"));
        opponent.configure(character("Yagami"));
        model.setOpponent(&opponent);
        model.setPosXRatio(0.4);
        opponent.setPosXRatio(0.55);
        model.setFacingLeft(false);
        model.playStand();
        QQmlEngine engine;
        engine.rootContext()->setContextProperty("fighter", &model);
        QQmlComponent component(&engine);
        component.setData("import QtQml\nQtObject { property int offset: fighter.animOffsetX; property bool left: fighter.facingLeft }", QUrl());
        std::unique_ptr<QObject> binding(component.create());
        QVERIFY2(binding != nullptr, qPrintable(component.errorString()));
        QSignalSpy finished(&model, &CharacterModel::dodgeFinished);
        model.playDodge(true);
        QCOMPARE(binding->property("offset").toInt(), 0);
        tick(model, 8);
        QCOMPARE(model.currentFrame(), 8);
        QCOMPARE(binding->property("offset").toInt(), 0);
        QVERIFY(binding->property("left").toBool());
        QVERIFY(std::abs(model.posXRatio() - 0.7) < 0.00001);
        tick(model, 5);
        QCOMPARE(binding->property("offset").toInt(), 0);
        tick(model);
        QCOMPARE(binding->property("offset").toInt(), 0);
        tick(model);
        QCOMPARE(binding->property("offset").toInt(), 0);
        tick(model);
        QCOMPARE(model.state(), CharacterModel::Stand);
        QCOMPARE(binding->property("offset").toInt(), 0);
        QCOMPARE(finished.count(), 1);
        model.playLightPunch();
        QCoreApplication::processEvents();
        QCOMPARE(model.state(), CharacterModel::LightPunch);
    }

    void repeatedDodgeDoesNotRestartAnimation() {
        CharacterModel model;
        model.configure(character("Orochi"));
        model.playStand();
        model.playDodge(true);
        tick(model, 5);
        model.playDodge(false);
        QCOMPARE(model.currentFrame(), 5);
        QCOMPARE(model.state(), CharacterModel::Dodge);
    }

};

QTEST_GUILESS_MAIN(AnimationRegressionTest)
#include "animation_regression_test.moc"
