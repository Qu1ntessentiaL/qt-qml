#pragma once

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>

class UartBackend : public QObject {
Q_OBJECT

public:
    explicit UartBackend(QObject *parent = nullptr);

    /**
     * @brief Сканирует список доступных COM-портов
     * @return Возвращает список доступных COM-портов
     */
    Q_INVOKABLE QStringList refreshPorts();

    /**
     * @brief Подключается к порту с именем portName
     * @param portName Имя доступного COM-порта
     * @param baudRate Символьная скорость COM-порта
     * @return true - успешно, false - не смог подключиться
     */
    Q_INVOKABLE bool connectPort(const QString &portName, int baudRate);

    /**
     * @brief Отключается от COM-порта
     */
    Q_INVOKABLE void disconnectPort();

    /**
     * @brief Очистить буфер данных
     */
    Q_INVOKABLE void clearData();

signals:

    void dataReceived(const QString &htmlText);

private slots:

    void handleReadyRead();

private:
    QSerialPort m_serial;
    QByteArray m_buffer;

    bool m_isConnected = false;
};
