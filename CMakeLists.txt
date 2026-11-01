cmake_minimum_required(VERSION 3.22)

project(Fightgame VERSION 0.1 LANGUAGES CXX)
set(QT_QML_GENERATE_QMLLS_INI ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network Widgets Multimedia)
qt_standard_project_setup(REQUIRES 6.9)

qt_add_executable(appFightGame  WIN32)
qt_add_qml_module(appFightGame
    URI FightGame
    VERSION 1.0
    SOURCES main.cpp
    SOURCES appcontroller.h appcontroller.cpp
    QML_FILES   Main.qml
    QML_FILES SelectScreen.qml
    QML_FILES StageSelectScreen.qml
    QML_FILES FightScreen.qml
    QML_FILES FighterGeometry.js
    QML_FILES SunshineEffect.qml
    QML_FILES SunshineGuardEffect.qml
    QML_FILES DarkParticleEffect.qml
    QML_FILES OnlineLobby.qml
    QML_FILES KeyBindingScreen.qml
    SOURCES charactermodel.h charactermodel.cpp
    SOURCES fightdirector.h fightdirector.cpp
    SOURCES characterconfig.h characterconfig.cpp
    SOURCES keybindingconfig.h keybindingconfig.cpp
    SOURCES networkmanager.h networkmanager.cpp
    SOURCES roommanager.h roommanager.cpp
    SOURCES voicemanager.h voicemanager.cpp
    SOURCES musicmanager.h musicmanager.cpp
)
qt_add_resources(appFightGame "appresources"
    PREFIX "/"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES
        images/background.jpg
        config/characters.json
        config/keybindings.json
        images/avatar/Kdash.jpg
        images/avatar/Kula.jpg
        images/avatar/Kusanagi.jpg
        images/avatar/Orochi.jpg
        images/avatar/Shiranui.jpg
        images/avatar/Terry.jpg
        images/avatar/Yagami.jpg
        images/portrait/Kdash.jpg
        images/portrait/Kula.jpg
        images/portrait/Kusanagi.jpg
        images/portrait/Orochi.jpg
        images/portrait/Shiranui.jpg
        images/portrait/Yagami.jpg
        images/character/Yagami/Imported/crouchAttack.png
        images/character/Yagami/Imported/crouchKick.png
        images/character/Yagami/Imported/airPunch.png
        images/character/Yagami/Imported/airKick.png
        images/character/Yagami/Imported/run.png
        images/character/Yagami/Imported/backstep.png
        images/character/Orochi/Imported/crouchKick.png
        images/character/Orochi/Imported/airPunch.png
        images/character/Orochi/Imported/airKick.png
        images/character/Orochi/Imported/run.png
        images/character/Orochi/Imported/backstep.png
        images/character/Orochi/Imported/hurt3.png
        images/character/Orochi/Imported/rise.png
        images/character/Orochi/Imported/jump.png
        images/character/Orochi/Imported/diagonalJump.png
        images/character/Yagami/Imported/jump.png
        images/character/Yagami/Imported/diagonalJump.png
        images/character/Orochi/ReviewHurt.png
        images/character/Yagami/ReviewHurt1.png
        images/character/Yagami/ReviewHurt2.png
        images/character/Orochi/Motion_jump.png
        images/character/Orochi/Motion_diagonalJump.png
        images/character/Orochi/Motion_airPunch.png
        images/character/Orochi/Motion_airKick.png
        images/character/Yagami/Motion_jump.png
        images/character/Yagami/Motion_diagonalJump.png
        images/character/Yagami/Motion_airPunch.png
        images/character/Yagami/Motion_airKick.png
        images/character/Orochi/Stand.png
        images/character/Orochi/DarkParticleActor.png
        images/character/Orochi/DarkParticleFlight.png
        images/character/Orochi/SunshineAtlas.png
        images/character/Orochi/SunshineForeground.png
        images/character/Orochi/SunshineBackground.png
        images/character/Orochi/opening.png
        images/character/Orochi/Forward.png
        images/character/Orochi/Backward.png
        images/character/Orochi/Jump.png
        images/character/Orochi/DiagonalJump.png
        images/character/Orochi/LightPunch.png
        images/character/Orochi/LightKick.png
        images/character/Orochi/HeavyPunch.png
        images/character/Orochi/HeavyKick.png
        images/character/Orochi/HeavyStrike.png
        images/character/Orochi/Hurt.png
        images/character/Orochi/CrouchAttack.png
        images/character/Orochi/Crouch.png
        images/character/Orochi/Dodge.png
        images/character/Orochi/DodgeSmoothAtlas.png
        images/character/Orochi/StandBlock.png
        images/character/Orochi/win.png
        images/character/Orochi/lose.png
        images/character/Kusanagi/opening.png
        images/character/Kusanagi/stand.png
        images/character/Kusanagi/forward.png
        images/character/Kusanagi/backward.png
        images/character/Kusanagi/jump.png
        images/character/Kusanagi/crouch.png
        images/character/Kusanagi/DiagonalJump.png
        images/character/Kusanagi/LightPunch.png
        images/character/Kusanagi/LightKick.png
        images/character/Kusanagi/HeavyPunch.png
        images/character/Kusanagi/HeavyKick.png
        images/character/Kusanagi/heavystrick.png
        images/character/Kusanagi/Dodge.png
        images/character/Kusanagi/standblock.png
        images/character/Kusanagi/hurt2.png
        images/character/Kusanagi/knockaway.png
        images/character/Kusanagi/win.png
        images/character/Kusanagi/lose.png
        images/character/Yagami/Stand.png
        images/character/Yagami/opening.png
        images/character/Yagami/Backward.png
        images/character/Yagami/Crouch.png
        images/character/Yagami/DiagonalJump.png
        images/character/Yagami/Dodge.png
        images/character/Yagami/Forward.png
        images/character/Yagami/Jump.png
        images/character/Yagami/Knockaway.png
        images/character/Yagami/PowerUp.png
        images/character/Yagami/Rise.png
        images/character/Yagami/Run.png
        images/character/Yagami/StandAttack_LightPunch.png
        images/character/Yagami/StandAttack_LightKick.png
        images/character/Yagami/StandAttack_HeavyPunch.png
        images/character/Yagami/StandAttack_HeavyKick.png
        images/character/Yagami/StandBlock.png
        images/character/Yagami/SuperMove.png
        images/character/Yagami/Hurt1_std.png
        images/character/Yagami/Hurt2_std.png
        images/character/Yagami/Hurt3_std.png
        images/character/Yagami/DarkThrust.png
        images/character/Yagami/HeavyPunch.png
        images/character/Yagami/CrouchAttack.png
        images/character/Yagami/CloseAttack.png
        images/character/Yagami/AirBlock.png
        images/character/Yagami/AirHeavyPunch.png
        images/character/Yagami/ReverseStrike.png
        images/character/Yagami/ReverseStrike2.png
        images/character/Yagami/win.png
        images/character/Yagami/lose.png
        images/character/Yagami/JumpAttack.png
        images/FightBackGround/AmusementPark.gif
        images/FightBackGround/Bali.gif
        images/FightBackGround/Gyeongbokgung.gif
        images/FightBackGround/Monaco.gif
        images/FightBackGround/OrochiShermie.gif
        images/Attack/1.png
        images/Attack/2.png
        images/Attack/3.png
        images/FightBackGround/jiulong.gif
        resources/voices/Orochi/heavyKick1.wav
        resources/voices/Orochi/heavyPunch1.wav
        resources/voices/Orochi/hitHeavyKick1.wav
        resources/voices/Orochi/hitHeavyPunch1.wav
        resources/voices/Orochi/hitKick1.wav
        resources/voices/Orochi/hitPunch1.wav
        resources/voices/Orochi/hitStrike1.wav
        resources/voices/Orochi/hurt1.wav
        resources/voices/Orochi/jump1.wav
        resources/voices/Orochi/kick1.wav
        resources/voices/Orochi/opening1.wav
        resources/voices/Orochi/punch1.wav
        resources/voices/Orochi/step1.wav
        resources/voices/Orochi/step2.wav
        resources/voices/Orochi/strike1.wav
        resources/voices/Orochi/win.wav
        resources/voices/Yagami/heavyKick1.wav
        resources/voices/Yagami/heavyPunch1.wav
        resources/voices/Yagami/hitHeavyKick1.wav
        resources/voices/Yagami/hitHeavyPunch1.wav
        resources/voices/Yagami/hitKick1.wav
        resources/voices/Yagami/hitPunch1.wav
        resources/voices/Yagami/hitStrike1.wav
        resources/voices/Yagami/hurt1.wav
        resources/voices/Yagami/hurt2.wav
        resources/voices/Yagami/jump1.wav
        resources/voices/Yagami/kick1.wav
        resources/voices/Yagami/opening1.wav
        resources/voices/Yagami/punch1.wav
        resources/voices/Yagami/step1.wav
        resources/voices/Yagami/step2.wav
        resources/voices/Yagami/strike1.wav
        resources/voices/Yagami/win.wav
        resources/voices/Kusanagi/opening1.mp3
        resources/voices/Kusanagi/punch1.mp3
        resources/voices/Kusanagi/kick1.mp3
        resources/voices/Kusanagi/heavyPunch1.mp3
        resources/voices/Kusanagi/heavyKick1.mp3
        resources/voices/Kusanagi/strike1.mp3
        resources/voices/Kusanagi/step1.mp3
        resources/voices/Kusanagi/step2.mp3
        resources/voices/Kusanagi/hurt1.mp3
        resources/voices/Kusanagi/hurt2.mp3
        resources/voices/Kusanagi/hurt3.mp3
        resources/voices/Kusanagi/hitPunch1.mp3
        resources/voices/Kusanagi/hitKick1.mp3
        resources/voices/Kusanagi/hitHeavyPunch1.mp3
        resources/voices/Kusanagi/hitHeavyKick1.mp3
        resources/voices/Kusanagi/hitStrike1.mp3
        resources/voices/Kusanagi/win.mp3
        resources/BackGround/AmusementPark.flac
        resources/BackGround/Bali.flac
        resources/BackGround/Gyeongbokgung.flac
        resources/BackGround/jiulong.flac
        resources/BackGround/Monaco.flac
        resources/select/begin.flac
        resources/select/select.flac
        resources/select/victory.flac
)

