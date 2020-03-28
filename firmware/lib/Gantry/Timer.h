#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

void start_timer(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t frequency);
void reset_timer(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t frequency);

#endif // TIMER_H