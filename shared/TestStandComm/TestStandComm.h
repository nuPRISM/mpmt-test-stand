#ifndef TEST_STAND_COMM_H
#define TEST_STAND_COMM_H

#include "SerialDevice.h"
#include "SerialTransport.h"
#include "SerialSession.h"

class TestStandComm
{
    public:
        TestStandComm(SerialDevice *device);

        bool ping();
        bool get_status(uint8_t *status_resp);
        bool home();
        bool move(uint16_t accel, uint16_t hold_vel, uint16_t dist, uint8_t axis, uint8_t dir); // TODO enums?
        bool stop();
        bool get_data(uint8_t data_id); // TODO structs for response?

    private:
        SerialDevice *device;
        SerialTransport transport;
        SerialSession session;
};

#endif // TEST_STAND_COMM_H