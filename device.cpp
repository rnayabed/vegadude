#include "device.h"

Device::Device()
{}

bool Device::read(unsigned char* bytes)
{
    return read(bytes, 1);
}

bool Device::write(const unsigned char* bytes)
{
    return write(bytes, 1);
}
