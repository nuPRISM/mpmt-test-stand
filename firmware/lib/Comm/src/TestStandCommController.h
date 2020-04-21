#ifndef TEST_STAND_COMM_CONTROLLER_H
#define TEST_STAND_COMM_CONTROLLER_H

/* **************************** Local Includes ***************************** */
// Serial Communication
#include "TestStandComm.h"
#include "TestStandMessages.h"
// Gantry
#include "Gantry.h"
// Temperature DAQ
#include "TemperatureDAQ.h"
// Other
#include "Calibration.h"

/* ************************ Shared Project Includes ************************ */
#include "shared_defs.h"

/**
 * @class TestStandCommController
 * 
 * @brief Extension of TestStandComm to implement the controller-side application layer
 */
class TestStandCommController : public TestStandComm
{
    public:
        TestStandCommController(SerialDevice &device);

        SerialResult log(LogLevel log_level, const char *fmt, ...);
        SerialResult status(Status status);
        SerialResult position(int32_t x_counts, int32_t y_counts);
        SerialResult axis_state(/*TODO*/);
        SerialResult temp(TempData *temp_data);
        SerialResult axis_result(AxisResult result);

        bool recv_move(MoveMsgData *data_out);
        bool recv_calibrate(Calibration *cal_out);
};

#endif // TEST_STAND_COMM_CONTROLLER_H
