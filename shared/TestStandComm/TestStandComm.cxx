#include "TestStandComm.h"

/**
 * @brief Constructs a new TestStandComm
 * 
 * @param device The SerialDevice to communicate with
 * 
 * A SerialSession and SerialTransport will be internally instantiated to fill in the middle layers
 * of the protocol stack
 */
TestStandComm::TestStandComm(SerialDevice& device) : device(device), transport(device), session(transport, this->received_msg)
{
    this->received_msg.data = this->received_data;
}

/**
 * @brief Helper method to send a "basic message"
 * 
 * A "basic message" is a message with just an ID and no payload data.
 * 
 * @param id The message ID
 * 
 * @return @see SerialSession::send_message(Message& msg)
 */
SerialResult TestStandComm::send_basic_msg(uint8_t id)
{
    Message msg = {
        .id = id,
        .length = 0,
        .data = nullptr
    };
    return this->session.send_message(msg);
}

/**
 * @brief Sends a ping message
 * 
 * @return @see send_basic_msg(uint8_t id)
 */
SerialResult TestStandComm::ping()
{
    return this->send_basic_msg(MSG_ID_PING);
}

SerialResult TestStandComm::echo(uint8_t *data, uint8_t length)
{
    Message msg = {
        .id = MSG_ID_ECHO,
        .length = length,
        .data = data
    };

    return this->session.send_message(msg);
}

/**
 * @brief Sends an ECHO message filled with every byte value to verify the serial link is working
 * 
 * @param timeout_ms Maximum time (in milliseconds) to wait for a response
 * 
 * @return @see SerialSession::send_message(Message& msg)
 */
SerialResult TestStandComm::link_check(uint32_t timeout_ms)
{
    int i;

    // Fill send buffer with every value between 0 and MSG_DATA_LENGTH_MAX
    for (i = 0; i < MSG_DATA_LENGTH_MAX; i++) {
        this->send_buf[i] = i;
    }

    // Send ECHO
    SerialResult res = this->echo(this->send_buf, MSG_DATA_LENGTH_MAX);
    if (res != SERIAL_OK) return res;

    // Receive ECHOED
    res = this->recv_message(MSG_ID_ECHOED, MSG_DATA_LENGTH_MAX, timeout_ms);
    if (res != SERIAL_OK) return res;

    // Verify data
    for (i = 0; i < MSG_DATA_LENGTH_MAX; i++) {
        if (this->received_message().data[i] != i) return SERIAL_ERR_DATA_CORRUPT;
    }

    return SERIAL_OK;
}

/**
 * @brief Handles receiving an ECHO message by sending the same data back in an ECHOED message
 * 
 * @return @see SerialSession::send_message(Message& msg)
 */
SerialResult TestStandComm::recv_echo()
{
    // Send the exact same data back
    Message msg = {
        .id = MSG_ID_ECHOED,
        .length = this->received_message().length,
        .data = this->received_message().data
    };

    return this->session.send_message(msg);
}

/**
 * @brief Wrapper around @see SerialSession::check_for_message()
 */
SerialResult TestStandComm::check_for_message()
{
    return this->session.check_for_message();
}

/**
 * @brief Wrapper around @see SerialSession::recv_message(uint32_t timeout_ms)
 * 
 * @param expect_id     The expected ID of the message being received
 * @param expect_length The expected data length of the message being received
 * @param timeout_ms    Maximum time (in milliseconds) to wait for a message
 * 
 * @return @see SerialSession::recv_message(uint32_t timeout_ms)
 *         SERIAL_ERR_WRONG_MSG if the received ID does not match expect_id
 *         SERIAL_ERR_DATA_LENGTH if the received data length does not match expect_length
 */
SerialResult TestStandComm::recv_message(uint8_t expect_id, uint8_t expect_length, uint32_t timeout_ms)
{
    SerialResult res = this->session.recv_message(timeout_ms);
    if (res != SERIAL_OK) return res;

    if (this->received_message().id != expect_id) return SERIAL_ERR_WRONG_MSG;
    if (this->received_message().length != expect_length) return SERIAL_ERR_DATA_LENGTH;
    return SERIAL_OK;
}

/**
 * @return A reference to the Message struct holding the most recently received message
 */
Message& TestStandComm::received_message()
{
    return this->received_msg;
}