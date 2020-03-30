#include "Timer.h"

// Helpful Resource: https://forum.arduino.cc/index.php?topic=130423.15

/**
 * \page timers Arduino Due Timer Interrupt Configurations
 * 
 * | ISR/IRQ | TC  | Channel | Due Pins |
 * | ------- | --- | ------- | -------- |
 * | TC0     | TC0 | 0       | 2, 13    |
 * | TC1     | TC0 | 1       | 60, 61   |
 * | TC2     | TC0 | 2       | 58       |
 * | TC3     | TC1 | 0       | none     |
 * | TC4     | TC1 | 1       | none     |
 * | TC5     | TC1 | 2       | none     |
 * | TC6     | TC2 | 0       | 4, 5     |
 * | TC7     | TC2 | 1       | 3, 10    |
 * | TC8     | TC2 | 2       | 11, 12   |
 */

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

/**
 * @brief Configures a timer channel to drive a PWM-type signal on its TIOA output
 * 
 * An interrupt is also enabled which will fire at the PWM frequency
 * 
 * @param tc       Pointer to the timer counter peripheral
 * @param channel  Channel number within the TC
 * @param irq      IRQ number corresponding to the TC channel
 * @param pio      Pointer to the PIO instance for the TIOA pin
 * @param periph   Peripheral mode for the TIOA pin
 * @param pin_mask Bitmask for the TIOA pin within the PIO instance
 */
void configure_pwm_timer(Tc *tc, uint32_t channel, IRQn_Type irq, Pio *pio, EPioType periph, uint32_t pin_mask)
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

    PIO_Configure(pio, periph, pin_mask, PIO_DEFAULT);

    // Enable RC compare interrupt
    tc->TC_CHANNEL[channel].TC_IER=TC_IER_CPCS;
    // Disable all other interrupts
    tc->TC_CHANNEL[channel].TC_IDR=~TC_IER_CPCS;
}

/**
 * @brief Starts / restarts a timer outputting a PWM-type signal
 * 
 * @param tc                    Pointer to the timer counter peripheral
 * @param channel               Channel number within the TC
 * @param irq                   IRQ number corresponding to the TC channel
 * @param frequency             Frequency for the PWM signal
 * @param duty_cycle_on_percent Percentage of the period that the signal should be ON
 *                              (an integer between 0 and 100)
 */
void reset_pwm_timer(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t frequency, uint8_t duty_cycle_on_percent)
{
    stop_timer(tc, channel, irq);

    uint32_t rc = VARIANT_MCK / TIMER_CLOCK_DIVISOR / frequency;
    uint32_t ra = rc * (100 - duty_cycle_on_percent) / 100;
    TC_SetRA(tc, channel, ra);
    TC_SetRC(tc, channel, rc);

    NVIC_EnableIRQ(irq);
    TC_Start(tc, channel);
}

/**
 * @brief Configures a timer channel to generate periodic interrupts
 * 
 * @param tc      Pointer to the timer counter peripheral
 * @param channel Channel number within the TC
 * @param irq     IRQ number corresponding to the TC channel
 */
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

/**
 * @brief Starts / restarts a timer generating periodic interrupts and enables
 *        the corresponding interrupt
 * 
 * @param tc        Pointer to the timer counter peripheral
 * @param channel   Channel number within the TC
 * @param irq       IRQ number corresponding to the TC channel
 * @param frequency Frequency for the interrupts
 */
void reset_timer_interrupt(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t frequency)
{
    stop_timer(tc, channel, irq);

    uint32_t rc = VARIANT_MCK / TIMER_CLOCK_DIVISOR / frequency;
    TC_SetRC(tc, channel, rc);

    NVIC_EnableIRQ(irq);
    TC_Start(tc, channel);
}

/**
 * @brief Stops a timer and disables the corresponding interrupt
 * 
 * @param tc      Pointer to the timer counter peripheral
 * @param channel Channel number within the TC
 * @param irq     IRQ number corresponding to the TC channel
 */
void stop_timer(Tc *tc, uint32_t channel, IRQn_Type irq)
{
    NVIC_DisableIRQ(irq);
    TC_Stop(tc, channel);
}
