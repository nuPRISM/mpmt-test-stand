#ifndef SERIAL_TRANSPORT_H
#define SERIAL_TRANSPORT_H

#include "SerialDevice.h"
#include "Messages.h"

typedef enum {
    MSG_SEG_START,
    MSG_SEG_ID,
    MSG_SEG_LENGTH,
    MSG_SEG_DATA,
    MSG_SEG_CRC,
    MSG_SEG_END
} MessageSegment;

typedef struct {
    MessageSegment current_segment;
    uint8_t bytes_read;
    uint8_t msg_length;
    uint16_t crc;
} PendingMessage;

class SerialTransport
{
    public:
        SerialTransport(SerialDevice *device);
        void start(uint32_t baud_rate);
        bool check_for_message(Message *msg);
        bool recv_message(Message *msg);
        bool send_message(Message *msg);

    private:
        SerialDevice *device;
        PendingMessage pending_message;
};

#endif // SERIAL_COMM_H