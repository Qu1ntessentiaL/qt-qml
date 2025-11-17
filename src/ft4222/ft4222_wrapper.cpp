#include "ft4222_wrapper.h"

#include <cstddef>
#include <cstdint>

DWORD Ft4222Wrapper::scanDevices() {
    if (!m_ft4222) {
        emit errorOccurred("FT4222 device is not connected");
        return 0;
    }

    DWORD numDevices = 0;
    try {
        bool wasConnected = m_ft4222->hasActiveDevice();
        std::size_t activeIndex = wasConnected ? m_ft4222->activeDeviceIndex() : 0;

        numDevices = m_ft4222->listFtDevices();
        emit logMessage(QString("Found %1 FTDI devices.").arg(numDevices));

        const auto &devices = m_ft4222->devices();
        for (std::size_t i = 0; i < devices.size(); ++i) {
            const auto &dev = devices.at(i);
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
                               .arg(dev.info.LocId, 8, 16, QLatin1Char('0'))
                               .arg(QString::fromLatin1(dev.info.SerialNumber))
                               .arg(QString::fromLatin1(dev.info.Description));
            emit logMessage(info);
        }

        if (wasConnected && activeIndex < devices.size()) {
            m_ft4222->initializeDevice(activeIndex);
            m_ft4222->setRegisterAddressWidth(static_cast<uint8_t>(m_registerAddressWidth));
            m_connected = true;
        } else if (wasConnected) {
            m_connected = false;
            emit connectionChanged();
        }

    } catch (const std::exception &e) {
        emit errorOccurred(QString("Device scan failed: %1").arg(e.what()));
    }

    return numDevices;
}
