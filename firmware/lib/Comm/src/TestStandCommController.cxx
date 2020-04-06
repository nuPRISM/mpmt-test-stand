#include "TestStandCommController.h"
#include "TestStandMessages.h"
#include "Gantry.h"
#include "macros.h"
#include <Arduino.h>

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
    uint8_t length = (uint8_t)vsnprintf((char *)(send_buf + 1), (MSG_DATA_LENGTH_MAX - 1), fmt, args);
    va_end(args);

    Message msg = {
        .id = MSG_ID_LOG,
        .length = (uint8_t)(1 + length + 1), // 1 byte for log level + string length
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

SerialResult TestStandCommController::position(int32_t x_counts, int32_t y_counts)
{
    PositionMsgData data;
    HTONL(&(data.x_counts), x_counts);
    HTONL(&(data.y_counts), y_counts);

    Message msg = {
        .id = MSG_ID_POSITION,
        .length = sizeof(data),
        .data = (uint8_t *)&data
    };
    return this->session.send_message(msg);
}

SerialResult TestStandCommController::axis_result(AxisResult result)
{
    uint8_t result8 = (uint8_t)result;

    Message msg = {
        .id = MSG_ID_AXIS_RESULT,
        .length = 1,
        .data = &result8
    };
    return this->session.send_message(msg);
}

bool TestStandCommController::recv_move(MoveMsgData *data_out)
{
    if (this->received_message().length != sizeof(MoveMsgData)) return false;

    MoveMsgData *data_in = (MoveMsgData *)this->received_message().data;
    data_out->vel_hold    = NTOHL(&(data_in->vel_hold));
    data_out->dist_counts = NTOHL(&(data_in->dist_counts));
    data_out->axis        = data_in->axis;
    data_out->dir         = data_in->dir;
    return true;
}
