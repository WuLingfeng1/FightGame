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

    void sunshineAvailabilityCooldownAndAtlas() {
        CharacterModel model;
        model.configure(character("Yagami"));
        model.playStand();
        QVERIFY(!model.hasSunshine());
        QVERIFY(!model.playSunshine());
        model.configure(character("Orochi"));
        model.playStand();
        QVERIFY(model.hasSunshine());
        QVERIFY(model.playSunshine());
        QCOMPARE(model.state(), CharacterModel::Sunshine);
        QCOMPARE(model.sheetColumns(), 8);
        QCOMPARE(model.totalFrames(), 64);
        QCOMPARE(model.sunshineCooldown(), 8);
        tick(model, 4);
        QVERIFY(!model.playSunshine());
        QCOMPARE(model.currentFrame(), 4);
        QSignalSpy finished(&model, &CharacterModel::attackFinished);
        for (int frame=4; frame<64; ++frame) {
            QCOMPARE(model.isAttackActive(), frame >= 28 && frame <= 49);
            tick(model);
        }
        QCOMPARE(model.state(), CharacterModel::Stand);
        QCOMPARE(finished.count(), 1);
        QCOMPARE(model.sheetColumns(), model.totalFrames());
        QVERIFY(!model.playSunshine());
        QSignalSpy cooldown(&model, &CharacterModel::sunshineCooldownChanged);
        QTest::qWait(1100);
        QVERIFY(model.sunshineCooldown() < 8);
        QVERIFY(cooldown.count() >= 1);
        model.configure(character("Orochi"));
        model.playStand();
        QCOMPARE(model.sunshineCooldown(), 0);
        QVERIFY(model.playSunshine());
    }

    void sunshineCannotStartDuringJumpHurtOrDodge() {
        CharacterModel model;
        const auto cfg=character("Orochi");
        model.configure(cfg);model.playStand();model.playJump();
        QVERIFY(!model.playSunshine());
        model.configure(cfg);model.playStand();model.playHurt();
        QVERIFY(!model.playSunshine());
        model.configure(cfg);model.playStand();model.playDodge(true);
        QVERIFY(!model.playSunshine());
        QCOMPARE(model.sunshineCooldown(),0);
    }

    void sunshineHasSixDistinctGuardableDamageStages() {
        for(bool guard:{false,true})for(bool reverse:{false,true}) {
            FightDirector d;d.resetForNewRound(reverse?"Yagami":"Orochi",reverse?"Orochi":"Yagami");
            auto a=reverse?d.p2Model():d.p1Model();auto b=reverse?d.p1Model():d.p2Model();
            a->setPosXRatio(.02);b->setPosXRatio(.98);if(guard)b->playStandBlock();
            QSignalSpy hits(&d,&FightDirector::hitDetected);
            connect(&d,&FightDirector::hitDetected,b,[&](int,int){if(guard)b->playSunshineBlock();else b->playSunshineHurt();});
            QVERIFY(a->playSunshine());int total=0,index=0;
            const QList<int> frames={28,32,36,40,44,49};
            for(int frame=0;frame<64;++frame) {
                const bool expected=frames.contains(frame);
                QCOMPARE(d.checkCollision(),expected);
                if(expected) {
                    QCOMPARE(hits.count(),++index);QCOMPARE(d.lastHitPart(),index);
                    QCOMPARE(hits.last()[1].toInt(),guard?3:6);total+=hits.last()[1].toInt();
                }
                QVERIFY(!d.checkCollision()); // No duplicate damage within a stage.
                tick(*b);tick(*a);
            }
            QCOMPARE(index,6);QCOMPARE(total,guard?18:36);
        }
    }

    void sunshineRespectsDodgeInvulnerability() {
        FightDirector director;
        director.resetForNewRound("Orochi","Yagami");
        auto attacker=director.p1Model();auto defender=director.p2Model();
        QVERIFY(attacker->playSunshine());
        tick(*attacker,28);defender->playDodge(true);
        QVERIFY(!director.checkCollision());
        defender->playStand();
        QVERIFY(director.checkCollision());
    }

    void sunshineReactionRecoversForEveryCharacter() {
        for (const QString &name : {"Orochi","Yagami","Kusanagi"}) {
            CharacterModel defender,attacker;
            defender.configure(character(name));attacker.configure(character("Orochi"));
            defender.setOpponent(&attacker);attacker.setPosXRatio(0.2);defender.setPosXRatio(0.8);
            defender.playStand();defender.playCrouch();
            defender.playSunshineHurt();
            QCOMPARE(defender.state(),CharacterModel::Hurt);
            const double groundY=defender.positionY();
            const double startX=defender.posXRatio();
            for (int i=0;i<30;++i) {
                tick(defender);
                QCOMPARE(defender.positionY(),groundY);
                QVERIFY(defender.posXRatio()>=startX);
                QVERIFY(defender.posXRatio()-startX<=0.04001);
                QCOMPARE(defender.state(),CharacterModel::Hurt);
            }
            tick(defender,7); // 1110 ms: release into the final launch.
            const double launchY=defender.positionY();
            tick(defender);
            QVERIFY(defender.positionY()<launchY);
            int frames=0;
            while (defender.state()!=CharacterModel::Stand && frames++<100) tick(defender);
            QVERIFY2(defender.state()==CharacterModel::Stand,qPrintable(name));
            QVERIFY(defender.posXRatio()>0.8);
            defender.playForward();
            QCOMPARE(defender.state(),CharacterModel::Forward);
        }
    }

    void sunshineBindingsAndTextureLimits() {
        KeyBindingConfig keys;
        QCOMPARE(keys.getBinding("P1","sunshine"),QString("O"));
        QCOMPARE(keys.getBinding("P2","sunshine"),QString("NumPad6"));
        QImage atlas(QStringLiteral(FIGHTGAME_SOURCE_DIR "/images/character/Orochi/SunshineAtlas.png"));
        QVERIFY(!atlas.isNull());
        QCOMPARE(atlas.size(),QSize(3072,4096));
        QVERIFY(atlas.width()<=4096 && atlas.height()<=4096);
        QImage original(QStringLiteral(FIGHTGAME_SOURCE_DIR "/images/character/Orochi/SunShine.png"));
        QVERIFY(!original.isNull());
        for (int frame=0;frame<64;++frame) {
            const QImage source = original.copy(frame*384,0,384,512);
            const QImage packed = atlas.copy((frame%8)*384,(frame/8)*512,384,512);
            QVERIFY2(source==packed,qPrintable(QString("Original frame %1 was changed or skipped").arg(frame)));
        }
    }

    void sunshineLaunchWaitsForTheCasterFinalFlash() {
        for (const QString &name : {"Orochi","Yagami","Kusanagi"}) {
            CharacterModel attacker,defender;
            attacker.configure(character("Orochi"));defender.configure(character(name));
            attacker.setOpponent(&defender);defender.setOpponent(&attacker);
            attacker.setPosXRatio(.2);defender.setPosXRatio(.7);
            attacker.playStand();defender.playStand();
            QVERIFY(attacker.playSunshine());tick(attacker,28);
            defender.playSunshineHurt();
            const double groundY=defender.positionY();
            const QString recoilSheet=defender.sourcePath();
            // Faster/slower clocks cannot launch the victim before frame 50.
            tick(defender,60);
            QCOMPARE(defender.positionY(),groundY);
            QCOMPARE(defender.sourcePath(),recoilSheet);
            QVERIFY(defender.posXRatio()<=.74001);
            tick(attacker,21);tick(defender);
            QCOMPARE(attacker.currentFrame(),49);
            QCOMPARE(defender.sourcePath(),recoilSheet);
            tick(attacker);tick(defender);
            QCOMPARE(attacker.currentFrame(),50);
            const double launchY=defender.positionY();
            tick(defender);QVERIFY(defender.positionY()<launchY);
        }
    }

    void interruptedSunshineRecoilDoesNotLeakIntoNextRound() {
        CharacterModel model;
        model.configure(character("Yagami"));model.playStand();
        model.playSunshineHurt();tick(model,10);
        model.playStand();tick(model,50);
        QCOMPARE(model.state(),CharacterModel::Stand);
        model.playSunshineHurt();tick(model,10);
        model.configure(character("Orochi"));model.playStand();tick(model,50);
        QCOMPARE(model.state(),CharacterModel::Stand);
    }

    void dodgeHasOneBriefInvisibleFrameAndMatchingIdleEnds() {
        const auto cfg=character("Orochi");
        QCOMPARE(cfg.dodge.interval,35);
        QCOMPARE(cfg.dodge.sheetColumns,8);
        QImage atlas(QStringLiteral(FIGHTGAME_SOURCE_DIR "/images/character/Orochi/DodgeSmoothAtlas.png"));
        QCOMPARE(atlas.size(),QSize(3072,1024));
        auto tile=[&](int i) { return atlas.copy((i%8)*384,(i/8)*512,384,512); };
        QCOMPARE(tile(0),tile(15));
        int invisible=0;
        for (int i=0;i<16;++i) {
            const auto f=tile(i);
            bool visible=false;
            for (int y=0;y<f.height() && !visible;++y)
                for (int x=0;x<f.width();++x)
                    if(qAlpha(f.pixel(x,y))) { visible=true;break; }
            if(!visible) { ++invisible;QCOMPARE(i,8); }
        }
        QCOMPARE(invisible,1);
        CharacterModel model;
        model.configure(cfg);model.playStand();model.playLightPunch();
        model.playDodge(true);QCOMPARE(model.state(),CharacterModel::LightPunch);
        model.playStand();model.playCrouch();model.playDodge(true);
        QCOMPARE(model.state(),CharacterModel::Crouch);
    }

    void crouchLoopsAndHoldsRecoverForAllCharacters() {
        for (const QString &name : {"Orochi","Yagami","Kusanagi"}) {
            CharacterModel model;const auto cfg=character(name);
            model.configure(cfg);model.playStand();model.playCrouch();
            tick(model,cfg.crouch.cols*3+2);
            QCOMPARE(model.state(),CharacterModel::Crouch);
            QVERIFY(model.currentFrame()<model.totalFrames());
            model.stopCrouch();
            for(int i=0;i<20 && model.state()!=CharacterModel::Stand;++i)tick(model);
            QCOMPARE(model.state(),CharacterModel::Stand);
            model.playForward();QCOMPARE(model.state(),CharacterModel::Forward);
        }
    }

    void crouchAttackReturnsToHeldPoseAndCanBeRepeated() {
        CharacterModel model;const auto cfg=character("Orochi");
        model.configure(cfg);model.playStand();model.playCrouch();
        QSignalSpy finished(&model,&CharacterModel::attackFinished);
        for(int attempt=0;attempt<3;++attempt) {
            model.playCrouchAttack();tick(model,cfg.crouchAttack.cols);
            QCOMPARE(model.state(),CharacterModel::Crouch);
            QCOMPARE(model.currentFrame(),0);
        }
        QCOMPARE(finished.count(),3);
        model.playCrouchAttack();tick(model,2);model.stopCrouch();
        QCOMPARE(model.state(),CharacterModel::CrouchAttack);
        tick(model,cfg.crouchAttack.cols-2);
        QCOMPARE(model.state(),CharacterModel::Stand);
        QCOMPARE(finished.count(),4);
    }

    void fullHitsBreakCrouchAndRecoverForEveryCharacter() {
        for (const QString &name : {"Orochi","Yagami","Kusanagi"}) {
            CharacterModel model;model.configure(character(name));model.playStand();model.playCrouch();
            model.playHurt1();QCOMPARE(model.state(),CharacterModel::Hurt);
            tick(model,model.totalFrames());QCOMPARE(model.state(),CharacterModel::Stand);
            model.playLightPunch();QCOMPARE(model.state(),CharacterModel::LightPunch);
        }
    }

    void riseOnlyPublishesRecoveryAfterTheGetUpFinishes() {
        CharacterModel model;model.configure(character("Yagami"));model.playStand();
        QSignalSpy recovered(&model,&CharacterModel::hurtFinished);
        model.playHurt3();tick(model,model.totalFrames());
        QCOMPARE(model.state(),CharacterModel::Rise);QCOMPARE(recovered.count(),0);
        tick(model,model.totalFrames());
        QCOMPARE(model.state(),CharacterModel::Stand);QCOMPARE(recovered.count(),1);
        model.playForward();QCOMPARE(model.state(),CharacterModel::Forward);
        model.playHurt3();model.setStayDown(true);tick(model,model.totalFrames());
        QCOMPARE(model.state(),CharacterModel::Hurt);
    }

    void jumpAndWalkPublishStateAndDoNotInheritAttackOffsets() {
        for(const QString &name : {"Orochi","Yagami","Kusanagi"}) {
            CharacterModel model;model.configure(character(name));model.playStand();
            QSignalSpy states(&model,&CharacterModel::stateChanged);
            model.playForward();QCOMPARE(states.count(),1);
            model.playBackward();QCOMPARE(states.count(),2);
            model.playJump();QCOMPARE(states.count(),3);
            QCOMPARE(model.animOffsetX(),character(name).jump.offsetX);
            model.playStand();model.playDiagonalJump(true);
            const auto y=model.positionY();model.updateRootHeight(640);
            QCOMPARE(model.positionY(),y); // First render uses the final scale.
        }
    }

    void interruptedHopDoesNotLiftTheHurtOrIdlePose() {
        CharacterModel model;model.configure(character("Kusanagi"));model.playStand();
        model.playHeavyKick();tick(model,2);model.playHurt1();
        const double y=model.positionY();tick(model,2);QCOMPARE(model.positionY(),y);
        tick(model,model.totalFrames());QCOMPARE(model.state(),CharacterModel::Stand);
    }

    void blockStartsAtTheFirstPoseAndRecoversOnRelease() {
        for(const QString &name : {"Orochi","Yagami","Kusanagi"}) {
            CharacterModel model;model.configure(character(name));model.playStand();model.playStandBlock();
            QCOMPARE(model.currentFrame(),0);
            tick(model,model.totalFrames()+2);QCOMPARE(model.state(),CharacterModel::StandBlock);
            model.releaseStandBlock();
            for(int i=0;i<20 && model.state()!=CharacterModel::Stand;++i)tick(model);
            QCOMPARE(model.state(),CharacterModel::Stand);
        }
    }

    void preloadListsAreUniqueAndPointAtExistingAssets() {
        for(const QString &name : {"Orochi","Yagami","Kusanagi"}) {
            CharacterModel model;model.configure(character(name));
            const auto paths=model.preloadSources();QVERIFY(paths.size()>=10);
            auto unique=paths;unique.removeDuplicates();QCOMPARE(unique,paths);
            for(const auto &path:paths) QVERIFY(QFile::exists(QStringLiteral(FIGHTGAME_SOURCE_DIR "/")+path.mid(5)));
        }
    }

    void releasedGuardAndCrouchSignalThatMovementCanResume() {
        for(const QString &name : {"Orochi","Yagami","Kusanagi"}) {
            CharacterModel model;model.configure(character(name));model.playStand();
            QSignalSpy ready(&model,&CharacterModel::controlRecovered);
            model.playStandBlock();tick(model,model.totalFrames()+2);
            model.releaseStandBlock();
            for(int i=0;i<20 && model.state()!=CharacterModel::Stand;++i)tick(model);
            QCOMPARE(ready.count(),1);
            model.playCrouch();tick(model,model.totalFrames()+2);model.stopCrouch();
            for(int i=0;i<20 && model.state()!=CharacterModel::Stand;++i)tick(model);
            QCOMPARE(ready.count(),2);
        }
    }

    void darkParticleOnlyOrochiCanCastAndReleaseOccursOnce() {
        for(const QString &name : {"Yagami","Kusanagi"}) {
            CharacterModel model;model.configure(character(name));model.playStand();
            QVERIFY(!model.hasDarkParticle());QVERIFY(!model.playDarkParticle());
        }
        CharacterModel model;model.configure(character("Orochi"));model.playStand();
        QSignalSpy release(&model,&CharacterModel::darkParticleReleased);
        QSignalSpy finish(&model,&CharacterModel::attackFinished);
        QVERIFY(model.playDarkParticle());QVERIFY(!model.playDarkParticle());
        QVERIFY(model.isAttacking());QVERIFY(!model.isAttackActive());
        tick(model,13);QCOMPARE(release.count(),0);
        tick(model);QCOMPARE(release.count(),1);
        tick(model,20);QCOMPARE(model.state(),CharacterModel::Stand);
        QCOMPARE(release.count(),1);QCOMPARE(finish.count(),1);
        model.playJump();QVERIFY(!model.playDarkParticle());
    }

    void darkParticleTravelsAndStillHitsAfterCasterRecovers() {
        for(bool p1 : {true,false}) {
            FightDirector director;director.resetForNewRound(p1?"Orochi":"Yagami",p1?"Yagami":"Orochi");
            auto caster=p1?director.p1Model():director.p2Model();
            auto target=p1?director.p2Model():director.p1Model();
            caster->setPosXRatio(p1?.2:.8);target->setPosXRatio(p1?.8:.2);
            caster->setFacingLeft(!p1);
            QSignalSpy hits(&director,&FightDirector::hitDetected);
            connect(&director,&FightDirector::hitDetected,target,[&](int,int) {
                QCOMPARE(director.lastHitState(),int(CharacterModel::DarkParticle));
                target->playDarkParticleHurt(director.lastHitPart());
            });
            QVERIFY(caster->playDarkParticle());tick(*caster,14);
            QCOMPARE(director.darkParticles().size(),1);
            for(int i=0;i<15;++i)director.advanceDarkParticles(16);
            QCOMPARE(hits.count(),0); // No instant full-screen/body-box hit.
            tick(*caster,20);QCOMPARE(caster->state(),CharacterModel::Stand);
            for(int i=0;i<200;++i)director.advanceDarkParticles(16);
            QCOMPARE(hits.count(),3);
            int damage=0;for(const auto &hit:hits) {QCOMPARE(hit[0].toInt(),p1?1:2);damage+=hit[1].toInt();}
            QCOMPARE(damage,24);QVERIFY(director.darkParticles().isEmpty());
            QCOMPARE(target->state(),CharacterModel::Hurt);
            tick(*target,target->totalFrames());QCOMPARE(target->state(),CharacterModel::Stand);
        }
    }

    void darkParticleGuardChipsAndPushesButDodgePassesThrough() {
        for(bool guard : {true,false}) {
            FightDirector director;director.resetForNewRound("Orochi","Yagami");
            auto a=director.p1Model();auto b=director.p2Model();
            if(guard)b->playStandBlock();else b->playDodge(true);
            const auto start=b->posXRatio();QSignalSpy hits(&director,&FightDirector::hitDetected);
            QVERIFY(a->playDarkParticle());tick(*a,14);
            for(int i=0;i<200;++i)director.advanceDarkParticles(16);
            QCOMPARE(hits.count(),guard?3:0);
            if(guard) {for(const auto &hit:hits)QCOMPARE(hit[1].toInt(),2);QVERIFY(b->posXRatio()>start);QCOMPARE(b->state(),CharacterModel::StandBlock);}
            QVERIFY(director.darkParticles().isEmpty());
        }
    }

    void darkParticleMissesHighJumpsAndAirHitKeepsTheCurrentHeight() {
        FightDirector director;director.resetForNewRound("Orochi","Yagami");
        auto a=director.p1Model();auto b=director.p2Model();
        QVERIFY(b->playRun());b->playJump();tick(*b,b->totalFrames()/2);
        QSignalSpy hits(&director,&FightDirector::hitDetected);
        QVERIFY(a->playDarkParticle());tick(*a,14);
        for(int i=0;i<200;++i)director.advanceDarkParticles(16);
        QCOMPARE(hits.count(),0);
        for(const QString &name : {"Orochi","Yagami","Kusanagi"}) {
            CharacterModel target;const auto cfg=character(name);target.configure(cfg);target.playStand();target.playJump();tick(target);
            auto feet=[&]() {double fb=target.state()==CharacterModel::Jump?(cfg.jump.feetBottom>0?cfg.jump.feetBottom:cfg.stand.feetBottom):(cfg.hurt3.cols>0?cfg.hurt3.feetBottom:cfg.hurt.feetBottom);
                return target.positionY()+target.frameHeight()+(fb-target.frameHeight())*target.visualScale();};
            const double before=feet();target.playDarkParticleHurt(3);
            QVERIFY(std::abs(feet()-before)<.001);QCOMPARE(target.state(),CharacterModel::Hurt);
        }
    }

    void darkParticleRoundResetAndClientAuthorityAreSafe() {
        FightDirector host;host.resetForNewRound("Orochi","Yagami");
        QVERIFY(host.p1Model()->playDarkParticle());tick(*host.p1Model(),14);
        const auto snapshot=host.darkParticles();QCOMPARE(snapshot.size(),1);
        FightDirector client;client.setAuthoritative(false);client.resetForNewRound("Orochi","Yagami");
        QVERIFY(client.p1Model()->playDarkParticle());tick(*client.p1Model(),14);
        QVERIFY(client.darkParticles().isEmpty());client.syncDarkParticles(snapshot);
        QSignalSpy hits(&client,&FightDirector::hitDetected);
        for(int i=0;i<200;++i)client.advanceDarkParticles(16);
        QCOMPARE(hits.count(),0);
        host.resetForNewRound("Orochi","Kusanagi");QVERIFY(host.darkParticles().isEmpty());
        client.syncDarkParticles({});QVERIFY(client.darkParticles().isEmpty());
    }

    void darkParticleTexturesAndDefaultBindings() {
        QImage actor(QStringLiteral(FIGHTGAME_SOURCE_DIR "/images/character/Orochi/DarkParticleActor.png"));
        QImage wave(QStringLiteral(FIGHTGAME_SOURCE_DIR "/images/character/Orochi/DarkParticleFlight.png"));
        QCOMPARE(actor.size(),QSize(3072,2560));QCOMPARE(wave.size(),QSize(1024,384));
        KeyBindingConfig keys;QCOMPARE(keys.getBinding("P1","darkParticle"),QString("P"));
        QCOMPARE(keys.getBinding("P2","darkParticle"),QString("NumPad7"));
        const auto cfg=character("Orochi");QCOMPARE(cfg.darkParticle.cols,34);QCOMPARE(cfg.darkParticle.sheetColumns,8);
    }

    void kusanagiBlockActualFeetMatchIdleAcrossWindowSizes() {
        const auto cfg=character("Kusanagi");
        QImage idle(QStringLiteral(FIGHTGAME_SOURCE_DIR "/")+cfg.stand.path.mid(5));
        QImage block(QStringLiteral(FIGHTGAME_SOURCE_DIR "/")+cfg.standBlock.path.mid(5));
        auto foot=[](const QImage &sheet,int frame,int fw,int fh) {
            for(int y=fh-1;y>=0;--y) for(int x=0;x<fw;++x)
                if(qAlpha(sheet.pixel((frame%8)*fw+x,(frame/8)*fh+y))>8) return y+1;
            return 0;
        };
        CharacterModel a,b;a.configure(cfg);b.configure(cfg);a.playStand();b.playStand();b.playStandBlock();
        for(const auto &size:QList<QSize>{{450,320},{640,360},{640,640},{900,640},{1280,720},{1920,1080}}) {
            a.updateRootHeight(size.height());b.updateRootHeight(size.height());
            const double fit=std::min(size.width()/900.0,size.height()/640.0);
            auto rendered=[&](const CharacterModel &model,int pixelFoot) {
                const double fh=model.frameHeight();
                const double y=size.height()+(model.positionY()-size.height()+fh)*fit-fh;
                return y+fh+(pixelFoot-fh)*fit*model.visualScale();
            };
            for(int i=0;i<cfg.standBlock.cols;++i)
                QVERIFY(std::abs(rendered(a,foot(idle,0,cfg.stand.fw,cfg.stand.fh))
                       -rendered(b,foot(block,i,cfg.standBlock.fw,cfg.standBlock.fh)))<fit+.001);
        }
    }

    void sunshineGuardRecoilsWithoutLiftingOrRepeatedDamage() {
        for(const QString &name:{"Orochi","Yagami","Kusanagi"}) {
            FightDirector director;director.resetForNewRound("Orochi",name);
            auto caster=director.p1Model();auto target=director.p2Model();
            target->playStandBlock();QVERIFY(caster->playSunshine());tick(*caster,28);
            QSignalSpy hits(&director,&FightDirector::hitDetected);
            connect(&director,&FightDirector::hitDetected,target,[&](int,int) {target->playSunshineBlock();});
            QVERIFY(director.checkCollision());QCOMPARE(hits.count(),1);QCOMPARE(hits[0][1].toInt(),3);
            QVERIFY(target->sunshineGuarded());const double y=target->positionY();const double x=target->posXRatio();
            for(int i=0;i<30;++i) {
                tick(*target);QCOMPARE(target->state(),CharacterModel::StandBlock);
                QCOMPARE(target->positionY(),y);QVERIFY(target->posXRatio()>=x);
                QVERIFY(target->posXRatio()-x<=.02801);QVERIFY(!director.checkCollision());
            }
            tick(*caster,22);tick(*target);
            QVERIFY(!target->sunshineGuarded());QCOMPARE(target->state(),CharacterModel::StandBlock);
            QCOMPARE(hits.count(),1);
            target->releaseStandBlock();
            for(int i=0;i<20 && target->state()!=CharacterModel::Stand;++i)tick(*target);
            QCOMPARE(target->state(),CharacterModel::Stand);
        }
    }

    void releasingSunshineGuardWaitsForTheEndOfTheLight() {
        CharacterModel caster,target;caster.configure(character("Orochi"));target.configure(character("Kusanagi"));
        caster.setOpponent(&target);target.setOpponent(&caster);caster.setPosXRatio(.2);target.setPosXRatio(.7);
        caster.playStand();target.playStand();target.playStandBlock();
        QVERIFY(caster.playSunshine());tick(caster,28);target.playSunshineBlock();
        target.releaseStandBlock();tick(target,50);
        QVERIFY(target.sunshineGuarded());QCOMPARE(target.state(),CharacterModel::StandBlock);
        QSignalSpy recovered(&target,&CharacterModel::controlRecovered);
        tick(caster,22);tick(target);
        for(int i=0;i<20 && target.state()!=CharacterModel::Stand;++i)tick(target);
        QCOMPARE(target.state(),CharacterModel::Stand);QCOMPARE(recovered.count(),1);
        target.playStandBlock();target.playSunshineBlock();target.playStand();
        QVERIFY(!target.sunshineGuarded());tick(target,50);QCOMPARE(target.state(),CharacterModel::Stand);
        target.playStandBlock();target.playSunshineBlock();target.configure(character("Orochi"));target.playStand();
        QVERIFY(!target.sunshineGuarded());
    }

    void qqCrouchPunchAndKickCanChainAndRecoverForEveryCharacter() {
        for(const QString &name:{"Orochi","Yagami"}) {
            CharacterModel model;model.configure(character(name));model.playStand();model.playCrouch();
            QSignalSpy done(&model,&CharacterModel::attackFinished);
            for(bool kick:{false,true,false,true}) {
                QVERIFY(model.playContextAttack(kick));
                QCOMPARE(model.state(),kick?CharacterModel::CrouchKick:CharacterModel::CrouchAttack);
                tick(model,model.totalFrames());QCOMPARE(model.state(),CharacterModel::Crouch);
            }
            QCOMPARE(done.count(),4);model.stopCrouch();
            for(int i=0;i<15 && model.state()!=CharacterModel::Stand;++i)tick(model);
            QCOMPARE(model.state(),CharacterModel::Stand);
        }
    }

    void qqAirAttackPreservesJumpHeightAndOnlyFiresOncePerJump() {
        for(const QString &name:{"Orochi","Yagami"}) {
            const auto cfg=character(name);
            for(bool kick:{false,true}) {
                CharacterModel model;model.configure(cfg);model.playStand();model.playJump();tick(model,(model.totalFrames()-1)/2);
                const double fb=cfg.jump.feetBottom>0?cfg.jump.feetBottom:cfg.stand.feetBottom;
                const double feet=model.positionY()+model.frameHeight()+(fb-model.frameHeight())*model.visualScale();
                QVERIFY(model.playContextAttack(kick));QVERIFY(model.isAirborne());
                const double attackFeet=model.positionY()+model.frameHeight()+(224-model.frameHeight())*model.visualScale();
                QVERIFY(std::abs(feet-attackFeet)<.001);
                QVERIFY(!model.playContextAttack(!kick));
                QSignalSpy land(&model,&CharacterModel::jumpFinished);
                QSignalSpy attack(&model,&CharacterModel::attackFinished);
                connect(&model,&CharacterModel::jumpFinished,&model,&CharacterModel::playStand);
                for(int i=0;i<80 && model.state()!=CharacterModel::Stand;++i)tick(model);
                QCOMPARE(model.state(),CharacterModel::Stand);QCOMPARE(land.count(),1);QCOMPARE(attack.count(),1);
                model.playJump();tick(model,(model.totalFrames()-1)/2);QVERIFY(model.playContextAttack(kick));
            }
        }
    }

    void qqAirAttacksRetainDiagonalTravelAndCanBeInterrupted() {
        for(const QString &name:{"Orochi","Yagami"}) {
            CharacterModel model;model.configure(character(name));model.setPosXRatio(.3);model.setFacingLeft(false);model.playStand();
            model.playDiagonalJump(true);while(!model.canAirAttack())tick(model);const double x=model.posXRatio();
            QVERIFY(model.playContextAttack(true));tick(model,2);QVERIFY(model.posXRatio()>x);
            model.playHurt1();QVERIFY(!model.isAirborne());
            tick(model,model.totalFrames());QCOMPARE(model.state(),CharacterModel::Stand);
        }
    }

    void qqRunLoopsAndBackstepIsAMovingVulnerableAction() {
        for(const QString &name:{"Orochi","Yagami"}) {
            CharacterModel model;model.configure(character(name));model.setPosXRatio(.6);model.setFacingLeft(false);model.playStand();
            QVERIFY(model.playRun());tick(model,model.totalFrames()*3+1);QCOMPARE(model.state(),CharacterModel::Run);
            QVERIFY(model.playBackstep());QCOMPARE(model.state(),CharacterModel::Backstep);
            QVERIFY(model.state()!=CharacterModel::Dodge);tick(model,2);QVERIFY(model.posXRatio()<.6);
            QSignalSpy recovered(&model,&CharacterModel::dodgeFinished);
            for(int i=0;i<30 && model.state()!=CharacterModel::Stand;++i)tick(model);
            QCOMPARE(model.state(),CharacterModel::Stand);QCOMPARE(recovered.count(),1);
        }
    }

    void qqOrochiAndKyoHaveKnockdownAndGetUpSequences() {
        for(const QString &name:{"Orochi"}) {
            CharacterModel model;model.configure(character(name));model.playStand();model.playHurt3();
            QVERIFY(model.sourcePath().contains("/Imported/hurt3.png"));
            tick(model,model.totalFrames());QCOMPARE(model.state(),CharacterModel::Rise);
            QVERIFY(model.sourcePath().contains("/Imported/rise.png"));tick(model,model.totalFrames());
            QCOMPARE(model.state(),CharacterModel::Stand);
        }
    }

    void qqImportedActionsHaveValidFramesAndActiveWindows() {
        for(const QString &name:{"Orochi","Yagami","Kusanagi"}) {
            const auto cfg=character(name);
            for(const auto *anim:{&cfg.crouchAttack,&cfg.crouchKick,&cfg.airPunch,&cfg.airKick,&cfg.run,&cfg.backstep}) {
                if(anim->cols<=0)continue;if(anim->path.contains("/Imported/"))QCOMPARE(anim->sheetColumns,8);
                QImage sheet(QStringLiteral(FIGHTGAME_SOURCE_DIR "/")+anim->path.mid(5));QVERIFY(!sheet.isNull());
                QVERIFY(sheet.width()<=4096 && sheet.height()<=4096);
                for(int i=0;i<anim->cols;++i) {
                    const int columns=anim->sheetColumns>0?anim->sheetColumns:anim->cols;
                    const QImage f=sheet.copy((i%columns)*anim->fw,(i/columns)*anim->fh,anim->fw,anim->fh);
                    bool visible=false;for(int y=0;y<f.height()&&!visible;++y)for(int x=0;x<f.width();++x)if(qAlpha(f.pixel(x,y))) {visible=true;break;}
                    QVERIFY(visible);
                }
            }
        }
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

    void reviewedCameraStaysStillForAnOnScreenTeleport() {
        FightDirector director;director.resetForNewRound("Orochi","Yagami");
        connect(director.p1Model(),&CharacterModel::posXRatioChanged,&director,&FightDirector::updateCamera);
        QSignalSpy camera(&director,&FightDirector::cameraOffsetChanged);
        director.p1Model()->playDodge(true);tick(*director.p1Model(),16);director.updateCamera();
        for(int i=0;i<30;++i)QVERIFY(QMetaObject::invokeMethod(&director,"onCameraTick",Qt::DirectConnection));
        QCOMPARE(director.cameraOffset(),0.0);QCOMPARE(camera.count(),0);
        director.p2Model()->setPosXRatio(1.2);director.updateCamera();
        double previous=0;
        for(int i=0;i<40;++i) {QVERIFY(QMetaObject::invokeMethod(&director,"onCameraTick",Qt::DirectConnection));QVERIFY(director.cameraOffset()>=previous);previous=director.cameraOffset();}
        QVERIFY(director.cameraOffset()>0);
    }

    void completedAirAttackRecoversToJumpWithoutUnlockingOrTurningMidair() {
        for(const QString &name:{"Orochi","Yagami"}) {
            CharacterModel a,b;const auto cfg=character(name);a.configure(cfg);b.configure(cfg);
            a.setOpponent(&b);a.setPosXRatio(.3);b.setPosXRatio(.7);a.playStand();a.playJump();
            QVERIFY(a.playContextAttack(true));b.setPosXRatio(.1);
            QSignalSpy land(&a,&CharacterModel::jumpFinished),attack(&a,&CharacterModel::attackFinished);
            bool recovered=false;
            while(a.isAirborne()) {
                a.advanceJump(16);
                if(a.isAirborne()) {
                    QVERIFY(!a.facingLeft());QCOMPARE(land.count(),0);QCOMPARE(attack.count(),0);
                    if(a.state()==CharacterModel::Jump){recovered=true;QVERIFY(!a.isAttackActive());QVERIFY(!a.canAirAttack());}
                }
            }
            QVERIFY(recovered);QVERIFY(a.facingLeft());QCOMPARE(land.count(),1);QCOMPARE(attack.count(),1);
        }
    }

    void reviewedPlainLandingFacesBeforePublishingRecovery() {
        for(const QString &name:{"Orochi","Yagami","Kusanagi"}) {
            for(bool diagonal:{false,true}) {
                CharacterModel model,other;model.configure(character(name));other.configure(character(name));
                model.setOpponent(&other);model.setPosXRatio(.3);other.setPosXRatio(.7);model.playStand();
                if(diagonal)model.playDiagonalJump(true);else model.playJump();
                other.setPosXRatio(.1);bool correct=false;
                connect(&model,&CharacterModel::jumpFinished,&model,[&]() {correct=model.state()==CharacterModel::Stand && model.facingLeft();});
                for(int i=0;i<80 && model.state()!=CharacterModel::Stand;++i)tick(model);
                QVERIFY(correct);
            }
        }
    }

    void reviewedAirContactPointsFollowTheVisibleAttack() {
        for(const QString &name:{"Orochi","Yagami","Kusanagi"}) {
            const auto cfg=character(name);
            for(const auto *anim:{&cfg.airPunch,&cfg.airKick}) {
                QCOMPARE(anim->attackContacts.size(),anim->cols);
                QImage image(QStringLiteral(FIGHTGAME_SOURCE_DIR "/")+anim->path.mid(5));
                for(int i=0;i<anim->cols;++i) {
                    const auto contact=anim->attackContacts[i];
                    const int px=qRound(128+contact.x()/anim->visualScale),py=qRound(224-contact.y()/anim->visualScale);
                    bool visible=false;
                    for(int y=std::max(0,py-3);y<std::min(256,py+4);++y)
                        for(int x=std::max(0,px-3);x<std::min(256,px+4);++x)
                            if(qAlpha(image.pixel((i%8)*256+x,(i/8)*256+y))>32)visible=true;
                    QVERIFY2(visible,qPrintable(name+QString(" frame %1").arg(i)));
                }
            }
        }
    }

    void reviewedAirKickCanHitNearTheGroundAndMissAtLongRange() {
        for(const QString &name:{"Orochi","Yagami"}) {
            bool found=false;
            for(int start=1;start<character(name).jump.cols-2 && !found;++start) {
                FightDirector director;director.resetForNewRound(name,"Yagami");
                auto a=director.p1Model();auto b=director.p2Model();a->setPosXRatio(.3);b->setPosXRatio(.49);
                a->playJump();tick(*a,start);if(!a->canAirAttack())continue;QVERIFY(a->playContextAttack(true));
                for(int i=0;i<30 && a->isAirborne();++i) {
                    if(director.checkCollision()){found=true;break;}tick(*a);
                }
            }
            QVERIFY2(found,qPrintable(name+" never hits a nearby standing defender"));
            FightDirector director;director.resetForNewRound(name,"Yagami");
            director.p1Model()->setPosXRatio(.1);director.p2Model()->setPosXRatio(.9);
            director.p1Model()->playJump();while(!director.p1Model()->canAirAttack())tick(*director.p1Model());QVERIFY(director.p1Model()->playContextAttack(true));
            for(int i=0;i<30 && director.p1Model()->isAirborne();++i) {QVERIFY(!director.checkCollision());tick(*director.p1Model());}
        }
    }

    void reviewedRunningJumpRetainsHeightWithoutOversizingNormalJumps() {
        for(const QString &name:{"Orochi","Yagami"}) {
            CharacterModel low,high;const auto cfg=character(name);low.configure(cfg);high.configure(cfg);
            low.playStand();high.playStand();low.playJump();QVERIFY(high.playRun());high.playJump();
            tick(low,cfg.jump.cols/2);tick(high,cfg.jump.cols/2);
            QVERIFY(high.positionY()<low.positionY()-150);
        }
    }

    void reviewedAirAndCrouchInputsBypassTheDodgeComboDelay() {
        QFile file(QStringLiteral(FIGHTGAME_SOURCE_DIR "/FightScreen.qml"));QVERIFY(file.open(QIODevice::ReadOnly));
        const auto qml=QString::fromUtf8(file.readAll());
        for(int player:{1,2})for(const QString &action:{"Punch","Kick"})for(bool airborne:{true,false}) {
            const QString name=QString("handleP%1Light%2").arg(player).arg(action);
            const int start=qml.indexOf("    function "+name+"()");QVERIFY(start>=0);
            const int end=qml.indexOf("\n    }",start);QVERIFY(end>start);
            QJSEngine engine;
            const QString setup=QString("var CharacterModel={Crouch:13};var calls=0,restarts=0;"
                "var director={p1Model:{airborne:%1,state:13},p2Model:{airborne:%1,state:13}};"
                "var p1ComboTimer={stop:function(){},restart:function(){restarts++}};"
                "var p2ComboTimer=p1ComboTimer;function tryContextAttack(p,k,h){calls++}").arg(airborne?"true":"false");
            QVERIFY(!engine.evaluate(setup).isError());
            QVERIFY(!engine.evaluate(qml.mid(start,end-start+6)).isError());
            auto result=engine.globalObject().property(name).call();QVERIFY(!result.isError());
            QCOMPARE(engine.globalObject().property("calls").toInt(),1);
            QCOMPARE(engine.globalObject().property("restarts").toInt(),0);
        }
    }

    void kyoExactlyMatchesThePreQQConfiguration() {
        QFile f(QStringLiteral(FIGHTGAME_SOURCE_DIR "/tests/fixtures/kusanagi_pre_qq.json"));
        QVERIFY(f.open(QIODevice::ReadOnly));
        QCOMPARE(characters.value("Kusanagi").toObject(),QJsonDocument::fromJson(f.readAll()).object());
        CharacterModel k;k.configure(character("Kusanagi"));k.playStand();
        QVERIFY(!k.playRun());QVERIFY(!k.playBackstep());k.playJump();
        for(int i=0;i<k.totalFrames()-1;++i){QVERIFY(!k.canAirAttack());QVERIFY(!k.playContextAttack(true));tick(k);}
    }

    void plannedLandingIsContinuousAndDoesNotTeleportAtRecovery() {
        for(const QString &name:{"Orochi","Yagami","Kusanagi"})for(bool left:{false,true})
        for(bool forward:{false,true})for(bool attack:{false,true})for(double start:{.02,.3,2.98}) {
            if(attack && name=="Kusanagi")continue;
            CharacterModel a,b;a.configure(character(name));b.configure(character("Yagami"));
            a.setOpponent(&b);a.setPosXRatio(start);b.setPosXRatio(left?start-.2:start+.2);
            a.playStand();a.setFacingLeft(left);a.playDiagonalJump(forward);
            const double end=a.landingX();QVERIFY(end>=0 && end<=3);
            const int count=forward?character(name).diagonalJump.divFrame:
                character(name).diagonalJump.cols-character(name).diagonalJump.divFrame;
            const double maxStep=std::abs(end-start)/std::max(1,count-1);
            double previous=start;bool recovered=false;
            connect(&a,&CharacterModel::jumpFinished,&a,[&](){recovered=true;QCOMPARE(a.posXRatio(),end);});
            for(int i=0;i<100 && a.state()!=CharacterModel::Stand;++i) {
                if(attack && a.canAirAttack())QVERIFY(a.playContextAttack(true));
                tick(a);
                QVERIFY2(std::abs(a.posXRatio()-previous)<=maxStep*2+.00001,"Landing produced an extra horizontal correction");
                previous=a.posXRatio();
            }
            QVERIFY(recovered);QCOMPARE(a.posXRatio(),end);
        }
    }

    void airAttackInputsWorkOnAscentApexAndDescentAndNeverRepeat() {
        for(const QString &name:{"Orochi","Yagami"})for(bool diagonal:{false,true})for(bool forward:{false,true}) {
            const auto cfg=character(name);
            const int n=diagonal?(forward?cfg.diagonalJump.divFrame:cfg.diagonalJump.cols-cfg.diagonalJump.divFrame):cfg.jump.cols;
            for(int frame=0;frame<n-1;++frame)for(bool kick:{false,true}) {
                CharacterModel a;a.configure(cfg);a.playStand();
                if(diagonal)a.playDiagonalJump(forward);else a.playJump();tick(a,frame);
                QVERIFY(a.canAirAttack());QVERIFY(a.playContextAttack(kick));
                QVERIFY(!a.playContextAttack(!kick));
                while(a.isAirborne()){a.advanceJump(16);QVERIFY(!a.playContextAttack(!kick));}
                QVERIFY(!a.isAttackActive());QCOMPARE(a.state(),CharacterModel::Stand);
            }
        }
    }

    void completedIntroRemainsVisibleAndStandingDuringTheNextIntro() {
        FightDirector d;d.start("Kusanagi","Orochi");
        QFile f(QStringLiteral(FIGHTGAME_SOURCE_DIR "/FightScreen.qml"));QVERIFY(f.open(QIODevice::ReadOnly));
        const QString qml=QString::fromUtf8(f.readAll());QJSEngine js;
        for(int phase=0;phase<3;++phase) {
            QCOMPARE(d.phase(),phase);
            if(phase>=1)QCOMPARE(d.p1Model()->state(),CharacterModel::Stand);
            if(phase==1)QCOMPARE(d.p2Model()->state(),CharacterModel::Opening);
            if(phase==2)QCOMPARE(d.p2Model()->state(),CharacterModel::Stand);
            for(int player=1;player<=2;++player) {
                const int pos=qml.indexOf(QString("id: p%1Layer").arg(player));QVERIFY(pos>=0);
                const QString expression=qml.mid(pos).section("visible:",1,1).section('\n',0,0).trimmed();
                QVERIFY(!js.evaluate(QString("var director={phase:%1};var FightDirector={Opening_P1:0,Opening_P2:1,Fighting:2}").arg(phase)).isError());
                QCOMPARE(js.evaluate(expression).toBool(),player==1 || phase>=1);
            }
            if(phase<2) {auto a=phase==0?d.p1Model():d.p2Model();for(int i=0;i<300 && d.phase()==phase;++i)tick(*a);}
        }
    }

    void movingOpponentDoesNotChangeTheLandingPlanAtTheLastFrame() {
        for(bool diagonal:{false,true})for(bool attack:{false,true}) {
            CharacterModel a,b;a.configure(character("Orochi"));b.configure(character("Yagami"));
            a.setOpponent(&b);b.setPosXRatio(.7);a.setPosXRatio(.3);a.playStand();
            if(diagonal)a.playDiagonalJump(true);else a.playJump();
            const double planned=a.landingX();
            if(attack){while(!a.canAirAttack())tick(a);QVERIFY(a.playContextAttack(true));}
            // Defender moves beneath the planned endpoint after takeoff.
            b.setPosXRatio(planned);
            for(int i=0;i<80 && a.state()!=CharacterModel::Stand;++i){QCOMPARE(a.landingX(),planned);tick(a);}
            QCOMPARE(a.state(),CharacterModel::Stand);QCOMPARE(a.posXRatio(),planned);
        }
    }

    void dodgedSunshineStageIsNeverChargedByALaterHit() {
        FightDirector d;d.resetForNewRound("Orochi","Yagami");auto a=d.p1Model();auto b=d.p2Model();
        QSignalSpy hits(&d,&FightDirector::hitDetected);QVERIFY(a->playSunshine());
        tick(*a,28);b->playDodge(true);
        for(int i=0;i<4;++i){QVERIFY(!d.checkCollision());tick(*a);}
        b->playStand();int total=0;
        for(int frame=32;frame<50;++frame){d.checkCollision();tick(*a);}
        QCOMPARE(hits.count(),5);for(const auto &hit:hits)total+=hit[1].toInt();QCOMPARE(total,30);
    }

    void airAttacksNeverChangeTheTrajectoryOrTheLandingTime() {
        for(const QString &name:{"Orochi","Yagami"})for(bool diagonal:{false,true})
        for(bool left:{false,true})for(bool kick:{false,true})for(double timing:{0.0,.15,.45,.72,.9}) {
            CharacterModel plain,attacked;const auto cfg=character(name);
            for(auto m:{&plain,&attacked}){m->configure(cfg);m->setPosXRatio(.8);m->playStand();m->setFacingLeft(left);
                if(diagonal)m->playDiagonalJump(true);else m->playJump();}
            auto feet=[](CharacterModel &m,const AnimParams &anim){const double fb=anim.feetBottom;
                return m.positionY()+m.frameHeight()+(fb-m.frameHeight())*m.visualScale();};
            bool fired=false;
            for(int step=0;step<100 && plain.isAirborne();++step) {
                if(!fired && plain.jumpProgress()>=timing) {
                    const double before=feet(attacked,diagonal?cfg.diagonalJump:cfg.jump);
                    QVERIFY(attacked.playContextAttack(kick));fired=true;
                    QVERIFY(std::abs(before-feet(attacked,kick?cfg.airKick:cfg.airPunch))<.001);
                }
                plain.advanceJump(16);attacked.advanceJump(16);
                QCOMPARE(plain.posXRatio(),attacked.posXRatio());QCOMPARE(plain.isAirborne(),attacked.isAirborne());
                const auto &current=attacked.state()==CharacterModel::AirKick?cfg.airKick:
                    attacked.state()==CharacterModel::AirPunch?cfg.airPunch:diagonal?cfg.diagonalJump:cfg.jump;
                if(plain.isAirborne())QVERIFY(std::abs(feet(plain,diagonal?cfg.diagonalJump:cfg.jump)-feet(attacked,current))<.001);
            }
            QVERIFY(fired);QCOMPARE(plain.posXRatio(),attacked.posXRatio());QVERIFY(!attacked.isAttackActive());
        }
    }

    void straightJumpsDoNotSlideWhenTheOpponentIsOverlapping() {
        for(const QString &name:{"Orochi","Yagami","Kusanagi"}) {
            CharacterModel a,b;a.configure(character(name));b.configure(character("Yagami"));
            a.setPosXRatio(.3);b.setPosXRatio(.31);a.setOpponent(&b);a.playStand();a.playJump();
            while(a.isAirborne()){a.advanceJump(16);QCOMPARE(a.posXRatio(),.3);}
        }
    }

    void sixteenMillisecondMotionHasNoExtraLandingStep() {
        for(const QString &name:{"Orochi","Yagami"})for(bool left:{false,true})for(bool attack:{false,true}) {
            CharacterModel a;const auto cfg=character(name);a.configure(cfg);a.setPosXRatio(1.0);a.playStand();a.setFacingLeft(left);a.playDiagonalJump(true);
            const double maxStep=cfg.diagonalJump.jumpDistance*16/((cfg.diagonalJump.divFrame-1)*cfg.diagonalJump.interval);
            double previous=a.posXRatio();if(attack)QVERIFY(a.playContextAttack(true));
            while(a.isAirborne()){a.advanceJump(16);QVERIFY(std::abs(a.posXRatio()-previous)<=maxStep+.000001);previous=a.posXRatio();}
            QCOMPARE(a.posXRatio(),a.landingX());
        }
    }

    void newJumpAtlasesMatchTheIdleSizeAndCenterAtBothEnds() {
        for(const QString &name:{"Orochi","Yagami"}) {
            const auto cfg=character(name);
            auto bounds=[](const QImage &im) {QRect r;for(int y=0;y<im.height();++y)for(int x=0;x<im.width();++x)
                if(qAlpha(im.pixel(x,y))>64)r=r.united(QRect(x,y,1,1));return r;};
            QImage stand(QStringLiteral(FIGHTGAME_SOURCE_DIR "/")+cfg.stand.path.mid(5));
            const QRect idle=bounds(stand.copy(0,0,cfg.stand.fw,cfg.stand.fh));
            for(const auto *anim:{&cfg.jump,&cfg.diagonalJump}) {
                QImage sheet(QStringLiteral(FIGHTGAME_SOURCE_DIR "/")+anim->path.mid(5));
                QList<int> ids={0,anim->cols-1};if(anim->divFrame>0){ids<<anim->divFrame-1<<anim->divFrame;}
                for(int i:ids) {
                    const QRect r=bounds(sheet.copy((i%8)*256,(i/8)*256,256,256));
                    QVERIFY(std::abs(r.height()*anim->visualScale-idle.height()*cfg.stand.visualScale)<5);
                    const double x=(r.center().x()-128)*anim->visualScale;
                    const double sx=(idle.center().x()-cfg.stand.fw/2.0)*cfg.stand.visualScale;
                    QVERIFY(std::abs(x-sx)<4);
                }
            }
        }
    }

    void walkingAfterLandingCannotPushAPlayerBackwardsOrExceedOneStep() {
        QFile f(QStringLiteral(FIGHTGAME_SOURCE_DIR "/FightScreen.qml"));QVERIFY(f.open(QIODevice::ReadOnly));
        const auto qml=QString::fromUtf8(f.readAll());QJSEngine js;
        for(int player:{1,2})for(bool right:{false,true})for(bool running:{false,true})for(double other:{.1,.39,.4,.41,.7,1.5}) {
            const QString pos=QString("p%1").arg(player);
            const QString search="director."+pos+"Model.posXRatio = ";
            int start=qml.indexOf(search,qml.indexOf(player==1?"id: moveTimer":"id: moveTimer2"));QVERIFY(start>=0);
            if(!right)start=qml.indexOf(search,start+search.size());
            const QString expr=qml.mid(start+search.size()).section('\n',0,0).trimmed();
            const double current=.4;const double limit=right?(current<other?other-.15:other+.95):(current>other?other+.15:other-.95);
            const QString setup=QString("var %1=.4,moveStep=.008,%1Running=%2,maxPos=%3,minPos=%3;").arg(pos).arg(running?"true":"false").arg(limit,0,'f',6);
            QVERIFY(!js.evaluate(setup).isError());const auto value=js.evaluate(expr);QVERIFY(!value.isError());
            const double delta=value.toNumber()-current;const double step=.008*(running?1.65:1);
            QVERIFY(std::abs(delta)<=step+.000001);QVERIFY(right?delta>=0:delta<=0);
        }
    }

    void aLateAirAttackIsCancelledOnLandingWithoutAGroundHit() {
        for(bool kick:{false,true})for(const QString &name:{"Orochi","Yagami"}) {
            FightDirector d;d.resetForNewRound(name,"Yagami");auto a=d.p1Model();auto b=d.p2Model();
            a->setPosXRatio(.3);b->setPosXRatio(.45);a->playJump();
            const auto cfg=character(name);a->advanceJump((cfg.jump.cols-1)*cfg.jump.interval-16);
            QVERIFY(a->playContextAttack(kick));QVERIFY(!d.checkCollision());a->advanceJump(16);
            QCOMPARE(a->state(),CharacterModel::Stand);QVERIFY(!d.checkCollision());QVERIFY(!a->isAttackActive());
        }
    }

    void orochiKnockdownTravelsTheSameDistanceAtAComparableSpeed() {
        const auto cfg=character("Orochi");
        QCOMPARE(cfg.hurt3.flyDistance,character("Kusanagi").hurt3.flyDistance);
        QCOMPARE(cfg.hurt3.flyDistance,character("Yagami").hurt3.flyDistance);
        const double landingMs=(cfg.hurt3.cols-1)*cfg.hurt3.interval*cfg.hurt3.flyLandT;
        QVERIFY(landingMs>=750 && landingMs<=900);
        CharacterModel a,b;a.configure(cfg);b.configure(character("Yagami"));
        a.setOpponent(&b);a.setPosXRatio(1.0);b.setPosXRatio(.7);a.playStand();a.playHurt3();
        QSignalSpy done(&a,&CharacterModel::hurtFinished);double previous=a.posXRatio();
        const double maxStep=cfg.hurt3.flyDistance*16/landingMs;
        for(int elapsed=16;elapsed<=800;elapsed+=16) {
            const int oldFrame=a.currentFrame();a.advanceKnockdown(16);
            QVERIFY(a.posXRatio()>previous);QVERIFY(a.posXRatio()-previous<=maxStep+.000001);
            if(elapsed==16)QCOMPARE(a.currentFrame(),oldFrame); // Movement between sprite frames.
            previous=a.posXRatio();QCOMPARE(done.count(),0);
        }
        QVERIFY(a.posXRatio()<1.5);a.advanceKnockdown(16);QCOMPARE(a.posXRatio(),1.5);
        for(int i=0;i<40 && a.state()==CharacterModel::Hurt;++i)a.advanceKnockdown(16);
        QCOMPARE(a.state(),CharacterModel::Rise);QCOMPARE(done.count(),0);
        tick(a,a.totalFrames());QCOMPARE(a.state(),CharacterModel::Stand);QCOMPARE(done.count(),1);
    }

    void allKnockdownsKeepTheirConfiguredLandingDistanceAndArc() {
        for(const QString &name:{"Orochi","Kusanagi","Yagami"})for(bool left:{false,true}) {
            CharacterModel a,b;const auto cfg=character(name);a.configure(cfg);b.configure(character("Yagami"));
            a.setPosXRatio(1.0);b.setPosXRatio(left?1.3:.7);a.setOpponent(&b);a.playStand();a.playHurt3();
            const double ground=a.positionY();bool lifted=false;
            const int landMs=qCeil((cfg.hurt3.cols-1)*cfg.hurt3.interval*cfg.hurt3.flyLandT);
            for(int elapsed=0;elapsed<landMs;elapsed+=16){a.advanceKnockdown(16);if(a.positionY()<ground)lifted=true;}
            QVERIFY(lifted);QVERIFY(std::abs(a.posXRatio()-(1+(left?-1:1)*cfg.hurt3.flyDistance))<.00001);
            QVERIFY(std::abs(a.positionY()-ground)<.00001);
        }
    }

};

QTEST_GUILESS_MAIN(AnimationRegressionTest)
#include "animation_regression_test.moc"
