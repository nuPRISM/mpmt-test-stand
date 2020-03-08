#ifndef SERIAL_TRANSPORT_H
#define SERIAL_TRANSPORT_H

#include "SerialDevice.h"
#include "Messages.h"

typedef enum {
    MSG_SEG_START    = 0,
    MSG_SEG_ID       = 1,
    MSG_SEG_LENGTH   = 2,
    MSG_SEG_DATA     = 3,
    MSG_SEG_CRC      = 4,
    MSG_SEG_END      = 5
} MessageSegment;

typedef struct {
    MessageSegment current_segment;
    uint8_t bytes_read;
    uint8_t msg_length;
    uint16_t crc;
} PendingMessage;

class SerialTransport
{
    private:
        SerialDevice *device;
        PendingMessage pending_message;

    public:
        bool msg_in_progress = false;

        SerialTransport(SerialDevice *device);
        
        bool check_for_message(Message *msg);
        bool recv_message(Message *msg, uint32_t timeout_ms);
        bool send_message(Message *msg);
};

#endif // SERIAL_COMM_H