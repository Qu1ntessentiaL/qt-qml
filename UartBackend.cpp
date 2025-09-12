#include "UartBackend.h"

UartBackend::UartBackend(QObject *parent) : QObject(parent) {
    connect(&m_serial, &QSerialPort::readyRead, this, &UartBackend::handleReadyRead);
}

QStringList UartBackend::refreshPorts() {
    QStringList ports;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const auto &info: infos)
        ports.append(info.portName());
    return ports;
}

bool UartBackend::connectPort(const QString &portName, int baudRate) {
    if (m_isConnected)
        return true;

    m_serial.setPortName(portName);
    m_serial.setBaudRate(baudRate);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial.open(QIODevice::ReadOnly))
        return false;

    m_isConnected = true;
    return true;
}

void UartBackend::disconnectPort() {
    if (!m_isConnected)
        return;
    m_serial.close();
    m_isConnected = false;
}

void UartBackend::clearData() {
    m_buffer.clear();
}

void UartBackend::handleReadyRead() {}