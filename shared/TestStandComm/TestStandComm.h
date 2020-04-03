#ifndef TEST_STAND_COMM_H
#define TEST_STAND_COMM_H

#include "SerialDevice.h"
#include "SerialTransport.h"
#include "SerialSession.h"

/**
 * @class TestStandComm
 * 
 * @brief Application layer of the serial communication protocol
 * 
 * This layer is responsible for single and multi-message exchanges between devices
 * and interpreting / constructing data payloads for the messages.
 * 
 * This class is meant to be extended on each device with implementations for that device's
 * portion of the application layer.
 */
class TestStandComm
{
    private:
        uint8_t received_data[MSG_DATA_LENGTH_MAX];
        Message received_msg;

        SerialDevice& device;
        SerialTransport transport;

    protected:
        SerialResult send_basic_msg(uint8_t id);
        SerialSession session;

    public:
        TestStandComm(SerialDevice& device);

        SerialResult check_for_message();
        SerialResult recv_message(uint32_t timeout_ms);
        Message& received_message();

};

#endif // TEST_STAND_COMM_H