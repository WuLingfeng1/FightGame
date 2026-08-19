cmake_minimum_required(VERSION 3.22)

project(FightGame VERSION 0.1 LANGUAGES CXX)
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
        images/character/Orochi/Stand.png
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
        images/character/Orochi/StandBlock.png
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
        images/character/Kusanagi/hurt2.png
        images/character/Kusanagi/knockaway.png
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
        resources/voices/Orochi/win1.wav
        resources/voices/Orochi/win2.wav
        resources/voices/Orochi/win3.wav
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
        resources/voices/Yagami/win1.wav
        resources/voices/Yagami/win2.wav
        resources/voices/Yagami/win3.wav
        resources/voices/Yagami/win4.wav
        resources/BackGround/AmusementPark.flac
        resources/BackGround/Bali.flac
        resources/BackGround/Gyeongbokgung.flac
        resources/BackGround/jiulong.flac
        resources/BackGround/Monaco.flac
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

# C++ Modules support
set(FIGHTGAME_MODULE_FILES
    # math.cppm                             # 模块文件
)
source_group("FightGame" FILES ${FIGHTGAME_MODULE_FILES})
target_sources(appFightGame
    PRIVATE FILE_SET cxx_modules TYPE CXX_MODULES
        BASE_DIRS   ${CMAKE_CURRENT_SOURCE_DIR}
        FILES  ${FIGHTGAME_MODULE_FILES}
)