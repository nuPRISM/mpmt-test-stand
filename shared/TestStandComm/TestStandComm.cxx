#include "TestStandComm.h"

TestStandComm::TestStandComm(SerialDevice *device) : device(device), transport(device), session(&transport, &(this->received_msg))
{
    this->received_msg.data = this->received_data;
}

bool TestStandComm::send_empty_msg(uint8_t id)
{
    Message msg = {
        .id = id,
        .length = 0,
        .data = nullptr
    };
    return this->session.send_message(&msg);
}

bool TestStandComm::check_for_message()
{
    return this->session.check_for_message();
}

bool TestStandComm::recv_message(uint32_t timeout_ms)
{
    return this->session.recv_message(timeout_ms);
}
