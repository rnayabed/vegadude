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

#include "serialdevice.h"
#include "xmodem.h"

enum ArgType
{
    BINARY_PATH,
    TARGET_PATH,
    SERIAL_PARITY_YES,
    SERIAL_STOP_BITS,
    SERIAL_RTS_CTS_YES,
    SERIAL_BITS,
    SERIAL_BAUD_RATE,
    START_AFTER_UPLOAD,
    INVALID
};

ArgType getArgType(char* const& arg)
{
    using namespace std;

    if (!string(arg).compare("-bp") ||
        !string(arg).compare("--binary-path"))
        return ArgType::BINARY_PATH;
    else if (!string(arg).compare("-tp") ||
        !string(arg).compare("--target-path"))
        return ArgType::TARGET_PATH;
    else if(!string(arg).compare("-sp") ||
            !string(arg).compare("--serial-parity"))
        return ArgType::SERIAL_PARITY_YES;
    else if(!string(arg).compare("-ssb") ||
            !string(arg).compare("--serial-stop-bits"))
        return ArgType::SERIAL_STOP_BITS;
    else if(!string(arg).compare("-src") ||
            !string(arg).compare("--serial-rts-cts"))
        return ArgType::SERIAL_RTS_CTS_YES;
    else if(!string(arg).compare("-sbi") ||
            !string(arg).compare("--serial-bits"))
        return ArgType::SERIAL_BITS;
    else if(!string(arg).compare("-sba") ||
            !string(arg).compare("--serial-baud-rate"))
        return ArgType::SERIAL_BAUD_RATE;
    else if(!string(arg).compare("-sau") ||
            !string(arg).compare("--start-after-upload"))
        return ArgType::START_AFTER_UPLOAD;

    return ArgType::INVALID;
}

void printUsage()
{
    std::clog << "Usage" << std::endl;
}

bool validateTargetProperties(const TargetProperties& tp)
{
    using namespace std;

    bool valid = true;

    if (tp.path.empty())
    {
        clog << "Target path not specified." << std::endl;
        valid = false;
    }

    if (tp.stopBits == -1)
    {
        clog << "Stop bits not specified." << std::endl;
        valid = false;
    }
    else if (tp.stopBits != 1 && tp.stopBits != 2)
    {
        // FIXME: Investigate: Is this true?
        clog << "There can only be 1 or 2 stop bits!" << std::endl;
        valid = false;
    }

    if (tp.bits == -1)
    {
        clog << "Bits per byte not specified." << std::endl;
        valid = false;
    }
    else if (!(tp.bits >= 5 && tp.bits <= 8))
    {
        clog << "There can be 5-8 bits per byte." << std::endl;
        valid = false;
    }

    if (tp.baud == -1)
    {
        clog << "Baud rate not specified." << std::endl;
        valid = false;
    }

    return valid;
}

void printTargetProperties(const TargetProperties& tp)
{
    using namespace std;
    clog << "path " << tp.path;
    clog << "\nparity " << tp.parity;
    clog << "\nstopBits " << tp.stopBits;
    clog << "\nrtsCts " << tp.rtsCts;
    clog << "\nbits " << tp.bits;
    clog << "\nbaud " << tp.baud << std::endl;
}

void handleError()
{
    std::clog << "Error code : " << errno << std::endl;
    std::clog << "Reason: " << strerror(errno);
}

int main(int argc, char** argv)
{
    // process args

    if (argc == 1)
    {
        printUsage();
        return -1;
    }

    TargetProperties tp;

    std::string binaryPath;
    bool startAfterUpload = false;

    for (size_t i = 1; i < argc; i++)
    {
        switch (getArgType(argv[i]))
        {
        case ArgType::BINARY_PATH:
            binaryPath = argv[++i];
            break;
        case ArgType::TARGET_PATH:
            tp.path = argv[++i];
            break;
        case ArgType::SERIAL_PARITY_YES:
            tp.parity = true;
            break;
        case ArgType::SERIAL_STOP_BITS:
            tp.stopBits = std::stoi(argv[++i]);
            break;
        case ArgType::SERIAL_RTS_CTS_YES:
            tp.rtsCts = true;
            break;
        case ArgType::SERIAL_BITS:
            tp.bits = std::stoi(argv[++i]);
            break;
        case ArgType::SERIAL_BAUD_RATE:
            tp.baud = std::stoi(argv[++i]);
            break;
        case ArgType::START_AFTER_UPLOAD:
            startAfterUpload = true;
            break;
        case ArgType::INVALID:
            std::clog << "Invalid argument " << argv[i] << std::endl;
            printUsage();
            return -1;
        }
    }

    if (!validateTargetProperties(tp)) return -1;

    if (binaryPath.empty())
    {
        std::clog << "Binary path not specified." << std::endl;
        return -1;
    }

    printTargetProperties(tp);
    std::clog << "Binary Path: " << binaryPath << std::endl;


    SerialDevice device{std::make_shared<TargetProperties>(tp)};

    SerialDevice::Response devResponse = device.open();
    if (devResponse != SerialDevice::Response::SUCCESS)
    {
        std::clog << "Failed to setup device. Exit with code : " << devResponse << std::endl;
        return -1;
    }

    XModem modem{std::make_unique<SerialDevice>(device), 128};

    XModem::Response modemResponse = modem.upload(binaryPath, startAfterUpload);

    if (modemResponse != XModem::Response::SUCCESS)
    {
        std::clog << "Failed to upload file. Exit with code : " << modemResponse << std::endl;
        return -1;
    }

    std::clog << "Successfully uploaded!" << std::endl;

    return 0;
}
