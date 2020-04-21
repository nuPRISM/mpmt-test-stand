#include "ArduinoSerialDevice.h"

ArduinoSerialDevice::ArduinoSerialDevice(HardwareSerial &device) : device(device)
{
    // Nothing else to do
}

bool ArduinoSerialDevice::ser_connect(SerialBaudRate baud_rate)
{
    this->device.begin(baud_rate);
    return true;
}

void ArduinoSerialDevice::ser_flush()
{
    // Wait for 10 ms for last bits of data
    delay(10);
    uint32_t avail = this->device.available();
    while (avail > 0) {
        this->device.read();
        avail--;
    }
}

uint32_t ArduinoSerialDevice::ser_available()
{
    return this->device.available();
}

bool ArduinoSerialDevice::ser_read(uint8_t *out)
{
    *out = this->device.read();
    return true;
}

bool ArduinoSerialDevice::ser_write(uint8_t *data, uint32_t length)
{
    return this->device.write(data, length) == length;
}

void ArduinoSerialDevice::ser_disconnect()
{
    // Do nothing
}

uint64_t ArduinoSerialDevice::platform_millis()
{
    return millis();
}