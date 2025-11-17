#include "ft4222.h"

#include <iostream>
#include <limits>
#include <cstring>

Ft4222::Ft4222(FT4222_ClockRate clock)
    : m_version{0, 0},
      m_clock(clock),
      m_devices{},
      m_activeIndex((std::numeric_limits<std::size_t>::max)()),
      m_hasActiveDevice(false),
      m_registerAddressWidth(1) {
    std::cout << "Ft4222 created, waiting for device initialization..." << std::endl;
}

Ft4222::~Ft4222() {
    try {
        shutdown();
    } catch (...) {
        // Деструктор не должен выбрасывать исключения
    }
}

DWORD Ft4222::listFtDevices() {
    FT_STATUS ftStatus = FT_OK;
    DWORD numDevices = 0;

    shutdown();
    ftStatus = FT_CreateDeviceInfoList(&numDevices);
    checkStatus(ftStatus, "Failed to create device info list!");

    std::cout << "Found " << numDevices << " FTDI devices." << std::endl;

    m_devices.clear();
    m_devices.reserve(numDevices);

    for (DWORD i = 0; i < numDevices; ++i) {
        ft_device_t dev{};
        std::memset(&dev.info, 0, sizeof(dev.info));
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
            dev.info.ftHandle = nullptr;
            m_devices.push_back(dev);

            std::ostringstream oss;
            oss << "Device " << i << ":\n"
                << "  Flags: 0x" << std::hex << std::setw(8) << std::setfill('0') << dev.info.Flags << "\n"
                << "  Type: 0x" << std::hex << std::setw(8) << std::setfill('0') << dev.info.Type << "\n"
                << "  ID: 0x" << std::hex << std::setw(8) << std::setfill('0') << dev.info.ID << "\n"
                << "  LocId: 0x" << std::hex << std::setw(8) << std::setfill('0') << dev.info.LocId << "\n"
                << "  SerialNumber: " << dev.info.SerialNumber << "\n"
                << "  Description: " << dev.info.Description << "\n";

            std::cout << oss.str();
        } else {
            std::cerr << "Failed to get device info for device " << i << "!" << std::endl;
        }
    }
    return numDevices;
}

FT4222_STATUS Ft4222::setClock(FT4222_ClockRate clock) {
    if (!m_hasActiveDevice) {
        throw std::runtime_error("No initialized FT4222 device to set clock on!");
    }
    m_clock = clock;
    return FT4222_SetClock(activeHandle(), clock);
}

FT4222_STATUS Ft4222::getClock() {
    if (!m_hasActiveDevice) {
        throw std::runtime_error("No initialized FT4222 device to query clock from!");
    }
    return FT4222_GetClock(activeHandle(), &m_clock);
}

void Ft4222::initializeDevice(std::size_t index, uint32_t clockRate) {
    if (index >= m_devices.size()) {
        throw std::out_of_range("Invalid device index");
    }

    auto &device = m_devices[index];
    if (device.isInitialized) {
        throw std::runtime_error("Device is already initialized!");
    }

    FT_HANDLE ftHandle = nullptr;
    const auto locationId = device.info.LocId;

    FT_STATUS ftStatus = FT_OpenEx(reinterpret_cast<PVOID>(static_cast<uintptr_t>(locationId)),
                                   FT_OPEN_BY_LOCATION,
                                   &ftHandle);
    checkStatus(ftStatus, "Failed to open FT4222 device!");

    if (ftHandle == nullptr) {
        throw std::runtime_error("Failed to open device: ftHandle is NULL!");
    }

    device.info.ftHandle = ftHandle;

    FT4222_STATUS ft4222Status = FT4222_I2CMaster_Init(ftHandle, clockRate);
    checkStatus(ft4222Status, "Failed to initialize I2C master!");

    ft4222Status = FT4222_GetVersion(ftHandle, &m_version);
    checkStatus(ft4222Status, "Failed to get FT4222 version!");

    device.isInitialized = true;
    m_activeIndex = index;
    m_hasActiveDevice = true;
}

void Ft4222::shutdownDevice(std::size_t index) {
    if (index >= m_devices.size()) {
        return;
    }

    auto &device = m_devices[index];
    if (device.isInitialized && device.info.ftHandle != nullptr) {
        if (FT4222_UnInitialize(device.info.ftHandle) != FT4222_OK) {
            std::cerr << "Failed to uninitialize FT4222 device at index " << index << std::endl;
        }
        if (FT_Close(device.info.ftHandle) != FT_OK) {
            std::cerr << "Failed to close FT4222 device at index " << index << std::endl;
        }
    }

    device.info.ftHandle = nullptr;
    device.isInitialized = false;

    if (m_hasActiveDevice && m_activeIndex == index) {
        m_hasActiveDevice = false;
        m_activeIndex = (std::numeric_limits<std::size_t>::max)();
    }
}

