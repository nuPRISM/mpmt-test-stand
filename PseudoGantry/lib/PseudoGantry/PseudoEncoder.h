#ifndef PSEUDO_ENCODER_H
#define PSEUDO_ENCODER_H

#include "Arduino.h"

typedef struct PseudoEncoder
{
    uint32_t motor_pulse_pin;
    volatile uint32_t channel_a_out;
} PseudoEncoder;


void set_up_encoder(PseudoEncoder *encoder, void (*isr_encoder)(void));

#endif // PSEUDO_ENCODER_H