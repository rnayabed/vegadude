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

#include "crc.h"

namespace CRC
{

uint16_t generateCRC16CCITT(std::span<const unsigned char> bytes)
{
    uint16_t result = 0;
    for (auto&& byte : bytes)
    {
        // we xor the MSB of result (u16) with the byte -> we get the index (byte value) to fetch.
        // then we xor the whole result with the existing result
        result = (result << 8) ^ CRC::CRC16CCITTable[(((result >> 8) ^ byte))];
    }

    return result;
}

}
