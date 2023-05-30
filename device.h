#ifndef DEVICE_H
#define DEVICE_H

#include <span>

class Device
{
public:

    virtual bool read(std::span<unsigned char> bytes) = 0;
    virtual bool write(std::span<const unsigned char> bytes) = 0;

    bool read(unsigned char* byte);
    bool write(const unsigned char* byte);
};

#endif // DEVICE_H
