#ifndef LIMIT_SWITCH_H
#define LIMIT_SWITCH_H

#include "Arduino.h"

typedef enum {
    PRESSED = LOW,
    RELEASED = HIGH
} LimitSwitchStatus;

typedef struct LimitSwitch
{
    uint32_t pin;
    LimitSwitchStatus status;
} LimitSwitch;

#endif // LIMIT_SWITCH_H