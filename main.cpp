// Module
// File: main.cpp   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-06 18:02:16
// Description:
//     应用程序入口: 创建 Qt6 Quick 窗口, 加载 Main.qml 主菜单界面
//
#include "appcontroller.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    AppController::singleton();

    return QCoreApplication::exec();
}
