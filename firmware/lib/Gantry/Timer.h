#ifndef TIMER_H
#define TIMER_H

#include "Arduino.h"

void start_timer(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t velocity);

void reset_timer(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t velocity);

void start_timer_accel(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t accel);

void reset_timer_accel(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t accel);

#endif // TIMER_H