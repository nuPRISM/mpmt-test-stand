#ifndef LIMITSWITCH_H
#define LIMITSWITCH_H

#include "Arduino.h"

typedef struct LimitSwitch
{
    uint32_t pin;
    int status;
} LimitSwitch;

#endif