#ifndef ARDUINO_SERIAL_DEVICE_H
#define ARDUINO_SERIAL_DEVICE_H

#include <SerialDevice.h>
#include <Arduino.h>

class ArduinoSerialDevice: public SerialDevice
{
    public:
        ArduinoSerialDevice(HardwareSerial *device);
        
        bool ser_connect(uint32_t baud_rate);
        uint32_t ser_available();
        uint8_t ser_read();
        bool ser_write(uint8_t *data, uint32_t length);
        void ser_disconnect();

    private:
        HardwareSerial *device;
};

#endif // ARDUINO_SERIAL_DEVICE_H