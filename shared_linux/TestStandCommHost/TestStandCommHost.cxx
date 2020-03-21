#include <stdio.h>
#include "TestStandCommHost.h"
#include "macros.h"

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

bool TestStandCommHost::move(uint32_t accel, uint32_t hold_vel, uint32_t dist, AxisId axis, DirectionId dir)
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
