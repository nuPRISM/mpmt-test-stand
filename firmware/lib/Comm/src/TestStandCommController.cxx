#include "TestStandCommController.h"

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
    PositionMsgData data = {
        .x_counts = htonl(x_counts),
        .y_counts = htonl(y_counts)
    };

    Message msg = {
        .id = MSG_ID_POSITION,
        .length = sizeof(data),
        .data = (uint8_t *)&data
    };
    return this->session.send_message(msg);
}

SerialResult TestStandCommController::temp(TempData *temp_data)
{
    TempMsgData data = {
        .temp_ambient = htonl(round(temp_data->temp_ambient * temp_data_scaler)),
        .temp_motor_x = htonl(round(temp_data->temp_motor_x * temp_data_scaler)),
        .temp_motor_y = htonl(round(temp_data->temp_motor_y * temp_data_scaler)),
        .temp_mpmt    = htonl(round(temp_data->temp_mpmt    * temp_data_scaler)),
        .temp_optical = htonl(round(temp_data->temp_optical * temp_data_scaler))
    };

    Message msg = {
        .id = MSG_ID_TEMP,
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

    // Copy message data into output struct
    memcpy(data_out, this->received_message().data, sizeof(MoveMsgData));
    // Fixup byte order
    data_out->vel_hold    = ntohl(data_out->vel_hold);
    data_out->dist_counts = ntohl(data_out->dist_counts);

    return true;
}
