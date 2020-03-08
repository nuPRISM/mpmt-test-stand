#ifndef TEST_STAND_COMM_HOST_H
#define TEST_STAND_COMM_HOST_H

#include "TestStandComm.h"

class TestStandCommHost : public TestStandComm
{
    public:
        TestStandCommHost(SerialDevice *device);

        bool ping();
        bool get_status();
        bool home();
        bool move(uint16_t accel, uint16_t hold_vel, uint16_t dist, uint8_t axis, uint8_t dir);
        bool stop();
        bool get_data(DataId data_id);
};

#endif // TEST_STAND_COMM_HOST_H