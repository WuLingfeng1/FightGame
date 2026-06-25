// Module
// File: appcontroller.h   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-25 14:30:00
// Description:
//     应用程序控制器单例类
#pragma once

#include <QObject>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

class AppController : public QObject
{
    Q_OBJECT

public:
    static AppController* singleton();

private:
    explicit AppController(QObject *parent = nullptr);
    ~AppController();

    AppController(const AppController&) = delete;
    AppController& operator=(const AppController&) = delete;

    QGuiApplication *m_app;
    QQmlApplicationEngine *m_engine;
};
