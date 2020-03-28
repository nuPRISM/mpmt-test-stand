#ifndef PSEUDO_LIMIT_SWITCH_H
#define PSEUDO_LIMIT_SWITCH_H

#include "Arduino.h"

typedef enum {
    PRESSED = LOW,
    UPRESSED = HIGH
} LimitSwitchStatus;

typedef struct PseudoLimitSwitch
{
    uint32_t current_position;
    uint32_t max_allowed_position;
} PseudoLimitSwitch;


#endif // PSEUDO_LIMIT_SWITCH_H