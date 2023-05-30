#include "device.h"

bool Device::read(unsigned char* byte)
{
    return read({byte, 1});
}

bool Device::write(const unsigned char* byte)
{
    return write({byte, 1});
}

