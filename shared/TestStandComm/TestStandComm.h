#ifndef TEST_STAND_COMM_H
#define TEST_STAND_COMM_H

#include "SerialDevice.h"
#include "SerialTransport.h"
#include "SerialSession.h"

class TestStandComm
{
    private:
        uint8_t received_data[MSG_DATA_LENGTH_MAX];
        Message received_msg;

        SerialDevice& device;
        SerialTransport transport;

    protected:
        bool send_basic_msg(uint8_t id);
        SerialSession session;

    public:
        TestStandComm(SerialDevice& device);

        bool check_for_message();
        bool recv_message(uint32_t timeout_ms);
        Message& received_message();

};

#endif // TEST_STAND_COMM_H