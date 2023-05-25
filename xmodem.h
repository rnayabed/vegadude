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

#ifndef XMODEM_H
#define XMODEM_H

#include "serialdevice.h"
#include <vector>

class XModem
{
public:

    enum Response
    {
        SUCCESS,
        ERROR_READ_FAILED,
        ERROR_FILE_OPEN_FAILED,
        CANCELLED
    };

    XModem(std::unique_ptr<SerialDevice> serialDevice,
           const size_t& blockSize);

    Response upload(const std::string& filePath, const bool& startAfterUpload);

private:
    std::unique_ptr<SerialDevice> m_serialDevice;
    size_t m_blockSize;

    constexpr static unsigned char SOH   {0x01};
    constexpr static unsigned char EOT   {0x04};
    constexpr static unsigned char ACK   {0x06};
    constexpr static unsigned char NAK   {0x15};
    constexpr static unsigned char CAN   {0x18};
    constexpr static unsigned char SUB   {0x1a};
    constexpr static unsigned char CR    {0x1d};
    constexpr static unsigned char C     {'C'};
};

#endif // XMODEM_H
