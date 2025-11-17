#include "SerialManager.h"

namespace {
constexpr int MAX_BUFFER_SIZE = 4096;
}

SerialManager::SerialManager(QObject *parent) : QObject(parent) {
    connect(&m_serial, &QSerialPort::readyRead, this, &SerialManager::handleReadyRead);
    connect(&m_timer, &QTimer::timeout, this, &SerialManager::updatePorts);
    connect(&m_serial,
            &QSerialPort::errorOccurred,
            this,
            [this](QSerialPort::SerialPortError error) {
                if (error == QSerialPort::NoError) {
                    return;
                }
                emit errorOccurred(m_serial.errorString());
                if (m_serial.isOpen() && error == QSerialPort::ResourceError) {
                    closePort();
                }
            });
    m_timer.start(1000);
    updatePorts();
}

void SerialManager::setPortName(const QString &name) {
    if (m_portName != name) {
        m_portName = name;
        emit portNameChanged();
    }
}

void SerialManager::updatePorts() {
    QStringList currentPorts;
    for (const QSerialPortInfo &info: QSerialPortInfo::availablePorts())
        currentPorts << info.portName();

    if (currentPorts != m_ports) {
        m_ports = currentPorts;
        emit portsChanged();
    }
}

void SerialManager::openPort() {
    if (m_serial.isOpen())
        m_serial.close();

    m_serial.setPortName(m_portName);
    m_serial.setBaudRate(QSerialPort::Baud115200);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::NoFlowControl);

    if (m_serial.open(QIODevice::ReadWrite)) {
        emit isOpenChanged();
    } else {
        emit errorOccurred("Cannot open port: " + m_serial.errorString());
    }
}

void SerialManager::closePort() {
    if (m_serial.isOpen()) {
        m_serial.close();
        emit isOpenChanged();
    }
}

void SerialManager::sendData(const QString &data) {
    if (m_serial.isOpen()) {
        const QByteArray payload = data.toUtf8();
        const qint64 bytesWritten = m_serial.write(payload);
        if (bytesWritten == -1) {
            emit errorOccurred("Failed to write data: " + m_serial.errorString());
        }
    } else {
        emit errorOccurred("Port is not open");
    }
}

void SerialManager::handleReadyRead() {
    const QByteArray data = m_serial.readAll();
    if (data.isEmpty()) {
        return;
    }

    m_receivedData += QString::fromUtf8(data);
    if (m_receivedData.size() > MAX_BUFFER_SIZE) {
        m_receivedData = m_receivedData.right(MAX_BUFFER_SIZE);
    }
    emit receivedDataChanged();
}