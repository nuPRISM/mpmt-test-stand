#ifndef TEST_STAND_MESSAGES_H
#define TEST_STAND_MESSAGES_H

#include "Gantry.h"
#include <stdint.h>

/*****************************************************************************/
/*                                MESSAGE IDS                                */
/*****************************************************************************/

// Two-way Messages
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
#define MSG_ID_AXIS_RESULT      0x85

/*****************************************************************************/
/*                                   ENUMS                                   */
/*****************************************************************************/

typedef enum {
    LL_DEBUG,
    LL_INFO,
    LL_WARNING,
    LL_ERROR,
    LL_CRITICAL
} LogLevel;

/*****************************************************************************/
/*                               DATA STRUCTS                                */
/*****************************************************************************/

/**
 * NOTE: These structs are packed so that they have the same size on a 64-bit
 *       PC as on the 32-bit Arduino Due
 */

typedef struct {
    uint32_t vel_hold;
    uint32_t dist_counts;
    uint8_t axis;
    uint8_t dir;
} __attribute__((__packed__)) MoveMsgData;

typedef struct {
    int32_t x_counts;
    int32_t y_counts;
} __attribute__((__packed__)) PositionMsgData;

#endif // TEST_STAND_MESSAGES_H