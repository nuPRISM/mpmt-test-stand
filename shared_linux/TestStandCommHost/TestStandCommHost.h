#ifndef TEST_STAND_COMM_HOST_H
#define TEST_STAND_COMM_HOST_H

#include "TestStandComm.h"
#include "TestStandMessages.h"

#include "Gantry.h"
#include "TemperatureDAQ.h"

#include "Calibration.h"

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

        SerialResult get_status(Status *status_out, uint32_t timeout_ms);
        SerialResult home();
        SerialResult move(AxisId axis, AxisDirection dir, uint32_t vel_hold, uint32_t dist_counts, AxisResult *res_out, uint32_t timeout_ms);
        SerialResult stop();
        SerialResult get_position(PositionMsgData *position_out, uint32_t timeout_ms);
        SerialResult get_temp(TempData *temp_out, uint32_t timeout_ms);
        SerialResult calibrate(CalibrationKey key, uint32_t data);
        SerialResult calibrate(CalibrationKey key, double data);
};

#endif // TEST_STAND_COMM_HOST_H