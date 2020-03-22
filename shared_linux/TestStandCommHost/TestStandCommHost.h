#ifndef TEST_STAND_COMM_HOST_H
#define TEST_STAND_COMM_HOST_H

#include "TestStandComm.h"

#include "shared_defs.h"

class TestStandCommHost : public TestStandComm
{
    public:
        TestStandCommHost(SerialDevice& device);

        bool ping();
        bool get_status();
        bool home();
        bool move(uint32_t accel, uint32_t hold_vel, uint32_t dist, AxisId axis, Direction dir);
        bool stop();
        bool get_data(DataId data_id);
};

#endif // TEST_STAND_COMM_HOST_H