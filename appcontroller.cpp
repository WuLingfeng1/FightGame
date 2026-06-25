// Module
// File: appcontroller.cpp   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-25 14:30:00
// Description:
//     应用程序控制器单例类实现
#include "appcontroller.h"
#include <QCoreApplication>

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    m_app = qobject_cast<QGuiApplication*>(QCoreApplication::instance());

    m_engine = new QQmlApplicationEngine(this);

    QObject::connect(
        m_engine,
        &QQmlApplicationEngine::objectCreationFailed,
        m_app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    m_engine->loadFromModule("FightGame", "Main");
}

AppController::~AppController()
{
    delete m_engine;
}

AppController* AppController::singleton()
{
    static AppController instance;
    return &instance;
}
