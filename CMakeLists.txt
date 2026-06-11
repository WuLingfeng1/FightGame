cmake_minimum_required(VERSION 4.2.3)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
# cmake_minimum_required(VERSION 4.3.2)
# set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
set(CMAKE_CXX_MODULE_STD ON)

project(FightGame VERSION 0.1 LANGUAGES CXX)
set(QT_QML_GENERATE_QMLLS_INI ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
qt_standard_project_setup(REQUIRES 6.11)

qt_add_executable(appFightGame  WIN32)
qt_add_qml_module(appFightGame
    URI FightGame
    VERSION 1.0
    SOURCES     main.cpp
    QML_FILES   Main.qml
    QML_FILES SelectScreen.qml
    QML_FILES FightScreen.qml
    SOURCES fightermodel.h fightermodel.cpp
)
qt_add_resources(appFightGame "appresources"
    PREFIX "/"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES
        images/background.jpg
        images/background2.jpg
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
        images/character/Yagami/Stand.png
        images/character/Yagami/opening.png
        images/character/Yagami/BackWard.png
        images/character/Yagami/Crouch.png
        images/character/Yagami/DiagonalJump.png
        images/character/Yagami/Dodge.png
        images/character/Yagami/Forward.png
        images/character/Yagami/Jump.png
        images/character/Yagami/Knockdown.png
        images/character/Yagami/PowerUp.png
        images/character/Yagami/Rise.png
        images/character/Yagami/Run.png
        images/character/Yagami/StandAttack.png
        images/character/Yagami/StandBlock.png
        images/character/Yagami/SuperMove.png
        images/character/Yagami/挨打1.png
        images/character/Yagami/挨打2.png
        images/character/Yagami/挨打3.png
        images/character/Yagami/挨打4.png
        images/character/Yagami/挨打5.png
        images/character/Yagami/暗勾手.png
        images/character/Yagami/超重击.png
        images/character/Yagami/蹲攻.png
        images/character/Yagami/疾进.png
        images/character/Yagami/疾退.png
        images/character/Yagami/近攻.png
        images/character/Yagami/空防.png
        images/character/Yagami/空中超重击.png
        images/character/Yagami/逆剥.png
        images/character/Yagami/逆逆剥.png
        images/character/Yagami/跳攻.png
        images/FightBackGround/AmusementPark.gif
        images/FightBackGround/Bali.gif
        images/FightBackGround/Gyeongbokgung.gif
        images/FightBackGround/Monaco.gif
        images/FightBackGround/OrochiShermie.gif
)
target_compile_features(appFightGame PRIVATE cxx_std_23)
target_link_libraries(appFightGame
    PRIVATE Qt6::Quick
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
