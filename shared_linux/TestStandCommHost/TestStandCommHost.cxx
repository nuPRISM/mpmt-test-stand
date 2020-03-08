#include "TestStandCommHost.h"

TestStandCommHost::TestStandCommHost(SerialDevice *device) : TestStandComm(device)
{
    // Nothing else to do
}

bool TestStandCommHost::ping()
{
    return this->send_empty_msg(MSG_ID_PING);
}

bool TestStandCommHost::get_status()
{
    return this->send_empty_msg(MSG_ID_STATUS);
}

bool TestStandCommHost::home()
{
    return this->send_empty_msg(MSG_ID_HOME);
}

bool TestStandCommHost::move(uint16_t accel, uint16_t hold_vel, uint16_t dist, uint8_t axis, uint8_t dir)
{
    // TODO
    return false;
}

bool TestStandCommHost::stop()
{
    return this->send_empty_msg(MSG_ID_STOP);
}

bool TestStandCommHost::get_data(DataId data_id)
{
    Message msg = {
        .id = MSG_ID_GET_DATA,
        .length = 1,
        .data = (uint8_t *)&data_id
    };
    return this->session.send_message(&msg);
}