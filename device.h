#ifndef DEVICE_H
#define DEVICE_H

#include <span>

class Device
{
public:
    Device();

    bool read(unsigned char* bytes);
    bool write(const unsigned char* bytes);

    virtual bool read(unsigned char* bytes, size_t size) = 0;
    virtual bool write(const unsigned char* bytes, size_t size) = 0;
};

#endif // DEVICE_H
