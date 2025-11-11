#pragma once

#include <memory>
#include <vector>

#include <QByteArray>
#include <QDebug>
#include <QObject>
#include <QString>

#include "ft4222.h"

class Ft4222Wrapper : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(QString slaveAddress READ slaveAddress WRITE setSlaveAddress NOTIFY
                   slaveAddressChanged)

public:
    explicit Ft4222Wrapper(QObject *parent = nullptr)
        : QObject(parent), m_connected(false) {}

    // Свойства
    bool isConnected() const { return m_connected; }

    QString slaveAddress() const { return m_slaveAddress; }

    void setSlaveAddress(const QString &addr) {
        if (m_slaveAddress == addr)
            return;
        m_slaveAddress = addr;
        emit slaveAddressChanged();
    }

    Q_INVOKABLE bool connectDevice() {
        if (m_connected)
            return true;

        try {
            m_ft4222 = std::make_unique<Ft4222>(SYS_CLK_60);
            m_connected = true;

            emit connectionChanged();

            // Сразу выводим информацию о найденных устройствах
            DWORD count = m_ft4222->listFtDevices();
            emit logMessage(QString("Found %1 device(s)").arg(count));

            for (DWORD i = 0; i < count; ++i) {
                const auto &dev = m_ft4222->getDeviceInfo(
                    i); // Нужно добавить метод getDeviceInfo в Ft4222
                QString info = QString("Device %1:\n  Serial: %2\n  Description: %3\n")
                                   .arg(i)
                                   .arg(dev.SerialNumber)
                                   .arg(dev.Description);
                emit logMessage(info);
            }

        } catch (const std::exception &e) {
            emit errorOccurred(QString("FT4222 connect failed: %1").arg(e.what()));
            m_connected = false;
            emit connectionChanged();
        }
        return m_connected;
    }

    Q_INVOKABLE DWORD scanDevices();

    // Отключение устройства
    Q_INVOKABLE void disconnectDevice() {
        if (m_ft4222) {
            m_ft4222.reset(); // безопасный вызов деструктора
            m_connected = false;
            emit connectionChanged();
        }
    }

    // Чтение памяти
    Q_INVOKABLE QByteArray readMem(int devAddr, int memAddr, int size) {
        QByteArray buffer(size, Qt::Uninitialized);
        if (!m_ft4222)
            return QByteArray();
        try {
            m_ft4222->i2cMemRead(devAddr, memAddr,
                                 reinterpret_cast<uint8_t *>(buffer.data()), size);
        } catch (const std::exception &e) {
            emit errorOccurred(QString("Read failed: %1").arg(e.what()));
            return QByteArray();
        }
        return buffer;
    }

    // Запись памяти
    Q_INVOKABLE void writeMem(int devAddr, int memAddr, QByteArray data) {
        if (!m_ft4222)
            return;
        try {
            m_ft4222->i2cMemWrite(devAddr, memAddr,
                                  reinterpret_cast<const uint8_t *>(data.constData()),
                                  data.size());
        } catch (const std::exception &e) {
            emit errorOccurred(QString("Write failed: %1").arg(e.what()));
        }
    }

signals:

    void connectionChanged();

    void slaveAddressChanged();

    void errorOccurred(const QString &msg);

    void logMessage(const QString &msg);

private:
    std::unique_ptr<Ft4222> m_ft4222;
    bool m_connected;
    QString m_slaveAddress;
};
