#include "TestStandCommHost.h"
#include "Gantry.h"
#include "macros.h"

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

    res = this->recv_message(timeout_ms);
    if (res != SERIAL_OK) return res;

    if (this->received_message().id != MSG_ID_STATUS) return SERIAL_ERR_WRONG_MSG;

    *status_out = (Status)((this->received_message().data)[0]);
    return SERIAL_OK;
}

SerialResult TestStandCommHost::home()
{
    return this->send_basic_msg(MSG_ID_HOME);
}

SerialResult TestStandCommHost::move(uint32_t accel, uint32_t hold_vel, uint32_t dist, AxisId axis, AxisDirection dir)
{
    // Transmit in big endian order
    uint8_t data[14];
    HTONL(data, accel);
    HTONL(data + 4, hold_vel);
    HTONL(data + 8, dist);
    data[12] = (uint8_t)axis;
    data[13] = (uint8_t)dir;

    Message msg = {
        .id = MSG_ID_MOVE,
        .length = sizeof(data),
        .data = data
    };
    return this->session.send_message(msg);
}

SerialResult TestStandCommHost::stop()
{
    return this->send_basic_msg(MSG_ID_STOP);
}

SerialResult TestStandCommHost::get_position(Position *position_out, uint32_t timeout_ms)
{
    SerialResult res = this->send_basic_msg(MSG_ID_GET_POSITION);
    if (res != SERIAL_OK) return res;

    res = this->recv_message(timeout_ms);
    if (res != SERIAL_OK) return res;

    if (this->received_message().id != MSG_ID_POSITION) return SERIAL_ERR_WRONG_MSG;

    uint8_t *data = this->received_message().data;
    position_out->x_counts = NTOHL(data);
    position_out->y_counts = NTOHL(data + 4);
    return SERIAL_OK;
}
