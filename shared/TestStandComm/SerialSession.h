#ifndef SERIAL_SESSION_H
#define SERIAL_SESSION_H

#include "SerialTransport.h"

class SerialSession
{
    private:
        Message& received_msg;
        SerialTransport& transport;

        void ack();

    public:
        SerialSession(SerialTransport& transport, Message& received_msg);
        
        bool check_for_message();
        bool recv_message(uint32_t timeout_ms);
        bool send_message(Message& msg);
};

#endif // SERIAL_SESSION_H