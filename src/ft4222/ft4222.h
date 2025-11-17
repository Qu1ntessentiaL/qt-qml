#pragma once

#include <windows.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstddef>

#include "ftd2xx.h"
#include "LibFT4222.h"

class Ft4222 {
public:
    struct ft_device_t {
        FT_DEVICE_LIST_INFO_NODE info;
        bool isInitialized;
    };

    using DeviceList = std::vector<ft_device_t>;

    explicit Ft4222(FT4222_ClockRate clock = SYS_CLK_60);

    ~Ft4222();

    DWORD listFtDevices();

    FT4222_STATUS setClock(FT4222_ClockRate clock);

    FT4222_STATUS getClock();

    void initializeDevice(std::size_t index, uint32_t clockRate = 100);

    void shutdownDevice(std::size_t index);

    void shutdown();

    const DeviceList &devices() const { return m_devices; }

    bool hasActiveDevice() const { return m_hasActiveDevice; }

    std::size_t activeDeviceIndex() const;

    void setRegisterAddressWidth(uint8_t width);

    uint8_t registerAddressWidth() const { return m_registerAddressWidth; }

    static void checkStatus(FT_STATUS status, const std::string &errorMessage);

    static void checkStatus(FT4222_STATUS status, const std::string &errorMessage);

    FT4222_STATUS
    i2cMemWrite(uint16_t devAddress,
                uint16_t memAddress,
                const uint8_t *pData,
                uint16_t size,
                uint8_t addressWidth = 0);

    FT4222_STATUS
    i2cMemRead(uint16_t devAddress,
               uint16_t memAddress,
               uint8_t *pData,
               uint16_t size,
               uint8_t addressWidth = 0);

    const FT_DEVICE_LIST_INFO_NODE &getDeviceInfo(std::size_t index) const;

private:
    FT4222_Version m_version;
    FT4222_ClockRate m_clock;
    DeviceList m_devices;
    std::size_t m_activeIndex;
    bool m_hasActiveDevice;
    uint8_t m_registerAddressWidth;

    FT_HANDLE activeHandle() const;
};
