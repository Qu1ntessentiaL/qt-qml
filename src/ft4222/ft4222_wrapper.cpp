#include "ft4222_wrapper.h"

Q_INVOKABLE DWORD Ft4222Wrapper::scanDevices() {
    if (!m_ft4222) return 0;

    DWORD numDevices = 0;
    try {
        FT_STATUS ftStatus = FT_CreateDeviceInfoList(&numDevices);
        if (ftStatus != FT_OK) {
            emit logMessage("Failed to create device info list!");
            return 0;
        }

        emit logMessage(QString("Found %1 FTDI devices.").arg(numDevices));
        m_ft4222->m_devices.clear();

        for (DWORD i = 0; i < numDevices; ++i) {
            Ft4222::ft_device_t dev;
            memset(&dev.info, 0, sizeof(dev.info));
            dev.isInitialized = false;

            ftStatus = FT_GetDeviceInfoDetail(i,
                                              &dev.info.Flags,
                                              &dev.info.Type,
                                              &dev.info.ID,
                                              &dev.info.LocId,
                                              dev.info.SerialNumber,
                                              dev.info.Description,
                                              &dev.info.ftHandle);
            if (ftStatus == FT_OK) {
                m_ft4222->m_devices.push_back(dev);

                QString info = QString(
                        "Device %1:\n"
                        "  Flags: 0x%2\n"
                        "  Type: 0x%3\n"
                        "  ID: 0x%4\n"
                        "  LocId: 0x%5\n"
                        "  SerialNumber: %6\n"
                        "  Description: %7\n"
                        "  ftHandle: %8"
                ).arg(i)
                        .arg(dev.info.Flags, 8, 16, QLatin1Char('0'))
                        .arg(dev.info.Type, 8, 16, QLatin1Char('0'))
                        .arg(dev.info.ID, 8, 16, QLatin1Char('0'))
                        .arg(dev.info.LocId, 8, 16, QLatin1Char('0'))
                        .arg(QString::fromLatin1(dev.info.SerialNumber))
                        .arg(QString::fromLatin1(dev.info.Description))
                        .arg(reinterpret_cast<quintptr>(dev.info.ftHandle));

                emit logMessage(info);
            } else {
                emit logMessage(QString("Failed to get device info for device %1").arg(i));
            }
        }
    } catch (const std::exception &e) {
        emit logMessage(QString("Exception: %1").arg(e.what()));
    }

    return numDevices;
}
