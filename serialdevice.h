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
#include <errno.h>
#include <chrono>
#include <thread>
#include <vector>
#include <array>
#include <cmath>
#include "device.h"

#ifdef __linux
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#elif __WIN32
#define UNICODE
#include <Windows.h>
#include <stdio.h>
#include <string.h>
#endif

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
                 const DeviceProperties& deviceProperties,
                 const int32_t& readTimeout);

    const Error& error();
    std::string errorStr();

    bool read(std::span<unsigned char> bytes);
    bool write(std::span<const unsigned char> bytes);

    bool open();
    bool close();

private:
    Error m_error;
    const std::filesystem::path& m_devicePath;
    const DeviceProperties& m_deviceProperties;
    int32_t m_readTimeout;

#ifdef __linux
    int32_t m_linuxFD = -1;
#elif __WIN32
    HANDLE m_winHandle = NULL;
#endif

    bool openLinux();
    bool closeLinux();

};

#endif // SERIALDEVICE_H
