#include <stdio.h>
#include "TestStandCommHost.h"

#define GET_BYTE(_x, _n) (uint8_t)(((_x) >> ((_n)*8)) & 0x00FF)

TestStandCommHost::TestStandCommHost(SerialDevice& device) : TestStandComm(device)
{
    // Nothing else to do
}

bool TestStandCommHost::ping()
{
    return this->send_basic_msg(MSG_ID_PING);
}

bool TestStandCommHost::get_status()
{
    return this->send_basic_msg(MSG_ID_STATUS);
}

bool TestStandCommHost::home()
{
    return this->send_basic_msg(MSG_ID_HOME);
}

bool TestStandCommHost::move(uint16_t accel, uint16_t hold_vel, uint16_t dist, AxisId axis, Direction dir)
{
    // Transmit in big endian order
    uint8_t data[8];
    data[0] = GET_BYTE(accel, 1);
    data[1] = GET_BYTE(accel, 0);
    data[2] = GET_BYTE(hold_vel, 1);
    data[3] = GET_BYTE(hold_vel, 0);
    data[4] = GET_BYTE(dist, 1);
    data[5] = GET_BYTE(dist, 0);
    data[6] = (uint8_t)axis;
    data[7] = (uint8_t)dir;

    Message msg = {
        .id = MSG_ID_MOVE,
        .length = sizeof(data),
        .data = data
    };
    return this->session.send_message(msg);
}

bool TestStandCommHost::stop()
{
    return this->send_basic_msg(MSG_ID_STOP);
}

bool TestStandCommHost::get_data(DataId data_id)
{
    Message msg = {
        .id = MSG_ID_GET_DATA,
        .length = 1,
        .data = (uint8_t *)&data_id
    };
    return this->session.send_message(msg);
}