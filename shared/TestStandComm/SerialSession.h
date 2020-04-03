#ifndef SERIAL_SESSION_H
#define SERIAL_SESSION_H

#include "SerialTransport.h"
#include "SerialResult.h"

/**
 * @class SerialSession
 * 
 * @brief Session layer of the serial communication protocol
 * 
 * This layer is responsible for sending/verifying ACK and NACK messages in response to
 * sending and receiving messages.
 */
class SerialSession
{
    private:
        Message& received_msg;
        SerialTransport& transport;

        bool ack();
        SerialResult check_received_msg();

    public:
        SerialSession(SerialTransport& transport, Message& received_msg);
        
        SerialResult check_for_message();
        SerialResult recv_message(uint32_t timeout_ms);
        SerialResult send_message(Message& msg);
};

#endif // SERIAL_SESSION_H