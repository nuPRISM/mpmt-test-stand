#include "TestStandComm.h"

TestStandComm::TestStandComm(SerialDevice *device) : device(device), transport(device), session(&transport)
{
    // Nothing else to do
}

bool TestStandComm::ping()
{
    Message msg = {
        .id = MSG_ID_PING,
        .length = 0,
        .data = nullptr
    };
    
    return this->session.send_message(&msg);
}

bool TestStandComm::get_status(uint8_t *status_resp)
{
    return false;
}

bool TestStandComm::home()
{
    return false;
}

bool TestStandComm::move(uint16_t accel, uint16_t hold_vel, uint16_t dist, uint8_t axis, uint8_t dir)
{
    return false;
}

bool TestStandComm::stop()
{
    return false;
}

bool TestStandComm::get_data(uint8_t data_id)
{
    return false;
}
