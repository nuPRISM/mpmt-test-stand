#ifndef SERIAL_TRANSPORT_H
#define SERIAL_TRANSPORT_H

#include "SerialDevice.h"
#include "Messages.h"

/**
 * @class SerialTransport
 * 
 * @brief Transport layer of the serial communication protocol
 * 
 * The layer is aware of "Messages" (distinct packets of data with a defined structure)
 * and is responsible for sending and receiving entire Messages.
 */
class SerialTransport
{
    private:
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

        SerialDevice& device;
        PendingMessage pending_message;

        void reset();

    public:
        bool msg_in_progress = false;

        SerialTransport(SerialDevice& device);
        
        bool check_for_message(Message& msg);
        bool recv_message(Message& msg, uint32_t timeout_ms);
        bool send_message(Message& msg);
};

#endif // SERIAL_TRANSPORT_H