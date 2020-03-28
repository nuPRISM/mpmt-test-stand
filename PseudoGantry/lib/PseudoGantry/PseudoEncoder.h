#ifndef PSEUDO_ENCODER_H
#define PSEUDO_ENCODER_H

#include "Arduino.h"

typedef struct PseudoEncoder
{
    uint32_t motor_pulse_pin;
    volatile uint32_t channel_a_out;
    void (*isr_encoder)(void);
} PseudoEncoder;


void set_up_encoder(PseudoEncoder *encoder) 
{
    pinMode(encoder->motor_pulse_pin, INPUT_PULLUP);
    pinMode(encoder->channel_a_out, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(encoder->motor_pulse_pin), encoder->isr_encoder, RISING);
}

#endif // PSEUDO_ENCODER_H