#ifndef SERIAL_RESULT_H
#define SERIAL_RESULT_H

/**
 * @enum SerialResult
 * 
 * @brief Possible outcomes of a serial communication operation
 */
typedef enum {
    // OKs
    SERIAL_OK,                  //!< Send or receive completed successfully
    SERIAL_OK_NO_MSG,           //!< No message was received, but that was expected
    // Errors
    SERIAL_ERR_NO_MSG,          //!< No message was received and one was expected
    SERIAL_ERR_TIMEOUT,         //!< A message was not received within the allotted timeout
    SERIAL_ERR_MSG_IN_PROGRESS, //!< A partial message has been received when another operation started
    SERIAL_ERR_SEND_FAILED,     //!< Message failed to send
    SERIAL_ERR_NO_ACK,          //!< After sending a message, the response was not an ACK
    SERIAL_ERR_ACK_FAILED,      //!< After receiving a message, failed to send an ACK
    SERIAL_ERR_WRONG_MSG        //!< An unexpected message was received
} SerialResult;

#endif // SERIAL_RESULT_H