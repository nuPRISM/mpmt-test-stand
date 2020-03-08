#ifndef LINUX_SERIAL_DEVICE_H
#define LINUX_SERIAL_DEVICE_H

#include "SerialDevice.h"

#include <string>

class LinuxSerialDevice: public SerialDevice
{
    public:
        LinuxSerialDevice(std::string device_file);

        bool ser_connect(uint32_t baud_rate);
        uint32_t ser_available();
        uint8_t ser_read();
        bool ser_write(uint8_t *data, uint32_t length);
        void ser_disconnect();

    private:
        std::string device_file;
        int serial_port;
};

#endif // LINUX_SERIAL_DEVICE_H