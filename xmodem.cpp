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

#include "xmodem.h"
#include "crc.h"

#include <fstream>
#include <istream>
#include <iostream>
#include <format>

#include "logger.h"

XModem::XModem(Device& device, const size_t &blockSize)
    : m_device{device},
      m_blockSize{blockSize},
      m_error{Error::NONE}
{}

const XModem::Error &XModem::error()
{
    return m_error;
}

std::string XModem::errorStr()
{
    switch (m_error)
    {
    case NONE:
        return "None";
    case DEVICE_RELATED:
        return "Device related error";
    case FILE_OPEN_FAILED:
        return "Failed to open file";
    case CANCELLED:
        return "Operation cancelled";
    }

    return std::format("Unknown error {}", static_cast<int>(m_error));
}

bool XModem::upload(const std::filesystem::path &filePath, const bool& startAfterUpload)
{
    unsigned char blockNumber1 = 1;
    unsigned char blockNumber2;
    unsigned char crcBlock[2];

    size_t g = 0;

    std::ifstream file{filePath};

    if(!file.is_open())
    {
        m_error = Error::FILE_OPEN_FAILED;
        return false;
    }

    std::vector<unsigned char> fileBlocks(m_blockSize, 0);

    while(true)
    {
        unsigned char rb;

        if (!m_device.read(&rb))
        {
            m_error = Error::DEVICE_RELATED;
            return false;
        }

        Logger::get() << "READ: " << +rb << Logger::NewLine;

        if (rb != XModem::NAK)
        {
            if (rb == XModem::C)
            {
                blockNumber1 = 0;
                file.seekg(0);
            }
            else if (rb == XModem::ACK)
            {
                if (blockNumber1 == 255)
                    blockNumber1 = 0;
                else
                    blockNumber1++;

                g++;
            }
            else if (rb == XModem::CAN)
            {
                m_error = Error::CANCELLED;
                return false;
            }
            else
            {
                continue;
            }

            if (file.eof())
            {
                m_device.write(&EOT, 1);

                if (startAfterUpload)
                    m_device.write(&CR, 1);

                m_error = Error::NONE;
                return true;
            }
            else
            {
                file.read(reinterpret_cast<char*>(fileBlocks.data()), m_blockSize);

                if (file.gcount() != m_blockSize)
                {
                    for (size_t i = file.gcount(); i < m_blockSize; i++)
                        fileBlocks[i] = XModem::SUB;
                }
            }
        }

        m_device.write(&XModem::SOH);
        m_device.write(&blockNumber1);

        blockNumber2 = 255 - blockNumber1;
        m_device.write(&blockNumber2);

        m_device.write(fileBlocks.data(), fileBlocks.size());

        uint16_t crc = CRC::generateCRC16CCITT(fileBlocks);
        crcBlock[0] = crc >> 8;
        crcBlock[1] = crc;
        m_device.write(crcBlock, 2);

        Logger::get() << "Sent block " << g << Logger::NewLine;
    }

    file.close();

    m_error = Error::NONE;
    return true;
}



