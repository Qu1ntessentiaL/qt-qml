#include "ft4222_wrapper.h"

DWORD Ft4222Wrapper::scanDevices() {
    if (!m_ft4222) {
        emit errorOccurred("FT4222 device is not connected");
        return 0;
    }

    DWORD numDevices = 0;
    bool wasConnected = false;
    std::size_t activeIndex = 0;

    try {
        // Сохраняем состояние ДО сканирования
        wasConnected = m_ft4222->hasActiveDevice();
        if (wasConnected) {
            activeIndex = m_ft4222->activeDeviceIndex();
        }

        // Сканируем устройства
        numDevices = m_ft4222->listFtDevices();
        emit logMessage(QString("Found %1 FTDI devices.").arg(numDevices));

        // Безопасно получаем список устройств
        const auto &devices = m_ft4222->devices();

        // Проверяем, что список не пустой
        if (devices.empty()) {
            emit logMessage("No devices found in device list.");
        } else {
            for (std::size_t i = 0; i < devices.size() && i < static_cast<std::size_t>(numDevices); ++i) {
                const auto &dev = devices.at(i);

                // Безопасное создание строк (проверка на nullptr)
                QString serialNumber = dev.info.SerialNumber ?
                                       QString::fromLatin1(dev.info.SerialNumber) : QString("(null)");
                QString description = dev.info.Description ?
                                      QString::fromLatin1(dev.info.Description) : QString("(null)");

                QString info = QStringLiteral(
                        "Device %1:\n"
                        "  Flags: 0x%2\n"
                        "  Type: 0x%3\n"
                        "  ID: 0x%4\n"
                        "  LocId: 0x%5\n"
                        "  SerialNumber: %6\n"
                        "  Description: %7")
                        .arg(i)
                        .arg(dev.info.Flags, 8, 16, QLatin1Char('0'))
                        .arg(dev.info.Type, 8, 16, QLatin1Char('0'))
                        .arg(dev.info.ID, 8, 16, QLatin1Char('0'))
                        .arg(dev.info.LocId, 4, 16, QLatin1Char('0')) // LocId обычно 2 байта
                        .arg(serialNumber)
                        .arg(description);
                emit logMessage(info);
            }
        }

        // Восстанавливаем соединение если нужно
        if (wasConnected) {
            if (activeIndex < devices.size()) {
                try {
                    m_ft4222->initializeDevice(activeIndex);
                    if (m_registerAddressWidth > 0) {
                        m_ft4222->setRegisterAddressWidth(static_cast<uint8_t>(m_registerAddressWidth));
                    }
                    m_connected = true;
                    emit connectionChanged();
                } catch (const std::exception &e) {
                    m_connected = false;
                    emit connectionChanged();
                    emit errorOccurred(QString("Failed to reinitialize device: %1").arg(e.what()));
                }
            } else {
                // Индекс устройства стал невалидным после сканирования
                m_connected = false;
                emit connectionChanged();
                emit errorOccurred("Previously connected device is no longer available");
            }
        }

    } catch (const std::exception &e) {
        emit errorOccurred(QString("Device scan failed: %1").arg(e.what()));
        // В случае ошибки сбрасываем состояние соединения
        if (wasConnected) {
            m_connected = false;
            emit connectionChanged();
        }
    }

    return numDevices;
}
