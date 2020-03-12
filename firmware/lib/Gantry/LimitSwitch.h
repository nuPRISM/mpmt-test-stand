#ifndef LIMIT_SWITCH_H
#define LIMIT_SWITCH_H

#include "Arduino.h"

typedef struct LimitSwitch
{
    uint32_t pin;
    int status;
} LimitSwitch;

#endif // LIMIT_SWITCH_H