target_compile_features(appFightGame PRIVATE cxx_std_23)
target_link_libraries(appFightGame
    PRIVATE Qt6::Quick Qt6::Network Qt6::Widgets Qt6::Multimedia
)

set(CMAKE_INSTALL_PREFIX "/opt/${PROJECT_NAME}" CACHE PATH "Install path prefix" FORCE)
install(TARGETS appFightGame
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

option(FIGHTGAME_BUILD_TESTS "Build character animation regression tests" OFF)
if(FIGHTGAME_BUILD_TESTS)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)
    qt_add_executable(fightgame_animation_tests
        tests/animation_regression_test.cpp
        fightdirector.h fightdirector.cpp
        keybindingconfig.h keybindingconfig.cpp
        charactermodel.h charactermodel.cpp
        characterconfig.h characterconfig.cpp
        voicemanager.h voicemanager.cpp
    )
    target_include_directories(fightgame_animation_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_definitions(fightgame_animation_tests PRIVATE FIGHTGAME_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_features(fightgame_animation_tests PRIVATE cxx_std_23)
    target_link_libraries(fightgame_animation_tests PRIVATE Qt6::Test Qt6::Qml Qt6::Multimedia)
    qt_add_resources(fightgame_animation_tests regression_config
        PREFIX "/"
        BASE "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES config/characters.json config/keybindings.json
    )
    add_test(NAME animation_regressions COMMAND fightgame_animation_tests)
endif()
