#pragma once

#include <QObject>
#include <QDebug>

#include "ft4222.h"

class Ft4222Wrapper : public QObject {
Q_OBJECT
    Q_PROPERTY(QString slaveAddress READ slaveAddress WRITE setSlaveAddress NOTIFY slaveAddressChanged)

public:
    explicit Ft4222Wrapper(QObject *parent = nullptr)
            : QObject(parent), m_ft4222(SYS_CLK_60) {}

    QString slaveAddress() const { return m_slaveAddress; }

    void setSlaveAddress(const QString &addr) {
        if (m_slaveAddress == addr) return;
        m_slaveAddress = addr;
        emit slaveAddressChanged();
    }

    Q_INVOKABLE void writeMem(int devAddr, int memAddr, QByteArray data) {
        m_ft4222.i2cMemWrite(devAddr, memAddr,
                             reinterpret_cast<const uint8_t *>(data.constData()),
                             data.size());
    }

    Q_INVOKABLE QByteArray readMem(int devAddr, int memAddr, int size) {
        QByteArray buffer(size, Qt::Uninitialized);
        m_ft4222.i2cMemRead(devAddr, memAddr,
                            reinterpret_cast<uint8_t *>(buffer.data()), size);
        return buffer;
    }

signals:

    void slaveAddressChanged();

private:
    Ft4222 m_ft4222;
    QString m_slaveAddress;
};
