#ifndef SERIALDEVICE_H
#define SERIALDEVICE_H

#include "targetproperties.h"
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

#include <array>

#include <cmath>

class SerialDevice
{
public:

    enum Response
    {
        SUCCESS,
        ERROR_FAILED_TO_OPEN_DEVICE,
        ERROR_FAILED_TO_GET_FD_ATTRS,
        ERROR_FAILED_TO_SET_FD_ATTRS,
        NOT_SUPPORTED,
        ERROR_READ_FAILED,
        ERROR_WRITE_FAILED,
        ERROR_DEVICE_NOT_OPEN
    };

    SerialDevice(std::shared_ptr<TargetProperties> targetProperties);
    Response open();
    Response close();
    const int& linuxFD();
    Response read(void* byte);
    Response read(void* bytes, size_t bytesLen);
    Response write(const void* byte);
    Response write(const void* bytes, size_t bytesLen);
    template <typename T> Response write(std::vector<T>& vector);

private:
    Response openLinux();
    Response closeLinux();

    int m_linuxFD;
    std::shared_ptr<TargetProperties> m_targetProperties;
};

#endif // SERIALDEVICE_H
