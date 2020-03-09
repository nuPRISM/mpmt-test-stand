#ifndef TEST_STAND_COMM_CONTROLLER_H
#define TEST_STAND_COMM_CONTROLLER_H

#include "TestStandComm.h"

class TestStandCommController : public TestStandComm
{
    public:
        TestStandCommController(SerialDevice& device);

        bool log(LogLevel log_level, const char *fmt, ...);
        bool status(Status status);
        bool data(uint8_t *data, uint8_t length);
};

#endif // TEST_STAND_COMM_CONTROLLER_H