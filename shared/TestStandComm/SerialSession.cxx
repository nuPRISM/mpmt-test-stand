#include "SerialSession.h"

#define ACK_TIMEOUT_MS 500

SerialSession::SerialSession(SerialTransport& transport, Message& received_msg) : received_msg(received_msg), transport(transport)
{
    // Nothing else to do
}

void SerialSession::ack()
{
    Message ack = {
        .id = MSG_ID_ACK,
        .length = 0,
        .data = nullptr
    };
    this->transport.send_message(ack);
}

bool SerialSession::check_for_message()
{
    if (this->transport.check_for_message(this->received_msg)) {
        if (this->received_msg.id != MSG_ID_ACK && this->received_msg.id != MSG_ID_NACK) {
            // If we received an ACK or a NACK, don't ACK back
            // Real ACKs should have been consumed in send_message
            // An ACK at this stage means a message was missed somewhere already
            this->ack();
            return true;
        }
    }
    return false;
}

bool SerialSession::recv_message(uint32_t timeout_ms)
{
    // Cannot try to receive a full message while a partial one is in progress
    if (this->transport.msg_in_progress) return false;

    // Continually check for messages until a full one is received
    if (this->transport.recv_message(this->received_msg, timeout_ms)) {
        if (this->received_msg.id != MSG_ID_ACK && this->received_msg.id != MSG_ID_NACK) {
            // If we received an ACK or a NACK, don't ACK back
            // Real ACKs should have been consumed in send_message
            // An ACK at this stage means a message was missed somewhere already
            this->ack();
            return true;
        }
    }
    return false;
}

bool SerialSession::send_message(Message& msg)
{
    // Cannot send a message while receiving a message is in progress
    if (this->transport.msg_in_progress) return false;

    // Send the message
    if (!this->transport.send_message(msg)) return false;

    // Wait for ACK
    if (!this->transport.recv_message(this->received_msg, ACK_TIMEOUT_MS)) return false;
    if (this->received_msg.id != MSG_ID_ACK) return false;

    return true;
}
