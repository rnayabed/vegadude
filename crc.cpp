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

uint16_t CRC::generateCRC16CCIT(std::vector<unsigned char> bytes)
{
    uint16_t final = 0;
    for (std::size_t i = 0; i < bytes.size(); i++)
    {
        // we xor the MSB of result (u16) with the byte -> we get the index (byte value) to fetch.
        // then we xor the whole result with the existing result
        final = (final << 8) ^ CRC::CRC16CCITTable[(((final >> 8) ^ bytes[i]))];
        //final = final ^ crc_table[bytes[i]];
    }

    return final;
}
