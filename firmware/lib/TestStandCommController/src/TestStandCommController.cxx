#include "TestStandCommController.h"

TestStandCommController::TestStandCommController(SerialDevice *device) : TestStandComm(device)
{
    // Nothing else to do
}

bool TestStandCommController::log(LogLevel log_level, const char *fmt, ...)
{
    return this->send_empty_msg(MSG_ID_LOG);
}

bool TestStandCommController::status(Status status)
{
    return false;
}

bool TestStandCommController::data(uint8_t *data, uint8_t length)
{
    return false;
}