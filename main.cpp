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

#include "logger.h"
#include "serialdevice.h"
#include "xmodem.h"

enum ArgType
{
    LOG_TO_FILE,
    BINARY_PATH,
    TARGET_PATH,
    XMODEM_MAX_RETRY,
    XMODEM_BLOCK_SIZE,
    SERIAL_DEVICE_ARIES,
    SERIAL_PARITY_YES,
    SERIAL_STOP_BITS,
    SERIAL_RTS_CTS_YES,
    SERIAL_BITS,
    SERIAL_BAUD_RATE,
    SERIAL_READ_TIMEOUT,
    START_AFTER_UPLOAD,
    PRINT_LICENSE,
    PRINT_USAGE,
    INVALID
};

ArgType getArgType(char* const& arg)
{
    using namespace std;

    if(!string(arg).compare("-l") ||
            !string(arg).compare("--log"))
        return ArgType::LOG_TO_FILE;
    else if (!string(arg).compare("-bp") ||
            !string(arg).compare("--binary-path"))
        return ArgType::BINARY_PATH;
    else if (!string(arg).compare("-tp") ||
            !string(arg).compare("--target-path"))
        return ArgType::TARGET_PATH;
    else if(!string(arg).compare("-xmr") ||
            !string(arg).compare("--xmodem-max-retry"))
        return ArgType::XMODEM_MAX_RETRY;
    else if(!string(arg).compare("-xbs") ||
            !string(arg).compare("--xmodem-block-size"))
        return ArgType::XMODEM_BLOCK_SIZE;
    else if(!string(arg).compare("--aries"))
        return ArgType::SERIAL_DEVICE_ARIES;
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
    else if(!string(arg).compare("-srt") ||
            !string(arg).compare("--serial-read-timeout"))
        return ArgType::SERIAL_READ_TIMEOUT;
    else if(!string(arg).compare("--license"))
        return ArgType::PRINT_LICENSE;
    else if(!string(arg).compare("-h") ||
            !string(arg).compare("--help"))
        return ArgType::PRINT_USAGE;

    return ArgType::INVALID;
}

void printLicense()
{
    constexpr const std::string_view license = R"(vegadude  Copyright (C) 2023  Debayan Sutradhar (rnayabed) (debayansutradhar3@gmail.com)
This program comes with ABSOLUTELY NO WARRANTY.
This is free software, and you are welcome to redistribute it
under certain conditions.

To view the full license, you may visit:
)";
    Logger::get() << license << LICENSE << Logger::NewLine;
}

void printUsage()
{
    constexpr const std::string_view usage = R"(Usage:  [-l | --log] [-bp | --binary-path]
        [-tp | --target-path]
        [-xmr | --xmodem-max-retry] [-xbs | --xmodem-block-size]
        [--aries] [-sp | --serial-parity] [-ssb | --serial-stop-bits]
        [-src | --serial-rts-cts] [-sbi | --serial-bits]
        [-sba | --serial-baud-rate] [-srt | --serial-read-timeout]
        [-sau | --start-after-upload] [--license] [-h | --help]

Option Summary:
    -l | --log                          Optional. Create a log file.

    -bp | --binary-path                 Required. Specify path to the binary file
                                        to be uploaded.

    -tp | --target-path                 Required. Specify path to the target board.

    -xmr | --xmodem-max-retry           Optional. Specify max amount of times to retry before aborting upload.
                                        Default is 10.

    -xbs | --xmodem-block-size          Required if not using automatic configuration.
                                        Specify block size of XModem data transfer.

    --aries                             Use CDAC Aries serial port configuration.

    -sp | --serial-parity               Optional. Specify if target uses parity bit.
                                        Default is false.

    -ssb | --serial-stop-bits           Required. Specify number of stop bits
                                        for target.

    -src | --serial-rts-cts             Optional. Specify if target uses RTS/CTS
                                        flow control.
                                        Default is false.

    -sbi | --serial-bits                Required. Specify the number of data bits sent
                                        in a byte to the target.

    -sba | --serial-baud-rate           Required. Specify serial baud rate of the
                                        target.

    -srt | --serial-read-timeout        Optional. Specify timeout for each read in
                                        milliseconds.
                                        Default is 500.

    -sau | --start-after-upload         Optional. Immediately start running program
                                        after uploading.

    --license                           Print license information.

    -h | --help                         Print this message.

NOTE: you cannot use --aries and --xmodem-block-size / --serial* arguments (except --serial-read-timeout) at the same time.)";
    Logger::get() << usage << Logger::NewLine;
}

