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
#include <cmath>

#include "logger.h"

XModem::XModem(Device& device, const int32_t& maxRetry, const int32_t &blockSize)
    : m_error{Error::NONE},
      m_device{device},
      m_maxRetry{maxRetry},
      m_blockSize{blockSize}
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
    case FILE_DOES_NOT_EXIST:
        return "File does not exist";
    case FILE_OPEN_FAILED:
        return "Failed to open file";
    case MAX_RETRY_SURPASSED:
        return "Max retries surpassed";
    case CANCELLED:
        return "Operation cancelled";
    }

    return "Unknown error " + std::to_string(m_error);
}

bool XModem::upload(const std::filesystem::path &filePath, const bool& startAfterUpload)
{
    unsigned char blockNumber1;
    unsigned char blockNumber2;
    unsigned char crcBlock[2];

    size_t currentBlock;
    int32_t currentTry = 0;

    if (!std::filesystem::exists(filePath))
    {
        m_error = Error::FILE_DOES_NOT_EXIST;
        return false;
    }

    size_t fileSize = std::filesystem::file_size(filePath);
    size_t noOfBlocks = std::ceil(float(fileSize) / float(m_blockSize));

    std::ifstream file{filePath, std::ios_base::in | std::ios_base::binary};

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

        if (rb != XModem::NAK)
        {
            if (rb == XModem::C)
            {
                blockNumber1 = 0;
                currentBlock = 1;
                currentTry = 0;
                file.seekg(0);
            }
            else if (rb == XModem::ACK)
            {

                if (blockNumber1 == 255)
                    blockNumber1 = 0;
                else
                    blockNumber1++;

                currentTry = 0;

                currentBlock++;
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
                if (!m_device.write(&EOT)) break;

                if (startAfterUpload)
                    if (!m_device.write(&CR)) break;

                Logger::get() << Logger::NewLine;
                file.close();
                m_error = Error::NONE;
                return true;
            }
            else
            {
                file.read(reinterpret_cast<char*>(fileBlocks.data()), m_blockSize);

                if (file.gcount() != m_blockSize)
                {
                    for (int32_t i = file.gcount(); i < m_blockSize; i++)
                        fileBlocks[i] = XModem::SUB;
                }
            }
        }

        if (currentTry >= m_maxRetry)
        {
            m_error = Error::MAX_RETRY_SURPASSED;
            return false;
        }

        currentTry++;

        blockNumber2 = 255 - blockNumber1;
        uint16_t crc = CRC::generateCRC16CCITT(fileBlocks);
        crcBlock[0] = crc >> 8;
        crcBlock[1] = crc;

        if (!m_device.write(&XModem::SOH) ||
                !m_device.write(&blockNumber1) ||
                !m_device.write(&blockNumber2) ||
                !m_device.write(fileBlocks) ||
                !m_device.write(std::span(crcBlock, 2))) break;

        Logger::get().showProgress("Sent block " + std::to_string(currentBlock) + "/" + std::to_string(noOfBlocks),
                                   (float(currentBlock)/float(noOfBlocks)));
    }

    m_error = Error::DEVICE_RELATED;
    return false;
}



