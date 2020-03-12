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
    DIR_POSITIVE = 0,
    DIR_NEGATIVE = 1
} DirectionId;

typedef enum {
    AXIS_X = 0,
    AXIS_Y = 1
} AxisId;

typedef enum {
    DATA_TEMP = 0,
    DATA_MOTOR = 1
} DataId;

typedef enum {
    LL_DEBUG = 0,
    LL_INFO = 1,
    LL_WARNING = 2,
    LL_ERROR = 3,
    LL_CRITICAL = 4
} LogLevel;

typedef enum {
    STATUS_IDLE = 0,
    STATUS_HOMING = 1,
    STATUS_MOVING = 2,
    STATUS_MEASURING = 3,
    STATUS_LIMIT_REACHED = 4
} Status;

#endif // MESSAGES_H