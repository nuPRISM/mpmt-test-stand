#ifndef TEST_STAND_COMM_CONTROLLER_H
#define TEST_STAND_COMM_CONTROLLER_H

#include "TestStandComm.h"

#include "shared_defs.h"

/**
 * @class TestStandCommController
 * 
 * @brief Extension of TestStandComm to implement the controller-side application layer
 */
class TestStandCommController : public TestStandComm
{
    public:
        TestStandCommController(SerialDevice& device);

        SerialResult ping();
        SerialResult log(LogLevel log_level, const char *fmt, ...);
        SerialResult status(Status status);
        SerialResult position(int32_t x_counts, int32_t y_counts);
        SerialResult axis_state(/*TODO*/);
        SerialResult temp(/*TODO*/);
};

#endif // TEST_STAND_COMM_CONTROLLER_H