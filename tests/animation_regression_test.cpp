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
