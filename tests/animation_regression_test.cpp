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

    void dodgeFeetUseTheSameGroundAsStanding() {
        CharacterModel model;
        auto cfg = character("Orochi");
        model.configure(cfg);
        model.playStand();
        double standingFeet = model.positionY() + cfg.stand.feetBottom;
        model.playDodge(true);
        QCOMPARE(model.positionY() + cfg.dodge.feetBottom, standingFeet);
    }

    void orochiGroundedSpritesNeverSinkBelowTheGround() {
        QFile script(QStringLiteral(FIGHTGAME_SOURCE_DIR "/FighterGeometry.js"));
        QVERIFY(script.open(QIODevice::ReadOnly));
        QString source = QString::fromUtf8(script.readAll());
        source.remove(".pragma library");
        QJSEngine js;
        QVERIFY(!js.evaluate(source).isError());
        auto spriteY = js.globalObject().property("spriteY");
        using Action = void (CharacterModel::*)();
        const QList<QPair<QString, Action>> actions = {
            {"lightPunch", &CharacterModel::playLightPunch},
            {"lightKick", &CharacterModel::playLightKick},
            {"heavyPunch", &CharacterModel::playHeavyPunch},
            {"heavyKick", &CharacterModel::playHeavyKick},
            {"heavyStrike", &CharacterModel::playHeavyStrike},
            {"hurt", &CharacterModel::playHurt},
            {"crouch", &CharacterModel::playCrouch},
            {"crouchAttack", &CharacterModel::playCrouchAttack},
            {"block", &CharacterModel::playStandBlock},
            {"dodge", nullptr}
        };
        int checked = 0;
        const auto cfg = character("Orochi");
        for (const auto &action : actions) {
            CharacterModel model;
            model.configure(cfg);
            model.playStand();
            if (action.second) (model.*action.second)(); else model.playDodge(true);
            const QString file = QStringLiteral(FIGHTGAME_SOURCE_DIR "/") + model.sourcePath().mid(5);
            QImage sheet(file);
            QVERIFY2(!sheet.isNull(), qPrintable(file));
            const int fw = model.frameWidth(), fh = model.frameHeight(), cols = model.totalFrames();
            QVERIFY(model.currentFrame() >= 0 && model.currentFrame() < cols);
            for (int frame = model.currentFrame(); frame < cols; ++frame) {
                QCOMPARE(model.currentFrame(), frame);
                int foot = 0;
                for (int y = fh - 1; y >= 0 && !foot; --y)
                    for (int x = 0; x < fw; ++x)
                        if (qAlpha(sheet.pixel((frame % model.sheetColumns())*fw+x,(frame / model.sheetColumns())*fh+y)) > 0) { foot=y+1;break; }
                if (foot) {
                    for (const QSize &size : QList<QSize>{{450,320},{640,360},{640,640},{900,640},{1280,720},{1920,1080}}) {
                        double scale = std::min(size.width()/900.0,size.height()/640.0);
                        model.updateRootHeight(size.height());
                        double y = spriteY.call({model.positionY(),double(size.height()),double(fh),scale}).toNumber();
                        double actualFoot = y + fh + (foot-fh)*scale*model.visualScale();
                        double ground = size.height() - cfg.stand.feetMargin*scale;
                        QVERIFY2(actualFoot <= ground + 0.001,
                            qPrintable(QString("%1 frame %2 at %3x%4 sinks %5 px").arg(action.first).arg(frame).arg(size.width()).arg(size.height()).arg(actualFoot-ground)));
                        ++checked;
                    }
                }
                if (frame+1 < cols) tick(model);
            }
        }
        QVERIFY(checked > 400);
        qInfo() << "Verified" << checked << "actual sprite-foot/window combinations";
    }

    void allActionsKeepTheirBaselineAcrossWindowSizes() {
        QFile script(QStringLiteral(FIGHTGAME_SOURCE_DIR "/FighterGeometry.js"));
        QVERIFY(script.open(QIODevice::ReadOnly));
        QString source = QString::fromUtf8(script.readAll());
        source.remove(".pragma library");
        QJSEngine js;
        QVERIFY(!js.evaluate(source).isError());
        auto spriteY = js.globalObject().property("spriteY");
        QVERIFY(spriteY.isCallable());
        using Action = void (CharacterModel::*)();
        const QList<QPair<QString, Action>> actions = {
            {"stand", &CharacterModel::playStand},
            {"lightPunch", &CharacterModel::playLightPunch},
            {"lightKick", &CharacterModel::playLightKick},
            {"heavyPunch", &CharacterModel::playHeavyPunch},
            {"heavyKick", &CharacterModel::playHeavyKick},
            {"heavyStrike", &CharacterModel::playHeavyStrike},
            {"hurt1", &CharacterModel::playHurt1},
            {"hurt2", &CharacterModel::playHurt2},
            {"hurt3", &CharacterModel::playHurt3},
            {"jump", &CharacterModel::playJump},
            {"block", &CharacterModel::playStandBlock}
        };
        const QList<QSize> sizes = {{450,320},{640,360},{640,640},{900,640},{1280,720},{1920,1080}};
        int checks = 0;
        for (const QString &name : {"Orochi", "Yagami", "Kusanagi"}) {
            for (const auto &action : actions) {
                CharacterModel model;
                model.configure(character(name));
                model.playStand();
                (model.*action.second)();
                if (model.totalFrames() > 2) tick(model);
                model.updateRootHeight(640);
                const double fh = model.frameHeight();
                const double visualScale = model.visualScale();
                // An authored point near the feet must follow the same transform
                // for every canvas height, sprite padding and visualScale.
                const double localFoot = fh * 0.9;
                const double reference = model.positionY() + fh + (localFoot - fh) * visualScale;
                for (const QSize &size : sizes) {
                    const double scale = std::min(size.width()/900.0, size.height()/640.0);
                    model.updateRootHeight(size.height());
                    const double y = spriteY.call({model.positionY(),double(size.height()),fh,scale}).toNumber();
                    const double renderedFoot = y + fh + (localFoot - fh)*scale*visualScale;
                    const double expected = size.height() + (reference - 640.0)*scale;
                    QVERIFY2(std::abs(renderedFoot - expected) < 0.0001,
                             qPrintable(QString("%1/%2 at %3x%4: %5 vs %6").arg(name,action.first).arg(size.width()).arg(size.height()).arg(renderedFoot).arg(expected)));
                    ++checks;
                }
            }
        }
        QCOMPARE(checks, 198);
    }

    void qqCrouchAttacksNeverMakeTheCharacterGiant() {
        for(const QString &name:{"Orochi","Yagami","Kusanagi"}) {
            const auto cfg=character(name);
            auto height=[&](const AnimParams &anim) {
                QImage im(QStringLiteral(FIGHTGAME_SOURCE_DIR "/")+anim.path.mid(5));
                const int cols=anim.sheetColumns>0?anim.sheetColumns:anim.cols;int tallest=0;
                for(int i=0;i<anim.cols;++i) {
                    int top=anim.fh,bottom=0;
                    for(int y=0;y<anim.fh;++y)for(int x=0;x<anim.fw;++x)
                        if(qAlpha(im.pixel((i%cols)*anim.fw+x,(i/cols)*anim.fh+y))>8) {top=std::min(top,y);bottom=std::max(bottom,y+1);}
                    tallest=std::max(tallest,bottom-top);
                }
                return tallest*anim.visualScale;
            };
            const double standing=height(cfg.stand);
            QVERIFY(height(cfg.crouchAttack)<=standing*1.25);
            QVERIFY(height(cfg.crouchKick)<=standing*1.25);
        }
    }

    void reviewedOpeningAppliesItsScaleAndGroundAnchor() {
        for(const QString &name:{"Orochi","Yagami","Kusanagi"}) {
            const auto cfg=character(name);CharacterModel model;model.configure(cfg);
            QCOMPARE(model.visualScale(),cfg.stand.visualScale);
            model.playOpening();QCOMPARE(model.visualScale(),cfg.opening.visualScale);
            if(cfg.opening.feetBottom>0) {
                const double feet=model.positionY()+model.frameHeight()
                    +(cfg.opening.feetBottom-model.frameHeight())*model.visualScale();
                QCOMPARE(feet,640.0-cfg.opening.feetMargin);
            }
            tick(model,model.totalFrames());
            for(int i=0;i<200 && model.state()==CharacterModel::Opening;++i)tick(model);
            QCOMPARE(model.state(),CharacterModel::Stand);
            QCOMPARE(model.visualScale(),cfg.stand.visualScale);
        }
    }

    void reviewedFlinchesHaveReadableHoldAndCompleteRecovery() {
        for(const QString &name:{"Orochi","Yagami","Kusanagi"}) {
            const auto cfg=character(name);const auto anim=cfg.hurt1.cols>0?cfg.hurt1:cfg.hurt;
            QVERIFY(anim.cols*anim.interval>=(name=="Kusanagi"?360:400));
            CharacterModel model;model.configure(cfg);model.playStand();model.playHurt1();
            QSignalSpy done(&model,&CharacterModel::hurtFinished);
            tick(model,2);QCOMPARE(model.state(),CharacterModel::Hurt);QCOMPARE(done.count(),0);
            tick(model,model.totalFrames()-2);QCOMPARE(model.state(),CharacterModel::Stand);QCOMPARE(done.count(),1);
        }
    }

};

QTEST_GUILESS_MAIN(AnimationRegressionTest)
#include "animation_regression_test.moc"
