#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdint.h>

#define MSG_DATA_LENGTH_MAX     0xFF

#define MSG_DELIM_START         0x7B
#define MSG_DELIM_END           0x7D

/*****************************************************************************/
/*                                MESSAGE IDS                                */
/*****************************************************************************/

#define MSG_ID_INVALID          0x00

#define MSG_ID_ACK              0x01
#define MSG_ID_NACK             0x02

#define MSG_ID_PING             0x03

// PC -> Arduino Messages
#define MSG_ID_GET_STATUS       0x40
#define MSG_ID_HOME             0x41
#define MSG_ID_MOVE             0x42
#define MSG_ID_STOP             0x43
#define MSG_ID_GET_POSITION     0x44
#define MSG_ID_GET_AXIS_STATE   0x45
#define MSG_ID_GET_TEMP         0x46

// Arduino -> PC Messages
#define MSG_ID_LOG              0x80
#define MSG_ID_STATUS           0x81
#define MSG_ID_POSITION         0x82
#define MSG_ID_AXIS_STATE       0x83
#define MSG_ID_TEMP             0x84

typedef enum {
    LL_DEBUG,
    LL_INFO,
    LL_WARNING,
    LL_ERROR,
    LL_CRITICAL
} LogLevel;

typedef struct {
    uint8_t id;
    uint8_t length;
    uint8_t *data;
} Message;

#endif // MESSAGES_H
