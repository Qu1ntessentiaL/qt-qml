#include "ft4222.h"

using namespace std;

Ft4222::Ft4222(FT4222_ClockRate clock) : m_version{0, 0}, m_clock(clock) {
    m_devCnt = listFtDevices();
    if (m_devCnt > 0) {
        setClock(m_clock);
    } else {
        throw std::runtime_error("No FTDI devices found!");
    }
}

Ft4222::~Ft4222() {
    if (!m_devices.empty()) {
        FT_HANDLE ftHandle = m_devices.front().info.ftHandle;
        bool *isInit = &m_devices.front().isInitialized;
        if (*isInit) {
            FT4222_UnInitialize(ftHandle);
            FT_Close(ftHandle);
        }
    }
}

DWORD Ft4222::listFtDevices() {
    FT_STATUS ftStatus;
    DWORD numDevices = 0;

    ftStatus = FT_CreateDeviceInfoList(&numDevices);
    checkStatus(ftStatus, "Failed to create device info list!");

    std::cout << "Found " << numDevices << " FTDI devices." << std::endl;

    m_devices.clear(); // Очищаем вектор перед добавлением новых устройств

    for (DWORD i = 0; i < numDevices; ++i) {
        ft_device_t dev;
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
            m_devices.push_back(dev); // Добавляем устройство в вектор

            std::ostringstream oss;
            oss << "Device " << i << ":\n"
                << "  Flags: 0x" << std::hex << std::setw(8) << std::setfill('0') << dev.info.Flags << "\n"
                << "  Type: 0x" << std::hex << std::setw(8) << std::setfill('0') << dev.info.Type << "\n"
                << "  ID: 0x" << std::hex << std::setw(8) << std::setfill('0') << dev.info.ID << "\n"
                << "  LocId: 0x" << std::hex << std::setw(8) << std::setfill('0') << dev.info.LocId << "\n"
                << "  SerialNumber: " << dev.info.SerialNumber << "\n"
                << "  Description: " << dev.info.Description << "\n"
                << "  ftHandle: " << dev.info.ftHandle << "\n";

            std::cout << oss.str();
        } else {
            std::cerr << "Failed to get device info for device " << i << "!" << std::endl;
        }
    }
    return numDevices;
}

FT4222_STATUS Ft4222::setClock(FT4222_ClockRate clock) {
    if (m_devices.empty()) {
        throw std::runtime_error("No devices available!");
    }
    return FT4222_SetClock(m_devices.front().info.ftHandle, clock);
}

FT4222_STATUS Ft4222::getClock() {
    if (m_devices.empty()) {
        throw std::runtime_error("No devices available!");
    }
    return FT4222_GetClock(m_devices.front().info.ftHandle, &m_clock);
}

