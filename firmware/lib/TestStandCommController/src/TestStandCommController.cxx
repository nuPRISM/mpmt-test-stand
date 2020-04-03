#include "Arduino.h"
#include "TestStandCommController.h"

static uint8_t send_buf[MSG_DATA_LENGTH_MAX];

TestStandCommController::TestStandCommController(SerialDevice& device) : TestStandComm(device)
{
    // Nothing else to do
}

SerialResult TestStandCommController::ping()
{
    return this->send_basic_msg(MSG_ID_PING);
}

SerialResult TestStandCommController::log(LogLevel log_level, const char *fmt, ...)
{
    send_buf[0] = (uint8_t)log_level;

    va_list args;
    va_start(args, fmt);
    uint8_t length = vsnprintf((char *)(send_buf + 1), (MSG_DATA_LENGTH_MAX - 1), fmt, args);
    va_end(args);

    Message msg = {
        .id = MSG_ID_LOG,
        .length = (1 + length + 1), // 1 byte for log level + string length
        .data = send_buf
    };
    return this->session.send_message(msg);
}

SerialResult TestStandCommController::status(Status status)
{
    uint8_t status8 = (uint8_t)status;
    Message msg = {
        .id = MSG_ID_STATUS,
        .length = 1,
        .data = &status8
    };
    return this->session.send_message(msg);
}

SerialResult TestStandCommController::data(uint8_t *data, uint8_t length)
{
    Message msg = {
        .id = MSG_ID_DATA,
        .length = length,
        .data = data
    };
    return this->session.send_message(msg);
}