#include "SerialSession.h"

/** Maximum time to wait to receive an ACK after sending a message (milliseconds) */
#define ACK_TIMEOUT_MS 100

/**
 * @brief Constructs a new SerialSession
 * 
 * @param transport The underlying SerialTransport to use for sending and receiving messages
 * @param received_msg A reference to a Message struct where received messages will be placed
 */
SerialSession::SerialSession(SerialTransport& transport, Message& received_msg) : received_msg(received_msg), transport(transport)
{
    // Nothing else to do
}

/**
 * @brief Transmits an ACK message
 */
bool SerialSession::ack()
{
    Message ack = {
        .id = MSG_ID_ACK,
        .length = 0,
        .data = nullptr
    };
    return this->transport.send_message(ack);
}

/**
 * @brief Checks if the received message was expected and sends an ACK if it was
 * 
 * @return SERIAL_OK             if the received message was expected
 *         SERIAL_ERR_ACK_FAILED if sending the ACK failed
 *         SERIAL_ERR_WRONG_MSG  if an unexpected message was received
 */
SerialResult SerialSession::check_received_msg()
{
    if (this->received_msg.id == MSG_ID_ACK || this->received_msg.id == MSG_ID_NACK) {
        // If we received an ACK or a NACK, don't ACK back
        // Real ACKs should have been consumed in send_message
        // An ACK at this stage means a message was missed somewhere already
        return SERIAL_ERR_WRONG_MSG;
    }
    else {
        if (!this->ack()) {
            return SERIAL_ERR_ACK_FAILED;
        }
        return SERIAL_OK;
    }
}

/**
 * @brief Checks if a full message has been received
 * 
 * This method is non-blocking, it will not wait for a message.
 * It will read the currently available serial data (if any) and if a full message has been received
 * it will be stored into the received_msg struct passed to the constructor.
 * 
 * If a full message was received, an ACK response will automatically be sent.
 * 
 * @return SERIAL_OK             if a full message was received and an ACK was sent
 *         SERIAL_ERR_ACK_FAILED if a full message was received but sending the ACK failed
 *         SERIAL_ERR_WRONG_MSG  if an unexpected ACK or NACK was received
 *         SERIAL_OK_NO_MSG      if no message was received
 */
SerialResult SerialSession::check_for_message()
{
    if (this->transport.check_for_message(this->received_msg)) {
        return this->check_received_msg();
    }
    return SERIAL_OK_NO_MSG;
}

/**
 * @brief Waits to receive a full message.
 * 
 * This method is blocking. It will continually check for serial data until a full message is received
 * or until timeout_ms has elapsed. If a message is received, an ACK will automatically be sent back.
 * 
 * If a partial message has been received when this method is called, it will not attempt to receive a full message.
 * 
 * @param timeout_ms Maximum time (in milliseconds) to wait for a message
 * 
 * @return SERIAL_OK                  if a full message was received and an ACK was sent
 *         SERIAL_ERR_MSG_IN_PROGRESS if a message is already in progress
 *         SERIAL_ERR_ACK_FAILED      if a full message was received but sending the ACK failed
 *         SERIAL_ERR_WRONG_MSG       if an unexpected ACK or NACK was received
 *         SERIAL_ERR_TIMEOUT         if no message was received
 */
SerialResult SerialSession::recv_message(uint32_t timeout_ms)
{
    // Cannot try to receive a full message while a partial one is in progress
    if (this->transport.msg_in_progress) return SERIAL_ERR_MSG_IN_PROGRESS;

    // Continually check for messages until a full one is received
    if (this->transport.recv_message(this->received_msg, timeout_ms)) {
        return this->check_received_msg();
    }
    return SERIAL_ERR_TIMEOUT;
}

/**
 * @brief Sends a message and waits to receive an ACK
 * 
 * If a partial message has been received when this method is called,
 * the message will not be sent.
 * 
 * @param msg A reference to the Message to send
 * 
 * @return SERIAL_OK                  if the message sent and an ACK was received
 *         SERIAL_ERR_MSG_IN_PROGRESS if a message is already in progress
 *         SERIAL_ERR_SEND_FAILED     if the message failed to be transmitted
 *         SERIAL_ERR_NO_MSG          if a response was not received after sending
 *         SERIAL_ERR_NO_ACK          if a response was received but it was not an ACK
 */
SerialResult SerialSession::send_message(Message& msg)
{
    // Cannot send a message while receiving a message is in progress
    if (this->transport.msg_in_progress) return SERIAL_ERR_MSG_IN_PROGRESS;

    // Send the message
    if (!this->transport.send_message(msg)) return SERIAL_ERR_SEND_FAILED;

    // Wait for ACK
    if (!this->transport.recv_message(this->received_msg, ACK_TIMEOUT_MS)) return SERIAL_ERR_NO_MSG;
    if (this->received_msg.id != MSG_ID_ACK) return SERIAL_ERR_NO_ACK;

    return SERIAL_OK;
}
