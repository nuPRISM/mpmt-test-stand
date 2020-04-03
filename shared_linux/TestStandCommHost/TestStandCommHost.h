#ifndef TEST_STAND_COMM_HOST_H
#define TEST_STAND_COMM_HOST_H

#include "TestStandComm.h"

#include "shared_defs.h"

/**
 * @class TestStandCommHost
 * 
 * @brief Extension of TestStandComm to implement the host-side application layer
 */
class TestStandCommHost : public TestStandComm
{
    public:
        TestStandCommHost(SerialDevice& device);

        SerialResult ping();
        SerialResult get_status();
        SerialResult home();
        SerialResult move(uint32_t accel, uint32_t hold_vel, uint32_t dist, AxisId axis, Direction dir);
        SerialResult stop();
        SerialResult get_data(DataId data_id);
};

#endif // TEST_STAND_COMM_HOST_H