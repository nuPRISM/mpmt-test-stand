#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

// PWM Timers
void configure_pwm_timer(Tc *tc, uint32_t channel, IRQn_Type irq, Pio *pio_bank, EPioType periph, uint32_t pin_mask);
void reset_pwm_timer(Tc *tc, uint32_t channel, uint32_t frequency);
void stop_pwm_timer(Tc *tc, uint32_t channel);

// Timer Interrupts
void configure_timer_interrupt(Tc *tc, uint32_t channel, IRQn_Type irq);
void reset_timer_interrupt(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t frequency);
void stop_timer_interrupt(Tc *tc, uint32_t channel, IRQn_Type irq);

#endif // TIMER_H