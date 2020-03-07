#ifndef SERIAL_SESSION_H
#define SERIAL_SESSION_H

#include "SerialTransport.h"

class SerialSession
{
    public:
        Message received_msg;

        SerialSession(SerialTransport *transport);
        
        bool check_for_message();
        bool recv_message();
        bool send_message(Message *msg);

    private:
        SerialTransport *transport;
        uint8_t received_data[MSG_DATA_LENGTH_MAX];

};

#endif // SERIAL_SESSION_H