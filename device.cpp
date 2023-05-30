#include "device.h"

bool Device::read(unsigned char* byte)
{
    return read(std::span(byte, 1));
}

bool Device::write(const unsigned char* byte)
{
    return write(std::span(byte, 1));
}
