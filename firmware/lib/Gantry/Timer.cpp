#include "Timer.h"

// Reference: https://forum.arduino.cc/index.php?topic=130423.15

/**
 * \page Arduino Due Timer Clock Sources
 * 
 * | Name         | Definition |
 * | ------------ | ---------- |
 * | TIMER_CLOCK1 | MCK/2      |
 * | TIMER_CLOCK2 | MCK/8      |
 * | TIMER_CLOCK3 | MCK/32     |
 * | TIMER_CLOCK4 | MCK/128    |
 * | TIMER_CLOCK5 | SLCK       |
 */

#define TIMER_CLOCK_SOURCE  TC_CMR_TCCLKS_TIMER_CLOCK4
#define TIMER_CLOCK_DIVISOR 128

void start_timer(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t frequency)
{
    pmc_set_writeprotect(false);
    pmc_enable_periph_clk((uint32_t)irq);

    // TC_CMR_WAVE         : Use Waveform Mode i.e. generate PWM signal
    // TC_CMR_WAVSEL_UP_RC : Counter increments from 0 up to RC then resets
    TC_Configure(tc, channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TIMER_CLOCK_SOURCE);

    reset_timer(tc, channel, irq, frequency);
}

void reset_timer(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t frequency)
{   
    NVIC_DisableIRQ(irq);

    uint32_t rc = VARIANT_MCK / TIMER_CLOCK_DIVISOR / frequency;
    TC_SetRA(tc, channel, rc/2); //50% high, 50% low
    TC_SetRC(tc, channel, rc);
    TC_Start(tc, channel);

    // Enable RC compare interrupt
    tc->TC_CHANNEL[channel].TC_IER=TC_IER_CPCS;
    // Disable all other interrupts
    tc->TC_CHANNEL[channel].TC_IDR=~TC_IER_CPCS;

    NVIC_EnableIRQ(irq);
}
