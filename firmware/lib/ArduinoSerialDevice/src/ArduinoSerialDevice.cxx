#include "ArduinoSerialDevice.h"

ArduinoSerialDevice::ArduinoSerialDevice(HardwareSerial *device)
{
    this->device = device;
}

bool ArduinoSerialDevice::connect(uint32_t baud_rate)
{
    this->device->begin(baud_rate);
    return true;
}

uint32_t ArduinoSerialDevice::available()
{
    return this->device->available();
}

uint8_t ArduinoSerialDevice::read()
{
    return this->device->read();
}

bool ArduinoSerialDevice::write(uint8_t *data, uint32_t length)
{
    return this->device->write(data, length) == length;
}

void ArduinoSerialDevice::close()
{
    // Do nothing
}
