#ifndef ARDUINO_SERIAL_DEVICE_H
#define ARDUINO_SERIAL_DEVICE_H

#include <SerialDevice.h>
#include <Arduino.h>

class ArduinoSerialDevice: public SerialDevice
{
    public:
        ArduinoSerialDevice(HardwareSerial *device);
        
        bool connect(uint32_t baud_rate);
        uint32_t available();
        uint8_t read();
        bool write(uint8_t *data, uint32_t length);
        void close();

    private:
        HardwareSerial *device;
};

#endif // ARDUINO_SERIAL_DEVICE_H