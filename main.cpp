// Module
// File: main.cpp   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-06 18:02:16
// Description:
//
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("FightGame", "Main");

    return QGuiApplication::exec();
}
