// SPDX-License-Identifier: GPL-2.0+

// Copyright © 2022,2026 Collabora Ltd
// Copyright © 2022,2026 Valve Corporation

#include <QIcon>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QIcon icon;

    icon.addFile(":/icons/steamos-reset.svg");
    engine.load(QUrl("qrc:/holo-reset.qml"));
    app.setWindowIcon(icon);

    return app.exec();
}
