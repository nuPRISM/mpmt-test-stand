#include "Timer.h"

// Reference: https://forum.arduino.cc/index.php?topic=130423.15

/**
 * \page clocks Arduino Due Timer Clock Sources
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

void configure_pwm_timer(Tc *tc, uint32_t channel, IRQn_Type irq, Pio *pio_bank, EPioType periph, uint32_t pin_mask)
{
    pmc_set_writeprotect(false);
    pmc_enable_periph_clk((uint32_t)irq);

    // TC_CMR_WAVE         : Use Waveform Mode i.e. generate PWM signal
    // TC_CMR_WAVSEL_UP_RC : Counter increments from 0 up to RC then resets
    // TIMER_CLOCK_SOURCE  : Set clock source for timer (see table above)
    // TC_CMR_ACPA_SET     : When counter = RA, TIOA -> 1
    // TC_CMR_ACPC_CLEAR   : When counter = RC, TIOA -> 0
    TC_Configure(tc, channel,
        TC_CMR_WAVE         |
        TC_CMR_WAVSEL_UP_RC |
        TIMER_CLOCK_SOURCE  |
        TC_CMR_ACPA_SET     |
        TC_CMR_ACPC_CLEAR
    );

    PIO_Configure(pio_bank, periph, pin_mask, PIO_DEFAULT);
}

void reset_pwm_timer(Tc *tc, uint32_t channel, uint32_t frequency)
{
    stop_pwm_timer(tc, channel);

    uint32_t rc = VARIANT_MCK / TIMER_CLOCK_DIVISOR / frequency;
    uint32_t ra = rc / 2; // 50% duty cycle
    TC_SetRA(tc, channel, ra);
    TC_SetRC(tc, channel, rc);

    TC_Start(tc, channel);
}

void stop_pwm_timer(Tc *tc, uint32_t channel)
{
    TC_Stop(tc, channel);
}

void configure_timer_interrupt(Tc *tc, uint32_t channel, IRQn_Type irq)
{
    pmc_set_writeprotect(false);
    pmc_enable_periph_clk((uint32_t)irq);

    // TC_CMR_WAVE         : Use Waveform Mode i.e. generate PWM signal
    // TC_CMR_WAVSEL_UP_RC : Counter increments from 0 up to RC then resets
    // TIMER_CLOCK_SOURCE  : Set clock source for timer (see table above)
    TC_Configure(tc, channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TIMER_CLOCK_SOURCE);

    // Enable RC compare interrupt
    tc->TC_CHANNEL[channel].TC_IER=TC_IER_CPCS;
    // Disable all other interrupts
    tc->TC_CHANNEL[channel].TC_IDR=~TC_IER_CPCS;
}

void reset_timer_interrupt(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t frequency)
{
    stop_timer_interrupt(tc, channel, irq);

    uint32_t rc = VARIANT_MCK / TIMER_CLOCK_DIVISOR / frequency;
    TC_SetRC(tc, channel, rc);

    NVIC_EnableIRQ(irq);
    TC_Start(tc, channel);
}

void stop_timer_interrupt(Tc *tc, uint32_t channel, IRQn_Type irq)
{
    NVIC_DisableIRQ(irq);
    TC_Stop(tc, channel);
}
