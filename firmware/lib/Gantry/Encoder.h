#ifndef ENCODER_H
#define ENCODER_H

#include "Arduino.h"

typedef struct Encoder
{
    uint32_t pin;
    volatile uint32_t current;
    uint32_t desired;
} Encoder;

#endif // ENCODER_H