void Ft4222::checkStatus(FT_STATUS status, const std::string &errorMessage) {
    if (status != FT_OK) {
        std::string errorDetail = "Error code: " + std::to_string(status) + ". ";

        // Преобразуем код статуса в текстовое описание
        switch (status) {
            case FT_INVALID_HANDLE:
                errorDetail += "Invalid handle";
                break;
            case FT_DEVICE_NOT_FOUND:
                errorDetail += "Device not found";
                break;
            case FT_DEVICE_NOT_OPENED:
                errorDetail += "Device not opened";
                break;
            case FT_IO_ERROR:
                errorDetail += "I/O error";
                break;
            case FT_INSUFFICIENT_RESOURCES:
                errorDetail += "Insufficient resources";
                break;
            case FT_INVALID_PARAMETER:
                errorDetail += "Invalid parameter";
                break;
            case FT_INVALID_BAUD_RATE:
                errorDetail += "Invalid baud rate";
                break;
            case FT_DEVICE_NOT_OPENED_FOR_ERASE:
                errorDetail += "Device not opened for erase";
                break;
            case FT_DEVICE_NOT_OPENED_FOR_WRITE:
                errorDetail += "Device not opened for write";
                break;
            case FT_FAILED_TO_WRITE_DEVICE:
                errorDetail += "Failed to write to device";
                break;
            case FT_EEPROM_READ_FAILED:
                errorDetail += "EEPROM read failed";
                break;
            case FT_EEPROM_WRITE_FAILED:
                errorDetail += "EEPROM write failed";
                break;
            case FT_EEPROM_ERASE_FAILED:
                errorDetail += "EEPROM erase failed";
                break;
            case FT_EEPROM_NOT_PRESENT:
                errorDetail += "EEPROM not present";
                break;
            case FT_EEPROM_NOT_PROGRAMMED:
                errorDetail += "EEPROM not programmed";
                break;
            case FT_INVALID_ARGS:
                errorDetail += "Invalid arguments";
                break;
            case FT_NOT_SUPPORTED:
                errorDetail += "Not supported";
                break;
            case FT_OTHER_ERROR:
                errorDetail += "Other error";
                break;
            case FT_DEVICE_LIST_NOT_READY:
                errorDetail += "Device list not ready";
                break;
            default:
                errorDetail += "Unknown error";
                break;
        }

        // Генерируем исключение с текстовым описанием ошибки
        throw std::runtime_error(errorMessage + " (" + errorDetail + ")");
    }
}

void Ft4222::checkStatus(FT4222_STATUS status, const std::string &errorMessage) {
    if (status != FT4222_OK) {
        std::string errorDetail = "Error code: " + std::to_string(status) + ". ";

        // Преобразуем код статуса в текстовое описание
        switch (status) {
            case FT4222_INVALID_HANDLE:
                errorDetail += "Invalid handle (FT4222)";
                break;
            case FT4222_DEVICE_NOT_FOUND:
                errorDetail += "Device not found (FT4222)";
                break;
            case FT4222_DEVICE_NOT_OPENED:
                errorDetail += "Device not opened (FT4222)";
                break;
            case FT4222_IO_ERROR:
                errorDetail += "I/O error (FT4222)";
                break;
            case FT4222_INSUFFICIENT_RESOURCES:
                errorDetail += "Insufficient resources (FT4222)";
                break;
            case FT4222_INVALID_PARAMETER:
                errorDetail += "Invalid parameter (FT4222)";
                break;
            case FT4222_INVALID_BAUD_RATE:
                errorDetail += "Invalid baud rate (FT4222)";
                break;
            case FT4222_DEVICE_NOT_OPENED_FOR_ERASE:
                errorDetail += "Device not opened for erase (FT4222)";
                break;
            case FT4222_DEVICE_NOT_OPENED_FOR_WRITE:
                errorDetail += "Device not opened for write (FT4222)";
                break;
            case FT4222_FAILED_TO_WRITE_DEVICE:
                errorDetail += "Failed to write to device (FT4222)";
                break;
            case FT4222_EEPROM_READ_FAILED:
                errorDetail += "EEPROM read failed (FT4222)";
                break;
            case FT4222_EEPROM_WRITE_FAILED:
                errorDetail += "EEPROM write failed (FT4222)";
                break;
            case FT4222_EEPROM_ERASE_FAILED:
                errorDetail += "EEPROM erase failed (FT4222)";
                break;
            case FT4222_EEPROM_NOT_PRESENT:
                errorDetail += "EEPROM not present (FT4222)";
                break;
            case FT4222_EEPROM_NOT_PROGRAMMED:
                errorDetail += "EEPROM not programmed (FT4222)";
                break;
            case FT4222_INVALID_ARGS:
                errorDetail += "Invalid arguments (FT4222)";
                break;
            case FT4222_NOT_SUPPORTED:
                errorDetail += "Not supported (FT4222)";
                break;
            case FT4222_OTHER_ERROR:
                errorDetail += "Other error (FT4222)";
                break;
            case FT4222_DEVICE_LIST_NOT_READY:
                errorDetail += "Device list not ready (FT4222)";
                break;
            default:
                errorDetail += "Unknown error (FT4222)";
                break;
        }

        // Генерируем исключение с текстовым описанием ошибки
        throw std::runtime_error(errorMessage + " (" + errorDetail + ")");
    }
}

