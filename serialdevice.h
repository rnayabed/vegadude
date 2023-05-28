/*
 * Copyright (C) 2023 Debayan Sutradhar (rnayabed) (debayansutradhar3@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef SERIALDEVICE_H
#define SERIALDEVICE_H

#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstring>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <chrono>
#include <thread>
#include <vector>
#include <array>
#include <cmath>

#include "device.h"

class SerialDevice : public Device
{
public:

    enum Error
    {
        NONE,
        FAILED_TO_OPEN_DEVICE,
        FAILED_TO_GET_FD_ATTRS,
        FAILED_TO_SET_FD_ATTRS,
        NOT_SUPPORTED,
        READ_FAILED,
        WRITE_FAILED,
        DEVICE_NOT_OPEN
    };

    struct DeviceProperties
    {
        bool parity = false;
        int32_t stopBits = -1;
        bool rtsCts = false;
        int32_t bits = -1;
        int32_t baud = -1;
    };

    constexpr static DeviceProperties ARIES{false, 1, false, 8, 115200};

    SerialDevice(const std::filesystem::path& devicePath,
                 const int32_t& readTimeout,
                 const DeviceProperties& deviceProperties);

    const Error& error();
    std::string errorStr();

    bool read(unsigned char* bytes, size_t size);
    bool write(const unsigned char* bytes, size_t size);

    bool open();
    bool close();
    const int& linuxFD();

private:
    Error m_error;

    bool openLinux();
    bool closeLinux();

    int m_linuxFD;
    int m_readTimeout;
    const std::filesystem::path& m_devicePath;
    const DeviceProperties& m_deviceProperties;
};

#endif // SERIALDEVICE_H
