#ifndef SERIAL_DEVICE_H
#define SERIAL_DEVICE_H

#include <stdint.h>

class SerialDevice
{
    public:
        virtual bool connect(uint32_t baud_rate) = 0;
        virtual uint32_t available() = 0;
        virtual uint8_t read() = 0;
        virtual bool write(uint8_t *data, uint32_t length) = 0;
        virtual void close() = 0;
};

#endif // SERIAL_DEVICE_H