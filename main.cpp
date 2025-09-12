#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "UartBackend.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    UartBackend backend;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("backend", &backend);


    QObject::connect(
            &engine,
            &QQmlApplicationEngine::objectCreationFailed,
            &app,
            []() { QCoreApplication::exit(-1); },
            Qt::QueuedConnection);
    engine.loadFromModule("qt-qml", "Main");

    return app.exec();
}
