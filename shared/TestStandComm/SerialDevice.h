#ifndef SERIAL_DEVICE_H
#define SERIAL_DEVICE_H

#include <stdint.h>

class SerialDevice
{
    public:
        virtual bool ser_connect(uint32_t baud_rate) = 0;
        virtual uint32_t ser_available() = 0;
        virtual uint8_t ser_read() = 0;
        virtual bool ser_write(uint8_t *data, uint32_t length) = 0;
        virtual void ser_disconnect() = 0;
};

#endif // SERIAL_DEVICE_H