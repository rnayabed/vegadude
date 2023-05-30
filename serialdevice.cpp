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

#include "serialdevice.h"

SerialDevice::SerialDevice(const std::filesystem::path& devicePath,
                           const DeviceProperties& deviceProperties,
                           const int32_t& readTimeout)
    : m_error{Error::NONE},
      m_devicePath{devicePath},
      m_deviceProperties{deviceProperties},
      m_readTimeout{readTimeout}
{}

const SerialDevice::Error &SerialDevice::error()
{
    return m_error;
}

std::string SerialDevice::errorStr()
{
    switch (m_error)
    {
    case NONE:
        return "None";
    case FAILED_TO_OPEN_DEVICE:
        return "Failed to open device";
    case FAILED_TO_GET_FD_ATTRS:
        return "Failed to get file descriptor attributes";
    case FAILED_TO_SET_FD_ATTRS:
        return "Failed to set file descriptor attributes";
    case NOT_SUPPORTED:
        return "Operation not supported";
    case READ_FAILED:
        return "Read failed";
    case WRITE_FAILED:
        return "Write failed";
    case DEVICE_NOT_OPEN:
        return "Device not open";
    }

    return "Unknown error " + std::to_string(m_error);
}

#ifdef __linux

bool SerialDevice::open()
{
    m_linuxFD = ::open(m_devicePath.c_str(),
                       O_RDWR | O_NOCTTY);

    if (m_linuxFD < 0)
    {
        m_error = Error::FAILED_TO_OPEN_DEVICE;
        return false;
    }

    struct termios tty;

    if (ioctl(m_linuxFD, TCGETS, &tty) < 0)
    {
        m_error =  Error::FAILED_TO_GET_FD_ATTRS;
        return false;
    }

    // Parity
    if (m_deviceProperties.parity)
        tty.c_cflag |= PARENB;
    else
        tty.c_cflag &= ~PARENB;


    // Stop Bit
    if (m_deviceProperties.stopBits == 1)
        tty.c_cflag &= ~CSTOPB;
    else
        tty.c_cflag |= CSTOPB;


    //Bits per byte
    tty.c_cflag &= ~CSIZE;
    if (m_deviceProperties.bits == 5)
        tty.c_cflag |= CS5;
    else if (m_deviceProperties.bits == 6)
        tty.c_cflag |= CS6;
    else if (m_deviceProperties.bits == 7)
        tty.c_cflag |= CS7;
    else if (m_deviceProperties.bits == 8)
        tty.c_cflag |= CS8;


    // RTS/CTS
    if (m_deviceProperties.rtsCts)
        tty.c_cflag |= CRTSCTS;
    else
        tty.c_cflag &= ~CRTSCTS;


    tty.c_cflag |= CREAD | CLOCAL;

    /*
     * We need cflag to be 0001 1100 1011 0010
     *
     * I could not find documentation on how to set nibbles 1 and 4.
     */
    tty.c_cflag = (tty.c_cflag | 0x1002) & ~0xd;

    //tty.c_cflag = 7346;

    // CREAD = allows us to read data
    // CLOCAL = disables "carrier detect"
    //          disables SIGHUP signal to be sent when disconnected


    // Local modes flags

    // Disable Canonical mode
    // Prevents presence of backspace to erase previous byte
    // Prevents taking newline as signal to process input
    tty.c_lflag &= ~ICANON;

    // Disabling canon mode is equivalent to disabling 3
    // below. but need to check
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo


    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP

    // Input modes flags

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl

    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    // Output modes flags

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed


    // Line discpline not covered

    // Control characters (cc) (VMIN, VTIME)
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = m_readTimeout / 100; // milli -> deci


    // set baud rate
    cfsetispeed(&tty, m_deviceProperties.baud);
    cfsetospeed(&tty, m_deviceProperties.baud);

    if (!ioctl(m_linuxFD, TCSETS, &tty))
    {
        m_error = Error::NONE;
        return true;
    }
    else
    {
        m_error = Error::FAILED_TO_SET_FD_ATTRS;
        return false;
    }
}

bool SerialDevice::close()
{
    ioctl(m_linuxFD, TCFLSH, 2);

    if (!::close(m_linuxFD))
    {
        m_error = Error::NONE;
        return true;
    }
    else
    {
        m_error = Error::FAILED_TO_SET_FD_ATTRS;
        return false;
    }
}

bool SerialDevice::read(std::span<unsigned char> bytes)
{
    if (m_linuxFD == -1)
    {
        m_error = Error::DEVICE_NOT_OPEN;
        return false;
    }
    else
    {
        if (::read(m_linuxFD, bytes.data(), bytes.size()) != -1)
        {
            m_error = Error::NONE;
            return true;
        }
        else
        {
            m_error = Error::READ_FAILED;
            return false;
        }
    }
}

bool SerialDevice::write(std::span<const unsigned char> bytes)
{
    if (m_linuxFD == -1)
    {
        m_error = Error::DEVICE_NOT_OPEN;
        return false;
    }
    else
    {
        if (::write(m_linuxFD, bytes.data(), bytes.size()) == static_cast<ssize_t>(bytes.size()))
        {
            m_error = Error::NONE;
            return true;
        }
        else
        {
            m_error = Error::WRITE_FAILED;
            return false;
        }
    }
    m_error = Error::NOT_SUPPORTED;
    return false;
}

#elif __WIN32

bool SerialDevice::open()
{
    m_winHandle = CreateFile(m_devicePath.c_str(),
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);

    if (m_winHandle == INVALID_HANDLE_VALUE)
    {
        m_error = Error::FAILED_TO_OPEN_DEVICE;
        return false;
    }

    DCB params;

    if(GetCommState(m_winHandle, &params) == FALSE)
    {
        m_error = Error::FAILED_TO_GET_FD_ATTRS;
        return false;
    }

    params.BaudRate = m_deviceProperties.baud;
    params.ByteSize = m_deviceProperties.bits;

    if (m_deviceProperties.stopBits == 1)
    {
        params.StopBits = ONESTOPBIT;
    }
    else
    {
        params.StopBits = TWOSTOPBITS;
    }

    params.Parity = m_deviceProperties.parity;

    if (SetCommState(m_winHandle, &params) == FALSE)
    {
        m_error = Error::FAILED_TO_SET_FD_ATTRS;
        return false;
    }

    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = m_readTimeout;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (SetCommTimeouts(m_winHandle, &timeouts) == FALSE)
    {
        // FIXME: This needs to be it's own enum entry
        m_error = Error::FAILED_TO_SET_FD_ATTRS;
        return false;
    }

    m_error = Error::NONE;
    return true;
}

bool SerialDevice::close()
{
    m_error = Error::NOT_SUPPORTED;
    return false;
}

bool SerialDevice::read(std::span<unsigned char> bytes)
{
    if (ReadFile(m_winHandle, bytes.data(), bytes.size(), NULL, NULL))
    {
        m_error = Error::NONE;
        return true;
    }
    else
    {
        m_error = Error::READ_FAILED;
        return false;
    }
}

bool SerialDevice::write(std::span<const unsigned char> bytes)
{
    unsigned long written;
    bool result = WriteFile(m_winHandle, bytes.data(), bytes.size(), &written, NULL);

    if (result && bytes.size() == written)
    {
        m_error = Error::NONE;
        return true;
    }
    else
    {
        m_error = Error::WRITE_FAILED;
        return false;
    }
}

#endif
