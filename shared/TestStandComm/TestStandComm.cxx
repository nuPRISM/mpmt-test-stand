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