void Ft4222::i2cMasterInit(uint32_t clockRate) {
    if (m_devices.empty()) {
        throw std::runtime_error("No devices available!");
    }

    auto it = findDevice();
    if (it != m_devices.end()) {
        m_index = std::distance(m_devices.begin(), it);
        std::cout << "Device found at index: " << m_index << std::endl;
    }

    FT_HANDLE ftHandle = m_devices.at(m_index).info.ftHandle;
    bool *isInit = &m_devices.at(m_index).isInitialized;
    uint32_t locationId = m_devices.at(m_index).info.LocId;

    if (*isInit) {
        throw std::runtime_error("Device is already initialized!");
    }

    FT_STATUS ftStatus = FT_OpenEx(reinterpret_cast<PVOID>(static_cast<uintptr_t>(locationId)),
                                   FT_OPEN_BY_LOCATION,
                                   &ftHandle);
    checkStatus(ftStatus, "Failed to open FT4222 device!");

    if (ftHandle == nullptr) {
        throw std::runtime_error("Failed to open device: ftHandle is NULL!");
    }

    // Обновляем ftHandle в структуре устройства
    m_devices.at(m_index).info.ftHandle = ftHandle;

    FT4222_STATUS ft4222Status = FT4222_I2CMaster_Init(ftHandle, clockRate);
    checkStatus(ft4222Status, "Failed to initialize I2C master!");

    ft4222Status = FT4222_GetVersion(ftHandle, &m_version);
    checkStatus(ft4222Status, "Failed to get FT4222 version!");

    *isInit = true;
}

FT4222_STATUS Ft4222::i2cMemWrite(uint16_t devAddress, uint16_t memAddress, const uint8_t *pData, uint16_t size) {
    if (m_devices.empty()) {
        return FT4222_DEVICE_NOT_FOUND;
    }

    FT_HANDLE ftHandle = m_devices.front().info.ftHandle;
    bool isInit = m_devices.front().isInitialized;

    if (!isInit) {
        return FT4222_DEVICE_NOT_FOUND;
    }

    std::vector<uint8_t> buffer;
    buffer.push_back(static_cast<uint8_t>(memAddress));
    buffer.insert(buffer.end(), pData, pData + size);

    uint16_t sizeTransferred;
    FT4222_STATUS ftStatus = FT4222_I2CMaster_Write(ftHandle,
                                                    devAddress,
                                                    buffer.data(),
                                                    buffer.size(),
                                                    &sizeTransferred);

    if (sizeTransferred != buffer.size()) {
        return FT4222_IO_ERROR;
    }

    return ftStatus;
}

FT4222_STATUS Ft4222::i2cMemRead(uint16_t devAddress, uint16_t memAddress, uint8_t *pData, uint16_t size) {
    if (m_devices.empty()) {
        return FT4222_DEVICE_NOT_FOUND;
    }

    FT_HANDLE ftHandle = m_devices.front().info.ftHandle;
    bool isInit = m_devices.front().isInitialized;

    if (!isInit) {
        return FT4222_DEVICE_NOT_FOUND;
    }

    auto regAddr = static_cast<uint8_t>(memAddress);
    uint16_t sizeTransferred;
    FT4222_STATUS ftStatus = FT4222_I2CMaster_WriteEx(ftHandle,
                                                      devAddress,
                                                      0,
                                                      &regAddr,
                                                      1,
                                                      &sizeTransferred);
    if (ftStatus != FT4222_OK) {
        return ftStatus;
    }

    ftStatus = FT4222_I2CMaster_ReadEx(ftHandle,
                                       devAddress,
                                       0,
                                       pData,
                                       size,
                                       &sizeTransferred);

    if (sizeTransferred != size) {
        return FT4222_IO_ERROR;
    }

    return ftStatus;
}