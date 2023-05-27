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

#include "logger.h"
#include "serialdevice.h"
#include "xmodem.h"

enum ArgType
{
    LOG_TO_FILE,
    BINARY_PATH,
    TARGET_PATH,
    SERIAL_DEVICE_ARIES,
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

    if(!string(arg).compare("-o"))
        return ArgType::LOG_TO_FILE;
    else if (!string(arg).compare("-bp") ||
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
    else if(!string(arg).compare("-aries"))
        return ArgType::SERIAL_DEVICE_ARIES;

    return ArgType::INVALID;
}

void printUsage()
{
    Logger::get() << "Usage." << Logger::NewLine;
}

bool validateDeviceProperties(const std::string& targetPath, const SerialDevice::DeviceProperties& deviceProperties)
{
    using namespace std;

    bool valid = true;

    if (targetPath.empty())
    {
        Logger::get() << "Target path not specified." << Logger::NewLine;
        valid = false;
    }

    if (deviceProperties.stopBits == -1)
    {
        Logger::get() << "Stop bits not specified." << Logger::NewLine;
        valid = false;
    }
    else if (deviceProperties.stopBits != 1 && deviceProperties.stopBits != 2)
    {
        // FIXME: Investigate: Is this true?
        Logger::get() << "There can only be 1 or 2 stop bits!" << Logger::NewLine;
        valid = false;
    }

    if (deviceProperties.bits == -1)
    {
        Logger::get() << "Bits per byte not specified." << Logger::NewLine;
        valid = false;
    }
    else if (!(deviceProperties.bits >= 5 && deviceProperties.bits <= 8))
    {
        Logger::get() << "There can be 5-8 bits per byte." << Logger::NewLine;
        valid = false;
    }

    if (deviceProperties.baud == -1)
    {
        Logger::get() << "Baud rate not specified." << Logger::NewLine;
        valid = false;
    }

    return valid;
}

void printDeviceProperties(const SerialDevice::DeviceProperties& deviceProperties)
{
    using namespace std;
    Logger::get() << "parity " << deviceProperties.parity << Logger::NewLine;
    Logger::get() << "stopBits " << deviceProperties.stopBits << Logger::NewLine;
    Logger::get() << "rtsCts " << deviceProperties.rtsCts << Logger::NewLine;
    Logger::get() << "bits " << deviceProperties.bits << Logger::NewLine;
    Logger::get() << "baud " << deviceProperties.baud << Logger::NewLine;
}

int main(int argc, char** argv)
{
    // process args

    if (argc == 1)
    {
        printUsage();
        return -1;
    }

    SerialDevice::DeviceProperties dp;

    std::string targetPath;
    std::string binaryPath;
    std::string logFilePath;

    bool startAfterUpload = false;

    bool isDevPropsSetManual = false;
    bool isDevPropsSetAuto = false;


    for (size_t i = 1; i < argc; i++)
    {
        switch (getArgType(argv[i]))
        {
        case ArgType::LOG_TO_FILE:
            logFilePath = argv[++i];
            break;
        case ArgType::BINARY_PATH:
            binaryPath = argv[++i];
            break;
        case ArgType::TARGET_PATH:
            targetPath = argv[++i];
            break;
        case ArgType::SERIAL_PARITY_YES:
            isDevPropsSetManual = true;
            dp.parity = true;
            break;
        case ArgType::SERIAL_STOP_BITS:
            isDevPropsSetManual = true;
            dp.stopBits = std::stoi(argv[++i]);
            break;
        case ArgType::SERIAL_RTS_CTS_YES:
            isDevPropsSetManual = true;
            dp.rtsCts = true;
            break;
        case ArgType::SERIAL_BITS:
            isDevPropsSetManual = true;
            dp.bits = std::stoi(argv[++i]);
            break;
        case ArgType::SERIAL_BAUD_RATE:
            isDevPropsSetManual = true;
            dp.baud = std::stoi(argv[++i]);
            break;
        case ArgType::START_AFTER_UPLOAD:
            isDevPropsSetManual = true;
            startAfterUpload = true;
            break;
        case ArgType::SERIAL_DEVICE_ARIES:
            isDevPropsSetAuto = true;
            dp = SerialDevice::ARIES;
        case ArgType::INVALID:
            Logger::get() << "Invalid argument " << argv[i];
            printUsage();
            return -1;
        }
    }

    if (isDevPropsSetAuto && isDevPropsSetManual)
    {
        Logger::get() << "You cannot use -aries and manually set device properties simultaneously."
                      << Logger::NewLine;
        return -1;
    }

    if (!validateDeviceProperties(targetPath, dp)) return -1;

    if (!logFilePath.empty())
    {
        if(!Logger::get().setup(logFilePath))
        {
            Logger::get() << "Unable to setup logging!"
                          << Logger::NewLine;
            return -1;
        }
    }

    if (binaryPath.empty())
    {
        Logger::get() << "Binary path not specified." << Logger::NewLine;
        return -1;
    }

    if (isDevPropsSetAuto)
    {
        dp = SerialDevice::ARIES;
    }

    printDeviceProperties(dp);

    Logger::get() << "Device Path: " << targetPath << Logger::NewLine;
    Logger::get() << "Binary Path: " << binaryPath << Logger::NewLine;


    SerialDevice device{targetPath, dp};

    if (!device.open())
    {
        Logger::get() << "Failed to setup serial device!"
                      << Logger::NewLine << device.errorStr()
                      << Logger::NewLine;
        return -1;
    }

    XModem modem{device, 128};

    if (!modem.upload(binaryPath, startAfterUpload))
    {
        Logger::get() << "Failed to upload file!"
                      << Logger::NewLine
                      << ((modem.error() == XModem::Error::DEVICE_RELATED) ? device.errorStr() : modem.errorStr())
                      << Logger::NewLine;
        return -1;
    }

    Logger::get() << "Successfully uploaded!";

    Logger::get().close();

    return 0;
}
