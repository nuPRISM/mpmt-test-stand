#ifndef LINUX_SERIAL_DEVICE_H
#define LINUX_SERIAL_DEVICE_H

#include "SerialDevice.h"

#include <string>

class LinuxSerialDevice
{
    public:
        LinuxSerialDevice(std::string device_file);

        bool connect(uint32_t baud_rate);
        uint32_t available();
        uint8_t read();
        bool write(uint8_t *data, uint32_t length);
        void close();

    private:
        std::string device_file;
        int serial_port;
};

#endif LINUX_SERIAL_DEVICE_H