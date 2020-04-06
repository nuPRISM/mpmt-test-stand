#include "TestStandCommHost.h"
#include "Gantry.h"

#include "macros.h"

#include <stdio.h>

TestStandCommHost::TestStandCommHost(SerialDevice& device) : TestStandComm(device)
{
    // Nothing else to do
}

SerialResult TestStandCommHost::ping()
{
    return this->send_basic_msg(MSG_ID_PING);
}

SerialResult TestStandCommHost::get_status(Status *status_out, uint32_t timeout_ms)
{
    SerialResult res = this->send_basic_msg(MSG_ID_GET_STATUS);
    if (res != SERIAL_OK) return res;

    res = this->recv_message(MSG_ID_STATUS, 1, timeout_ms);
    if (res != SERIAL_OK) return res;

    *status_out = (Status)((this->received_message().data)[0]);
    return SERIAL_OK;
}

SerialResult TestStandCommHost::home()
{
    return this->send_basic_msg(MSG_ID_HOME);
}

SerialResult TestStandCommHost::move(AxisId axis, AxisDirection dir, uint32_t vel_hold, uint32_t dist_counts, AxisResult *res_out, uint32_t timeout_ms)
{
    // Send move message
    MoveMsgData data;
    data.axis = (uint8_t)axis;
    data.dir = (uint8_t)dir;
    HTONL(&(data.vel_hold), vel_hold);
    HTONL(&(data.dist_counts), dist_counts);

    Message msg = {
        .id = MSG_ID_MOVE,
        .length = sizeof(data),
        .data = (uint8_t *)&data
    };
    
    SerialResult res = this->session.send_message(msg);
    if (res != SERIAL_OK) return res;

    // Get result
    res = this->recv_message(MSG_ID_AXIS_RESULT, 1, timeout_ms);
    if (res != SERIAL_OK) return res;

    *res_out = (AxisResult)((this->received_message().data)[0]);
    return SERIAL_OK;
}

SerialResult TestStandCommHost::stop()
{
    return this->send_basic_msg(MSG_ID_STOP);
}

SerialResult TestStandCommHost::get_position(PositionMsgData *position_out, uint32_t timeout_ms)
{
    SerialResult res = this->send_basic_msg(MSG_ID_GET_POSITION);
    if (res != SERIAL_OK) return res;

    res = this->recv_message(MSG_ID_POSITION, sizeof(PositionMsgData), timeout_ms);
    if (res != SERIAL_OK) return res;

    PositionMsgData *data_in = (PositionMsgData *)this->received_message().data;
    position_out->x_counts = NTOHL(&(data_in->x_counts));
    position_out->y_counts = NTOHL(&(data_in->y_counts));
    return SERIAL_OK;
}
