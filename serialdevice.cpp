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

SerialDevice::SerialDevice(const std::filesystem::path& devicePath, const DeviceProperties& deviceProperties)
    : m_devicePath{devicePath},
      m_deviceProperties{deviceProperties},
      m_linuxFD{-1},
      m_error{Error::NONE}
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

    return std::format("Unknown error {}", static_cast<int>(m_error));
}

bool SerialDevice::read(unsigned char *bytes, size_t size)
{
#ifdef __linux
    if (m_linuxFD == -1)
    {
        m_error = Error::DEVICE_NOT_OPEN;
        return false;
    }
    else
    {
        if (::read(m_linuxFD, bytes, size) != -1)
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
#endif
    m_error = Error::NOT_SUPPORTED;
    return false;
}

bool SerialDevice::write(const unsigned char *bytes, size_t size)
{
#ifdef __linux
    if (m_linuxFD == -1)
    {
        m_error = Error::DEVICE_NOT_OPEN;
        return false;
    }
    else
    {
        if (::write(m_linuxFD, bytes, size) == size)
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
    m_error = Error::NOT_SUPPORTED;
    return false;
}

bool SerialDevice::open()
{
#ifdef __linux
    return openLinux();
#endif
    m_error = Error::NOT_SUPPORTED;
    return false;
}

bool SerialDevice::close()
{
#ifdef __linux
    return closeLinux();
#endif
    m_error = Error::NOT_SUPPORTED;
    return false;
}

const int &SerialDevice::linuxFD()
{
    return m_linuxFD;
}

bool SerialDevice::openLinux()
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
    if ( m_deviceProperties.parity)
        tty.c_cflag |= PARENB;
    else
        tty.c_cflag &= ~PARENB;


    // Stop Bit
    if ( m_deviceProperties.stopBits == 1)
        tty.c_cflag &= ~CSTOPB;
    else
        tty.c_cflag |= CSTOPB;


    //Bits per byte
    tty.c_cflag &= ~CSIZE;
    if ( m_deviceProperties.bits == 5)
        tty.c_cflag |= CS5;
    else if ( m_deviceProperties.bits == 6)
        tty.c_cflag |= CS6;
    else if ( m_deviceProperties.bits == 7)
        tty.c_cflag |= CS7;
    else if ( m_deviceProperties.bits == 8)
        tty.c_cflag |= CS8;


    // RTS/CTS
    if ( m_deviceProperties.rtsCts)
        tty.c_cflag |= CRTSCTS;
    else
        tty.c_cflag &= ~CRTSCTS;


    tty.c_cflag |= CREAD | CLOCAL;
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
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 255; // wait upto 10 desiseconds


    // set baud rate
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

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

bool SerialDevice::closeLinux()
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
