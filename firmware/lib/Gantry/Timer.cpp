#include "Timer.h"

void start_timer(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t velocity)
{
    pmc_set_writeprotect(false);
    pmc_enable_periph_clk((uint32_t)irq);
    TC_Configure(tc, channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC
                                          |TC_CMR_TCCLKS_TIMER_CLOCK4);
    uint32_t rc = VARIANT_MCK/128/velocity/2; // over 2 because need to toggle down
    TC_SetRA(tc, channel, rc/2); //50% high, 50% low
    TC_SetRC(tc, channel, rc);
    TC_Start(tc, channel);
    tc->TC_CHANNEL[channel].TC_IER=TC_IER_CPCS;
    tc->TC_CHANNEL[channel].TC_IDR=~TC_IER_CPCS;
    NVIC_EnableIRQ(irq);
}

void reset_timer(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t velocity)
{   
    NVIC_DisableIRQ(irq);
    uint32_t rc = VARIANT_MCK/128/velocity/2; // over 2 because need to toggle down
    TC_SetRA(tc, channel, rc/2); //50% high, 50% low
    TC_SetRC(tc, channel, rc);
    TC_Start(tc, channel);
    tc->TC_CHANNEL[channel].TC_IER=TC_IER_CPCS;
    tc->TC_CHANNEL[channel].TC_IDR=~TC_IER_CPCS;
    NVIC_EnableIRQ(irq);
}

void start_timer_accel(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t accel)
{
    pmc_set_writeprotect(false);
    pmc_enable_periph_clk((uint32_t)irq);
    TC_Configure(tc, channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC
                                          |TC_CMR_TCCLKS_TIMER_CLOCK4);
    uint32_t rc = VARIANT_MCK/128/accel; // over 2 because need to toggle down
    TC_SetRA(tc, channel, rc/2); //50% high, 50% low
    TC_SetRC(tc, channel, rc);
    TC_Start(tc, channel);
    tc->TC_CHANNEL[channel].TC_IER=TC_IER_CPCS;
    tc->TC_CHANNEL[channel].TC_IDR=~TC_IER_CPCS;
    NVIC_EnableIRQ(irq);
}

void reset_timer_accel(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t accel)
{   
    NVIC_DisableIRQ(irq);
    uint32_t rc = VARIANT_MCK/128/accel;
    TC_SetRA(tc, channel, rc/2); //50% high, 50% low
    TC_SetRC(tc, channel, rc);
    TC_Start(tc, channel);
    tc->TC_CHANNEL[channel].TC_IER=TC_IER_CPCS;
    tc->TC_CHANNEL[channel].TC_IDR=~TC_IER_CPCS;
    NVIC_EnableIRQ(irq);
}