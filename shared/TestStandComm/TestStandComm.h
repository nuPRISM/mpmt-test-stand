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
        SerialResult recv_message(uint8_t expect_id, uint8_t expect_length, uint32_t timeout_ms);
        Message& received_message();

};

/**
 * @brief Convert a uint32_t from network byte order (big endian) to host byte order
 */
int32_t inline ntohl(int32_t val)
{
    uint8_t *ptr = (uint8_t *)&val;
    return (ptr[0] << 24 |
            ptr[1] << 16 |
            ptr[2] <<  8 |
            ptr[3] <<  0 );
}

/**
 * @brief Convert a uint32_t from host byte order to network byte order (big endian)
 */
int32_t inline htonl(int32_t val)
{
    uint32_t out;
    uint8_t *ptr = (uint8_t *)&out;
    ptr[0] = (val >> 24) & 0xFF;
    ptr[1] = (val >> 16) & 0xFF;
    ptr[2] = (val >>  8) & 0xFF;
    ptr[3] = (val >>  0) & 0xFF;
    return out;
}

#endif // TEST_STAND_COMM_H