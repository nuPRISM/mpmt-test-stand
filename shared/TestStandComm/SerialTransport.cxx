#include "SerialTransport.h"

/**
 * @brief Constructs a new SerialTransport
 * 
 * @param device The underlying SerialDevice that will be used for transmitting / receiving data
 */
SerialTransport::SerialTransport(SerialDevice& device) : device(device)
{
    this->reset();
}

/**
 * @brief Resets the state of the receiver state machine
 * 
 * Any in progress message data is discarded.
 */
void SerialTransport::reset()
{
    this->pending_message.current_segment = MSG_SEG_START;
    this->pending_message.bytes_read = 0;
    this->pending_message.msg_length = 0;
    this->pending_message.crc = 0;

    this->msg_in_progress = false;
}

/**
 * @brief Checks if a full message has been received
 * 
 * This method is non-blocking. It will read the currently available serial data and
 * process it into a partial or fully complete Message.
 * Sequential calls to this method will continue to build off a previous partial message
 * until it is complete.
 * 
 * As soon as a message is complete, this method will return (even if there is more serial data
 * available), so a subsequent call will be required to process the leftover data.
 * 
 * This method is intended to be called in a loop so the serial buffer is regularly cleared
 * and full Messages are identified as they arrive.
 * 
 * @param msg As serial data comes in, the processed fields will be placed into
 *            this Message reference
 * 
 * @return true if a full message has been received (msg will be complete), otherwise false
 */
bool SerialTransport::check_for_message(Message& msg)
{
    uint8_t byte_in;
    while (this->device.ser_available() > 0) {
        // Read next byte in
        if (!this->device.ser_read(&byte_in)) break;

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
                this->reset();
                if (byte_in == MSG_DELIM_END) {
                    // Stop processing serial data as soon as we've read in a full message
                    return true;
                }
                break;
            default:
                break;
        }
    }

    return false;
}

/**
 * @brief Waits until a full message has been received
 * 
 * This is blocking version of check_for_message that will repeatedly call
 * check_for_message until a full message has been received or the timeout has elapsed.
 * 
 * @param msg        The received Message will be placed here
 * @param timeout_ms The timeout in milliseconds
 * 
 * @return true if a full message was received before the timeout, otherwise false
 */
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

/**
 * @brief Sends a message
 * 
 * @param msg The message to send
 * 
 * @return true if the entirety of the message was successfully sent
 *         false if any part of the message failed to send
 */
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