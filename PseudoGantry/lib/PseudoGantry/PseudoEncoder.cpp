#include "PseudoEncoder.h"

void set_up_encoder(PseudoEncoder *encoder, void (*isr_encoder)(void)) 
{
    pinMode(encoder->motor_pulse_pin, INPUT_PULLUP);
    pinMode(encoder->channel_a_out, OUTPUT);
    digitalWrite(encoder->channel_a_out, LOW);
    attachInterrupt(digitalPinToInterrupt(encoder->motor_pulse_pin), isr_encoder, CHANGE);
}