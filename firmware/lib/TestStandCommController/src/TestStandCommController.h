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
        SerialResult data(uint8_t *data, uint8_t length);
};

#endif // TEST_STAND_COMM_CONTROLLER_H