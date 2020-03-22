#include "SerialTransport.h"

SerialTransport::SerialTransport(SerialDevice& device) : device(device)
{
    this->reset();
}

void SerialTransport::reset()
{
    this->pending_message.current_segment = MSG_SEG_START;
    this->pending_message.bytes_read = 0;
    this->pending_message.msg_length = 0;
    this->pending_message.crc = 0;

    this->msg_in_progress = false;
}

bool SerialTransport::check_for_message(Message& msg)
{
    uint32_t avail;
    if ((avail = this->device.ser_available()) > 0) {
        uint8_t byte_in;
        while (avail > 0) {
            // Read next byte in
            byte_in = this->device.ser_read();
            avail--;

            switch (this->pending_message.current_segment) {
                case MSG_SEG_START:
                    if (byte_in == MSG_DELIM_START) {
                        this->msg_in_progress = true;
                        this->pending_message.current_segment = MSG_SEG_ID;
                    }
                    break;
                case MSG_SEG_ID:
                    msg.id = byte_in;
                    this->pending_message.current_segment = MSG_SEG_LENGTH;
                    break;
                case MSG_SEG_LENGTH:
                    msg.length = byte_in;
                    this->pending_message.msg_length = byte_in;
                    this->pending_message.bytes_read = 0;
                    this->pending_message.current_segment = (byte_in == 0 ? MSG_SEG_CRC : MSG_SEG_DATA);
                    break;
                case MSG_SEG_DATA:
                    msg.data[this->pending_message.bytes_read] = byte_in;
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
                    if (byte_in == MSG_DELIM_END) {
                        this->reset();
                        return true;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    return false;
}

bool SerialTransport::recv_message(Message& msg, uint32_t timeout_ms)
{
    uint32_t time_start = this->device.platform_millis();
    // Continually check for messages
    while (!this->check_for_message(msg)) {
        // Check if we've hit the timeout
        if ((this->device.platform_millis() - time_start) > timeout_ms) {
            // Abandon the message
            this->reset();
            return false;
        }
    }

    return true;
}

bool SerialTransport::send_message(Message& msg)
{
    uint8_t byte_out;

    // START
    byte_out = MSG_DELIM_START;
    if (!this->device.ser_write(&byte_out, 1)) return false;

    // MESSAGE
    if (!this->device.ser_write(&(msg.id), sizeof(msg.id))) return false;
    if (!this->device.ser_write(&(msg.length), sizeof (msg.length))) return false;
    if (!this->device.ser_write(msg.data, msg.length)) return false;

    // TODO CRC
    byte_out = 0;
    if (!this->device.ser_write(&byte_out, 1)) return false;
    if (!this->device.ser_write(&byte_out, 1)) return false;

    // END
    byte_out = MSG_DELIM_END;
    if (!this->device.ser_write(&byte_out, 1)) return false;

    return true;
}