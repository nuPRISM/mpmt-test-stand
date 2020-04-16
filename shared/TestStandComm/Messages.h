#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdint.h>

#define MSG_DATA_LENGTH_MAX  0xFF

#define MSG_DELIM_START      0x7B
#define MSG_DELIM_END        0x7D

#define MSG_ID_INVALID       0x00

#define MSG_ID_ACK           0x01
#define MSG_ID_NACK          0x02

#define MSG_ID_PING          0x10
#define MSG_ID_ECHO          0x11
#define MSG_ID_ECHOED        0x12

typedef struct {
    uint8_t id;
    uint8_t length;
    uint8_t *data;
} Message;

#endif // MESSAGES_H
