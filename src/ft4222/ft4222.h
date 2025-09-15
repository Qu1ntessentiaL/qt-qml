#pragma once

#include <windows.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include "ftd2xx.h"
#include "LibFT4222.h"

class Ft4222 {
public:
    typedef struct ft_device {
        FT_DEVICE_LIST_INFO_NODE info;
        bool isInitialized;
    } ft_device_t;
private:
    DWORD m_devCnt;
    FT4222_Version m_version;
    FT4222_ClockRate m_clock;
    size_t m_index;

    inline std::vector<ft_device>::iterator findDevice() {
        return std::find_if(m_devices.begin(), m_devices.end(), [](const ft_device &device) {
            return device.info.SerialNumber[0] == 'A';
        });
    }

public:
    std::vector<ft_device> m_devices;

    explicit Ft4222(FT4222_ClockRate clock = SYS_CLK_60);

    ~Ft4222();

    DWORD listFtDevices();

    FT4222_STATUS setClock(FT4222_ClockRate clock);

    FT4222_STATUS getClock();

    static void checkStatus(FT_STATUS status, const std::string &errorMessage);

    static void checkStatus(FT4222_STATUS status, const std::string &errorMessage);

    void i2cMasterInit(uint32_t clockRate = 100);

    FT4222_STATUS
    i2cMemWrite(uint16_t devAddress, uint16_t memAddress, const uint8_t *pData, uint16_t size);

    FT4222_STATUS
    i2cMemRead(uint16_t devAddress, uint16_t memAddress, uint8_t *pData, uint16_t size);

    const FT_DEVICE_LIST_INFO_NODE &getDeviceInfo(DWORD index) const {
        if (index >= m_devices.size()) throw std::out_of_range("Invalid device index");
        return m_devices[index].info;
    }
};
