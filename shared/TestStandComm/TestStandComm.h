#ifndef TEST_STAND_COMM_H
#define TEST_STAND_COMM_H

#include "SerialDevice.h"
#include "SerialTransport.h"
#include "SerialSession.h"

#include <stddef.h>

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
        uint8_t send_buf[MSG_DATA_LENGTH_MAX];

    public:
        TestStandComm(SerialDevice& device);

        SerialResult ping();
        SerialResult echo(uint8_t *data, uint8_t length);
        SerialResult recv_echo();

        SerialResult link_check(uint32_t timeout_ms);

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

#ifndef __FLOAT_WORD_ORDER__
#error "__FLOAT_WORD_ORDER__ must be defined"
#endif

#if __SIZEOF_DOUBLE__ != 8
#error "Invalid __SIZEOF_DOUBLE__, must be equal to 8"
#endif // __SIZEOF_DOUBLE__ != 8

static double inline reverse(double val)
{
    double out;

    uint8_t *ptr_val = (uint8_t *)&val;
    uint8_t *ptr_out = (uint8_t *)&out;
    for (size_t i = 0; i < sizeof(double); i++) {
        ptr_out[i] = ptr_val[sizeof(double) - i - 1];
    }
    return out;
}

double inline ntohd(double val)
{
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
    return val;
#else // __ORDER_LITTLE_ENDIAN__
    return reverse(val);
#endif // __ORDER_LITTLE_ENDIAN__
}

double inline htond(double val)
{
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
    return val;
#else // __ORDER_LITTLE_ENDIAN__
    return reverse(val);
#endif // __ORDER_LITTLE_ENDIAN__
}

#endif // TEST_STAND_COMM_H