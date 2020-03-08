#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdint.h>

#define MSG_DATA_LENGTH_MAX  0xFF

#define MSG_DELIM_START 0x7B
#define MSG_DELIM_END   0x7D

/*****************************************************************************/
/*                                MESSAGE IDS                                */
/*****************************************************************************/

#define MSG_ID_INVALID     0x00

#define MSG_ID_ACK         0x01
#define MSG_ID_NACK        0x02

// PC -> Arduino Messages
#define MSG_ID_PING        0x40
#define MSG_ID_GET_STATUS  0x41
#define MSG_ID_HOME        0x42
#define MSG_ID_MOVE        0x43
#define MSG_ID_STOP        0x44
#define MSG_ID_GET_DATA    0x45

// Arduino -> PC Messages
#define MSG_ID_LOG         0x80
#define MSG_ID_STATUS      0x81
#define MSG_ID_DATA        0x82

typedef struct {
    uint8_t id;
    uint8_t length;
    uint8_t *data;
} Message;

typedef enum {
    POSITIVE = 0,
    NEGATIVE = 1
} Direction;

typedef enum {
    TEMP = 0,
    MOTOR = 1
} DataId;

typedef enum {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
} LogLevel;

typedef enum {
    IDLE = 0,
    HOMING = 1,
    MOVING = 2,
    MEASURING = 3,
    LIMIT_REACHED = 4
} Status;

#endif // MESSAGES_H