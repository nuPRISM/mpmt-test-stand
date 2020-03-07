#include "SerialSession.h"

SerialSession::SerialSession(SerialTransport *transport)
{
    this->transport = transport;
    this->received_msg.data = this->received_data;
}

bool SerialSession::check_for_message()
{
    bool ret = this->transport->check_for_message(&(this->received_msg));

    // Send back an ACK if we've received a message
    if (ret) {
        Message ack = {
            .id = MSG_ID_ACK,
            .length = 0,
            .data = nullptr
        };
        this->transport->send_message(&ack);
    }

    return ret;
}

bool SerialSession::recv_message()
{
    // Cannot try to receive a full message while a partial one is in progress
    if (this->transport->msg_in_progress) return false;

    // Continually check for messages until a full one is received
    while (!this->check_for_message());
    return true;
}

bool SerialSession::send_message(Message *msg)
{
    // Cannot send a message while receiving a message is in progress
    if (this->transport->msg_in_progress) return false;

    // Send the message
    if (!this->transport->send_message(msg)) return false;

    // Wait for ACK
    while (!this->transport->check_for_message(&(this->received_msg)));
    if (this->received_msg.id != MSG_ID_ACK) return false;

    return true;
}
