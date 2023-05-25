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

SerialDevice::SerialDevice(std::shared_ptr<TargetProperties> targetProperties)
    : m_targetProperties{targetProperties},
      m_linuxFD{-1}
{}

SerialDevice::Response SerialDevice::open()
{
#ifdef __linux
    return openLinux();
#endif
    return Response::NOT_SUPPORTED;
}

SerialDevice::Response SerialDevice::close()
{
#ifdef __linux
    return closeLinux();
#endif
    return Response::NOT_SUPPORTED;
}

const int &SerialDevice::linuxFD()
{
    return m_linuxFD;
}

SerialDevice::Response SerialDevice::read(void *byte)
{
    return read(byte, 1);
}

SerialDevice::Response SerialDevice::read(void* bytes,
                                          size_t bytesLen)
{
#ifdef __linux
    return (m_linuxFD == -1) ?
                Response::ERROR_DEVICE_NOT_OPEN :
                (::read(m_linuxFD, bytes, bytesLen) < 0) ?
                                Response::ERROR_READ_FAILED :
                                Response::SUCCESS;
#endif
    return Response::NOT_SUPPORTED;
}

SerialDevice::Response SerialDevice::write(const void *byte)
{
    return write(byte, 1);
}

SerialDevice::Response SerialDevice::write(const void* bytes,
                                           size_t bytesLen)
{
#ifdef __linux
    return (m_linuxFD == -1) ?
                Response::ERROR_DEVICE_NOT_OPEN :
                (::write(m_linuxFD, bytes, bytesLen) < 0) ?
                                Response::ERROR_WRITE_FAILED :
                                Response::SUCCESS;
#endif
    return Response::NOT_SUPPORTED;
}

template <typename T>
SerialDevice::Response SerialDevice::write(std::vector<T> &vector)
{
#ifdef __linux
    return (m_linuxFD == -1) ?
                Response::ERROR_DEVICE_NOT_OPEN :
                (::write(m_linuxFD, vector.data(), vector.size()) < 0) ?
                                Response::ERROR_WRITE_FAILED :
                                Response::SUCCESS;
#endif
    return Response::NOT_SUPPORTED;
}

template SerialDevice::Response SerialDevice
    ::write<unsigned char>(std::vector<unsigned char> &vector);

SerialDevice::Response SerialDevice::openLinux()
{
    m_linuxFD = ::open(m_targetProperties->path.c_str(),
                           O_RDWR | O_NOCTTY);

    if (m_linuxFD < 0)
    {
        return Response::ERROR_FAILED_TO_OPEN_DEVICE;
    }

    struct termios tty;

    if (ioctl(m_linuxFD, TCGETS, &tty) < 0)
    {
        return Response::ERROR_FAILED_TO_GET_FD_ATTRS;
    }

    // Parity
    if (m_targetProperties->parity)
        tty.c_cflag |= PARENB;
    else
        tty.c_cflag &= ~PARENB;


    // Stop Bit
    if (m_targetProperties->stopBits == 1)
        tty.c_cflag &= ~CSTOPB;
    else
        tty.c_cflag |= CSTOPB;


    //Bits per byte
    tty.c_cflag &= ~CSIZE;
    if (m_targetProperties->bits == 5)
        tty.c_cflag |= CS5;
    else if (m_targetProperties->bits == 6)
        tty.c_cflag |= CS6;
    else if (m_targetProperties->bits == 7)
        tty.c_cflag |= CS7;
    else if (m_targetProperties->bits == 8)
        tty.c_cflag |= CS8;


    // RTS/CTS
    if (m_targetProperties->rtsCts)
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


    return (ioctl(m_linuxFD, TCSETS, &tty) < 0) ?
                ERROR_FAILED_TO_SET_FD_ATTRS :
                SUCCESS;
}

SerialDevice::Response SerialDevice::closeLinux()
{
    ioctl(m_linuxFD, TCFLSH, 2);
    return (::close(m_linuxFD) < 0) ?
                ERROR_FAILED_TO_SET_FD_ATTRS :
                SUCCESS;
}