bool validateProps(const std::filesystem::path& targetPath,
                   const int32_t& xmodemMaxRetry,
                   const int32_t& xmodemBlockSize,
                   const int32_t& serialReadTimeout,
                   const SerialDevice::DeviceProperties& dp)
{
    bool valid = true;

    if (targetPath.empty())
    {
        Logger::get() << "Target path not specified." << Logger::NewLine;
        valid = false;
    }

    if (xmodemMaxRetry == -1)
    {
        Logger::get() << "XMODEM Max retry invalid." << Logger::NewLine;
        valid = false;
    }
    else if (xmodemMaxRetry < 5)
    {
        Logger::get() << "XMODEM Max retry needs to be atleast 5." << Logger::NewLine;
        valid = false;
    }

    if (xmodemBlockSize == -1)
    {
        Logger::get() << "XMODEM Block size invalid." << Logger::NewLine;
        valid = false;
    }
    else if (xmodemMaxRetry < 1)
    {
        Logger::get() << "XMODEM Block size needs to be atleast 1." << Logger::NewLine;
        valid = false;
    }

    if (serialReadTimeout == -1)
    {
        Logger::get() << "Serial read timeout invalid." << Logger::NewLine;
        valid = false;
    }
    else if (!(serialReadTimeout >= 0 && serialReadTimeout <= 25500))
    {
        Logger::get() << "Serial read timeout can range only in between 0 and 25500 milliseconds." << Logger::NewLine;
        valid = false;
    }

    if (dp.stopBits == -1)
    {
        Logger::get() << "Stop bits invalid/not specified." << Logger::NewLine;
        valid = false;
    }
    else if (dp.stopBits != 1 && dp.stopBits != 2)
    {
        Logger::get() << "There can only be 1 or 2 stop bits!" << Logger::NewLine;
        valid = false;
    }

    if (dp.bits == -1)
    {
        Logger::get() << "Bits per byte invalid/not specified." << Logger::NewLine;
        valid = false;
    }
    else if (!(dp.bits >= 5 && dp.bits <= 8))
    {
        Logger::get() << "There can be 5-8 bits per byte." << Logger::NewLine;
        valid = false;
    }

    if (dp.baud == -1)
    {
        Logger::get() << "Baud rate not invalid/specified." << Logger::NewLine;
        valid = false;
    }
    else if(dp.baud < 1)
    {
        Logger::get() << "Baud rate needs to be atleast 1." << Logger::NewLine;
        valid = false;
    }

    return valid;
}

int stoi_e(char* str)
{
    int result = -1;

    try
    {
        result = std::stoi(str);
    }
    catch(std::invalid_argument)
    {
        result = -1;
    }

    return result;
}

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        printUsage();
        return -1;
    }

    SerialDevice::DeviceProperties dp;

    std::filesystem::path targetPath;
    std::filesystem::path binaryPath;
    std::filesystem::path logFilePath;

    bool startAfterUpload = false;

    bool isDevPropsSetManual = false;
    bool isDevPropsSetAuto = false;

    int32_t xmodemMaxRetry = 10;
    int32_t xmodemBlockSize = -1;
    int32_t serialReadTimeout = 500;

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
        case ArgType::XMODEM_MAX_RETRY:
            xmodemMaxRetry = stoi_e(argv[++i]);
            break;
        case ArgType::XMODEM_BLOCK_SIZE:
            isDevPropsSetManual = true;
            xmodemBlockSize = stoi_e(argv[++i]);
            break;
        case ArgType::SERIAL_DEVICE_ARIES:
            isDevPropsSetAuto = true;
            break;
        case ArgType::SERIAL_PARITY_YES:
            isDevPropsSetManual = true;
            dp.parity = true;
            break;
        case ArgType::SERIAL_STOP_BITS:
            isDevPropsSetManual = true;
            dp.stopBits = stoi_e(argv[++i]);
            break;
        case ArgType::SERIAL_RTS_CTS_YES:
            isDevPropsSetManual = true;
            dp.rtsCts = true;
            break;
        case ArgType::SERIAL_BITS:
            isDevPropsSetManual = true;
            dp.bits = stoi_e(argv[++i]);
            break;
        case ArgType::SERIAL_BAUD_RATE:
            isDevPropsSetManual = true;
            dp.baud = stoi_e(argv[++i]);
            break;
        case ArgType::SERIAL_READ_TIMEOUT:
            serialReadTimeout = stoi_e(argv[++i]);
            break;
        case ArgType::START_AFTER_UPLOAD:
            startAfterUpload = true;
            break;
        case ArgType::PRINT_LICENSE:
            printLicense();
            return 0;
        case ArgType::PRINT_USAGE:
            printUsage();
            return 0;
        case ArgType::INVALID:
            Logger::get() << "Invalid argument " << argv[i] << Logger::NewLine;
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
    else if (isDevPropsSetAuto)
    {
        dp = SerialDevice::ARIES;
        xmodemBlockSize = XModem::ARIES_blockSize;
    }

    if (!validateProps(targetPath, xmodemMaxRetry, xmodemBlockSize, serialReadTimeout, dp)) return -1;

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

    Logger::get() << "vegadude " << VERSION << Logger::NewLine << Logger::NewLine
                  << "================================================" << Logger::NewLine
                  << "Device Path: " << targetPath << Logger::NewLine
                  << "Binary Path: " << binaryPath << Logger::NewLine
                  << "Target device properties:" << Logger::NewLine
                  << "Parity: " << dp.parity << Logger::NewLine
                  << "Stop bits: " << dp.stopBits << Logger::NewLine
                  << "RTS CTS: " << dp.rtsCts << Logger::NewLine
                  << "Bits: " << dp.bits << Logger::NewLine
                  << "Baud Rate: " << dp.baud << Logger::NewLine
                  << "Read Timeout (in milliseconds): " << serialReadTimeout << Logger::NewLine
                  << "XMODEM Block Size " << xmodemBlockSize << Logger::NewLine
                  << "XMODEM Max Retry: " << xmodemMaxRetry << Logger::NewLine
                  << "================================================" << Logger::NewLine;

    SerialDevice device{targetPath, serialReadTimeout, dp};

    if (!device.open())
    {
        Logger::get() << "Failed to setup serial device!"
                      << Logger::NewLine << device.errorStr()
                      << Logger::NewLine;
        return -1;
    }

    XModem modem{device, xmodemMaxRetry, xmodemBlockSize};

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