void Ft4222::shutdown() {
    for (std::size_t i = 0; i < m_devices.size(); ++i) {
        shutdownDevice(i);
    }
}

std::size_t Ft4222::activeDeviceIndex() const {
    if (!m_hasActiveDevice) {
        throw std::runtime_error("No active device");
    }
    return m_activeIndex;
}

void Ft4222::setRegisterAddressWidth(uint8_t width) {
    if (width == 0 || width > 2) {
        throw std::invalid_argument("Register address width must be 1 or 2 bytes");
    }
    m_registerAddressWidth = width;
}

FT_HANDLE Ft4222::activeHandle() const {
    if (!m_hasActiveDevice) {
        throw std::runtime_error("No active FT4222 handle");
    }
    return m_devices.at(m_activeIndex).info.ftHandle;
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

const FT_DEVICE_LIST_INFO_NODE &Ft4222::getDeviceInfo(std::size_t index) const {
    if (index >= m_devices.size()) {
        throw std::out_of_range("Invalid device index");
    }
    return m_devices[index].info;
}

FT4222_STATUS Ft4222::i2cMemWrite(uint16_t devAddress,
                                  uint16_t memAddress,
                                  const uint8_t *pData,
                                  uint16_t size,
                                  uint8_t addressWidth) {
    if (!m_hasActiveDevice) {
        return FT4222_DEVICE_NOT_FOUND;
    }

    const uint8_t addrWidth = addressWidth == 0 ? m_registerAddressWidth : addressWidth;
    if (addrWidth == 0 || addrWidth > 2) {
        return FT4222_INVALID_ARGS;
    }

    std::vector<uint8_t> buffer;
    buffer.reserve(addrWidth + size);

    for (int shift = static_cast<int>(addrWidth); shift > 0; --shift) {
        const uint8_t byte = static_cast<uint8_t>((memAddress >> ((shift - 1) * 8)) & 0xFF);
        buffer.push_back(byte);
    }

    buffer.insert(buffer.end(), pData, pData + size);

    uint16_t sizeTransferred = 0;
    const auto handle = activeHandle();
    FT4222_STATUS ftStatus = FT4222_I2CMaster_Write(handle,
                                                    devAddress,
                                                    buffer.data(),
                                                    static_cast<uint16_t>(buffer.size()),
                                                    &sizeTransferred);

    if (ftStatus != FT4222_OK) {
        return ftStatus;
    }

    if (sizeTransferred != buffer.size()) {
        return FT4222_IO_ERROR;
    }

    return ftStatus;
}

FT4222_STATUS Ft4222::i2cMemRead(uint16_t devAddress,
                                 uint16_t memAddress,
                                 uint8_t *pData,
                                 uint16_t size,
                                 uint8_t addressWidth) {
    if (!m_hasActiveDevice) {
        return FT4222_DEVICE_NOT_FOUND;
    }

    const uint8_t addrWidth = addressWidth == 0 ? m_registerAddressWidth : addressWidth;
    if (addrWidth == 0 || addrWidth > 2) {
        return FT4222_INVALID_ARGS;
    }

    std::vector<uint8_t> addressBytes;
    addressBytes.reserve(addrWidth);
    for (int shift = static_cast<int>(addrWidth); shift > 0; --shift) {
        const uint8_t byte = static_cast<uint8_t>((memAddress >> ((shift - 1) * 8)) & 0xFF);
        addressBytes.push_back(byte);
    }

    uint16_t transferred = 0;
    const auto handle = activeHandle();

    FT4222_STATUS ftStatus = FT4222_I2CMaster_WriteEx(handle,
                                                      devAddress,
                                                      0,
                                                      addressBytes.data(),
                                                      static_cast<uint16_t>(addressBytes.size()),
                                                      &transferred);
    if (ftStatus != FT4222_OK) {
        return ftStatus;
    }

    if (transferred != addressBytes.size()) {
        return FT4222_IO_ERROR;
    }

    ftStatus = FT4222_I2CMaster_ReadEx(handle,
                                       devAddress,
                                       0,
                                       pData,
                                       size,
                                       &transferred);

    if (ftStatus != FT4222_OK) {
        return ftStatus;
    }

    if (transferred != size) {
        return FT4222_IO_ERROR;
    }

    return ftStatus;
}