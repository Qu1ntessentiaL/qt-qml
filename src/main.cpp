#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "SerialManager.h"
#include "ft4222_wrapper.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // Регистрируем классы как QML-типы
    qmlRegisterType<SerialManager>("MyLib", 1, 0, "SerialManager");
    qmlRegisterType<Ft4222Wrapper>("MyLib", 1, 0, "Ft4222Wrapper");

    QQmlApplicationEngine engine;

    // Подключаем обработку ошибки создания объекта
    QObject::connect(
            &engine,
            &QQmlApplicationEngine::objectCreationFailed,
            &app,
            []() { QCoreApplication::exit(-1); },
            Qt::QueuedConnection);

    engine.loadFromModule("qt-qml", "Main");

    return app.exec();
}
