#include "SerialTransport.h"

SerialTransport::SerialTransport(SerialDevice *device)
{
    this->device = device;
}

void SerialTransport::start(uint32_t baud_rate)
{
    this->device->open(baud_rate);
    this->pending_message.current_segment = MSG_SEG_START;
}

bool SerialTransport::check_for_message(Message *msg)
{
    if (this->device->available() > 0) {
        // Wait for buffer to fill
        // delay(3);
        uint32_t avail = this->device->available();

        uint8_t byte_in;
        while (avail > 0) {
            // Read next byte in
            byte_in = this->device->read();
            avail--;

            switch (this->pending_message.current_segment) {
                case MSG_SEG_START:
                    if (byte_in == MSG_DELIM) {
                        this->pending_message.current_segment = MSG_SEG_ID;
                    }
                    break;
                case MSG_SEG_ID:
                    msg->id = byte_in;
                    this->pending_message.current_segment = MSG_SEG_LENGTH;
                    break;
                case MSG_SEG_LENGTH:
                    msg->length = byte_in;
                    this->pending_message.msg_length = byte_in;
                    this->pending_message.bytes_read = 0;
                    this->pending_message.current_segment = (byte_in == 0 ? MSG_SEG_CRC : MSG_SEG_DATA);
                    break;
                case MSG_SEG_DATA:
                    msg->data[this->pending_message.bytes_read] = byte_in;
                    this->pending_message.bytes_read++;

                    if (this->pending_message.bytes_read == this->pending_message.msg_length) {
                        this->pending_message.bytes_read = 0;
                        this->pending_message.current_segment = MSG_SEG_CRC;
                    }
                    break;
                case MSG_SEG_CRC:
                    ((uint8_t *)&(this->pending_message.crc))[this->pending_message.bytes_read] = byte_in;
                    this->pending_message.bytes_read++;
                    if (this->pending_message.bytes_read == sizeof(this->pending_message.crc)) {
                        this->pending_message.bytes_read = 0;
                        // TODO CRC check
                        this->pending_message.current_segment = MSG_SEG_END;
                    }
                    break;
                case MSG_SEG_END:
                    if (byte_in == MSG_DELIM) {
                        this->pending_message.current_segment = MSG_SEG_START;
                        return true;
                    }
                default:
                    break;
            }
        }
    }
    return false;
}

bool SerialTransport::recv_message(Message *msg)
{
    while (!this->check_for_message(msg)) {}
    return true;
}

bool SerialTransport::send_message(Message *msg)
{
    uint8_t byte_out;

    // START
    byte_out = MSG_DELIM;
    if (!this->device->write(&byte_out, 1)) return false;

    // MESSAGE
    if (!this->device->write(&(msg->id), sizeof(msg->id))) return false;
    if (!this->device->write(&(msg->length), sizeof (msg->length))) return false;
    if (!this->device->write(msg->data, msg->length)) return false;

    // TODO CRC
    byte_out = 0;
    if (!this->device->write(&byte_out, 1)) return false;
    if (!this->device->write(&byte_out, 1)) return false;

    // END
    byte_out = MSG_DELIM;
    if (!this->device->write(&byte_out, 1)) return false;

    return true;
}