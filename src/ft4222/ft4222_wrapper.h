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
    Q_PROPERTY(int registerAddressWidth READ registerAddressWidth WRITE setRegisterAddressWidth
                   NOTIFY registerAddressWidthChanged)

public:
    explicit Ft4222Wrapper(QObject *parent = nullptr)
        : QObject(parent),
          m_connected(false),
          m_registerAddressWidth(1) {}

    // Свойства
    bool isConnected() const { return m_connected; }

    QString slaveAddress() const { return m_slaveAddress; }

    void setSlaveAddress(const QString &addr) {
        if (m_slaveAddress == addr)
            return;
        m_slaveAddress = addr;
        emit slaveAddressChanged();
    }

    int registerAddressWidth() const { return m_registerAddressWidth; }

    void setRegisterAddressWidth(int width) {
        if (width == m_registerAddressWidth) {
            return;
        }
        if (width < 1 || width > 2) {
            emit errorOccurred(tr("Register address width must be 1 or 2 bytes"));
            return;
        }
        m_registerAddressWidth = width;
        if (m_ft4222) {
            try {
                m_ft4222->setRegisterAddressWidth(static_cast<uint8_t>(m_registerAddressWidth));
            } catch (const std::exception &e) {
                emit errorOccurred(QString("Failed to set address width: %1").arg(e.what()));
            }
        }
        emit registerAddressWidthChanged();
    }

    Q_INVOKABLE bool connectDevice() {
        if (m_connected)
            return true;

        try {
            m_ft4222 = std::make_unique<Ft4222>(SYS_CLK_60);
            m_ft4222->setRegisterAddressWidth(static_cast<uint8_t>(m_registerAddressWidth));

            const DWORD count = m_ft4222->listFtDevices();
            emit logMessage(QString("Found %1 device(s)").arg(count));

            if (count == 0) {
                emit errorOccurred("FT4222 device not found");
                m_ft4222.reset();
                return false;
            }

            m_ft4222->initializeDevice(0);
            m_connected = true;
            emit connectionChanged();

            const auto &devices = m_ft4222->devices();
            for (std::size_t i = 0; i < devices.size(); ++i) {
                const auto &dev = devices.at(i);
                QString info = QString("Device %1:\n  Serial: %2\n  Description: %3\n")
                                   .arg(i)
                                   .arg(dev.info.SerialNumber)
                                   .arg(dev.info.Description);
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
            try {
                m_ft4222->shutdown();
            } catch (const std::exception &e) {
                emit errorOccurred(QString("Shutdown failed: %1").arg(e.what()));
            }
            m_ft4222.reset();
            m_connected = false;
            emit connectionChanged();
        }
    }

    // Чтение памяти
    Q_INVOKABLE QString readMem(int devAddr, int memAddr, int size, int addressWidth = -1) {
        if (!m_ft4222)
            return {};

        const int effectiveWidth = addressWidth > 0 ? addressWidth : m_registerAddressWidth;
        if (size <= 0) {
            emit errorOccurred("Read size must be positive");
            return {};
        }

        QByteArray buffer(size, Qt::Uninitialized);

        try {
            const auto status = m_ft4222->i2cMemRead(static_cast<uint16_t>(devAddr),
                                                     static_cast<uint16_t>(memAddr),
                                                     reinterpret_cast<uint8_t *>(buffer.data()),
                                                     static_cast<uint16_t>(size),
                                                     static_cast<uint8_t>(effectiveWidth));
            if (status != FT4222_OK) {
                emit errorOccurred(
                    QString("Read failed with status code: %1").arg(static_cast<int>(status)));
                return {};
            }
        } catch (const std::exception &e) {
            emit errorOccurred(QString("Read failed: %1").arg(e.what()));
            return {};
        }

        QString hexString = QString::fromLatin1(buffer.toHex(' ')).toUpper();
        return hexString;
    }

    // Запись памяти
    Q_INVOKABLE void writeMem(int devAddr, int memAddr, const QByteArray &data, int addressWidth = -1) {
        if (!m_ft4222) {
            return;
        }
        const int effectiveWidth = addressWidth > 0 ? addressWidth : m_registerAddressWidth;
        if (data.isEmpty()) {
            emit errorOccurred("Write buffer is empty");
            return;
        }
        try {
            const auto status = m_ft4222->i2cMemWrite(static_cast<uint16_t>(devAddr),
                                                      static_cast<uint16_t>(memAddr),
                                                      reinterpret_cast<const uint8_t *>(data.constData()),
                                                      static_cast<uint16_t>(data.size()),
                                                      static_cast<uint8_t>(effectiveWidth));
            if (status != FT4222_OK) {
                emit errorOccurred(
                    QString("Write failed with status code: %1").arg(static_cast<int>(status)));
            }
        } catch (const std::exception &e) {
            emit errorOccurred(QString("Write failed: %1").arg(e.what()));
        }
    }

signals:

    void connectionChanged();

    void slaveAddressChanged();

    void registerAddressWidthChanged();

    void errorOccurred(const QString &msg);

    void logMessage(const QString &msg);

private:
    std::unique_ptr<Ft4222> m_ft4222;
    bool m_connected;
    QString m_slaveAddress;
    int m_registerAddressWidth;
};
