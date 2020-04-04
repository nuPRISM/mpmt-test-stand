#ifndef TEST_STAND_COMM_HOST_H
#define TEST_STAND_COMM_HOST_H

#include "TestStandComm.h"

#include "shared_defs.h"

typedef struct {
    int32_t x;
    int32_t y;
} Position;

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
        SerialResult get_status(Status *status_out, uint32_t timeout_ms);
        SerialResult home();
        SerialResult move(uint32_t accel, uint32_t hold_vel, uint32_t dist, AxisId axis, Direction dir);
        SerialResult stop();
        SerialResult get_position(Position *position_out, uint32_t timeout_ms);
};

#endif // TEST_STAND_COMM_HOST_H