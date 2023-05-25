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

XModem::XModem(std::unique_ptr<SerialDevice> serialDevice, const size_t &blockSize)
    : m_serialDevice{std::move(serialDevice)}, m_blockSize{blockSize}
{}

XModem::Response XModem::upload(const std::string &filePath, const bool& startAfterUpload)
{
    unsigned char blockNumber1 = 1;
    unsigned char blockNumber2;
    unsigned char crcBlock[2];

    size_t g = 0;

    std::ifstream file{filePath};

    if(!file.is_open())
    {
        return XModem::Response::ERROR_FILE_OPEN_FAILED;
    }

    std::vector<unsigned char> fileBlocks(m_blockSize, 0);

    while(true)
    {
        unsigned char rb;

        if (m_serialDevice->read(&rb) !=
                SerialDevice::Response::SUCCESS)
        {
            return XModem::Response::ERROR_READ_FAILED;
        }

        std::clog << "RECEIVED " << +rb << std::endl;

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
                return XModem::Response::CANCELLED;
            }
            else
            {
                continue;
            }

            if (file.eof())
            {
                m_serialDevice->write(&EOT);

                if (startAfterUpload)
                    m_serialDevice->write(&CR);

                return XModem::Response::SUCCESS;
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

        m_serialDevice->write(&XModem::SOH);
        m_serialDevice->write(&blockNumber1);

        blockNumber2 = 255 - blockNumber1;
        m_serialDevice->write(&blockNumber2);

        m_serialDevice->write(fileBlocks);

        uint16_t crc = CRC::generateCRC16CCIT(fileBlocks);
        crcBlock[0] = crc >> 8;
        crcBlock[1] = crc;
        m_serialDevice->write(crcBlock, 2);

        std::clog << "CRC " << crc << "; Sent block " << g << std::endl << std::endl;
    }

    m_serialDevice->close();

    return XModem::SUCCESS;
}



