#include "PseudoEncoder.h"

void set_up_encoder(PseudoEncoder *encoder) 
{
    pinMode(encoder->motor_pulse_pin, INPUT_PULLUP);
    pinMode(encoder->channel_a_out, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(encoder->motor_pulse_pin), encoder->isr_encoder, RISING);
}