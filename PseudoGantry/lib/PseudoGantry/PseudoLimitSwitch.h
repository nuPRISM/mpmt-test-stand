#ifndef PSEUDO_LIMIT_SWITCH_H
#define PSEUDO_LIMIT_SWITCH_H

#include "Arduino.h"

typedef enum {
    PRESSED = LOW,
    UNRESSED = HIGH
} LimitSwitchStatus;

typedef struct PseudoLimitSwitch
{
    uint32_t output_pin;
    uint32_t status;
} PseudoLimitSwitch;


#endif // PSEUDO_LIMIT_SWITCH_H