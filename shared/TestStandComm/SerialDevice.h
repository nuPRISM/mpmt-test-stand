#ifndef SERIAL_DEVICE_H
#define SERIAL_DEVICE_H

#include <stdint.h>

typedef enum {
    BAUD_9600   = 9600,
    BAUD_19200  = 19200,
    BAUD_38400  = 38400,
    BAUD_57600  = 57600,
    BAUD_115200 = 115200,
    BAUD_230400 = 230400,
    BAUD_460800 = 460800,
    BAUD_500000 = 500000
} SerialBaudRate;

/**
 * @class SerialDevice
 * 
 * @brief Abstract base class representing an interface to serial communication hardware
 *        on an arbitrary system
 * 
 * This is the lowest layer in the protocol stack and is only responsible for sending
 * bytes. It performs no interpretation of the data being sent.
 */
class SerialDevice
{
    public:
        /**
         * @brief Open a connection to the serial hardware
         * 
         * @param baud_rate The baud rate for the serial connection
         * 
         * @return true if the connection opened successfully, otherwise false
         */
        virtual bool ser_connect(SerialBaudRate baud_rate) = 0;

        /**
         * @brief Flush all data currently in the receive buffer of the serial device
         */
        virtual void ser_flush() = 0;

        /**
         * @brief Check how much data is currently available in the receive buffer
         *        of the serial device
         * 
         * @return The number bytes available to be read
         */
        virtual uint32_t ser_available() = 0;

        /**
         * @brief Read a single byte from the serial device
         * 
         * @param out Pointer to where the read byte will be stored
         * 
         * @return true if the bytes was read successfully, otherwise false
         */
        virtual bool ser_read(uint8_t *out) = 0;

        /**
         * @brief Transmits a buffer of data on the serial device
         * 
         * @param data   A pointer to the data buffer
         * @param length The number of bytes to transmit from the buffer
         * 
         * @return true if all the bytes were transmitted, otherwise false
         */
        virtual bool ser_write(uint8_t *data, uint32_t length) = 0;

        /**
         * @brief Closes an open connection to the serial hardware
         * 
         * Should not be called unless ser_connect has already been called and returned true
         */
        virtual void ser_disconnect() = 0;

        /**
         * @brief Returns a system timestamp (does not have to be calendar time) in milliseconds
         */
        virtual uint64_t platform_millis() = 0;
};

#endif // SERIAL_DEVICE_H