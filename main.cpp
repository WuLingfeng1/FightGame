// Module
// File: main.cpp   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-06 18:02:16
// Description:
//     应用程序入口: 创建 Qt6 Quick 窗口, 加载 Main.qml 主菜单界面
//
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    // 如果 QML 模块加载失败则退出程序
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("FightGame", "Main");  // 加载 FightGame 模块中的 Main.qml

    return QGuiApplication::exec();  // 进入 Qt 事件循环
}

