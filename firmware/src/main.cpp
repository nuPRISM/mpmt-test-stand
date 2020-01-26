#include <Arduino.h>

#define MICROSTEP     1
#define RPS           1
uint32_t freq = (200 * RPS / MICROSTEP);
uint32_t TIMER_RST_VAL = (1E6 / freq / 2);


void setup()
{
    PMC->PMC_PCER0 |= PMC_PCER0_PID12;                        // PIOB power ON
    // PIOB->PIO_OER |= PIO_OER_P27;
    // PIOB->PIO_OWER |= PIO_OWER_P27;                           // Built In LED output write enable
    PIOC->PIO_OER  |= PIO_OER_P24;
    PIOC->PIO_OWER |= PIO_OWER_P24;

    /*************  Timer Counter 0 Channel 0 to generate PWM pulses thru TIOA0  ************/
    PMC->PMC_PCER0 |= PMC_PCER0_PID27;                        // TC0 power ON - Timer Counter 0 channel 0 IS TC0

    PIOB->PIO_PDR |= PIO_PDR_P25;                             // The pin is no more driven by the GPIO
    PIOB->PIO_ABSR |= PIO_PB25B_TIOA0;                        // TIOA0 (pin 2) is PB25 peripheral type B

    TC0->TC_CHANNEL[0].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1    // MCK/2, clk on rising edge
                              | TC_CMR_WAVE                   // Waveform mode
                              | TC_CMR_WAVSEL_UP_RC           // UP mode with automatic trigger on RC Compare
                              | TC_CMR_ACPA_CLEAR             // Clear TIOA0 on RA compare match
                              | TC_CMR_ACPC_SET;              // Set TIOA0 on RC compare match

    TC0->TC_CHANNEL[0].TC_RC = 42;  //<*********************  Frequency = (Mck/2)/TC_RC  Hz = 1 MHz
    TC0->TC_CHANNEL[0].TC_RA = 5;  //<********************   Any Duty cycle between 1 and TC_RC, Duty cycle = (TC_RA/TC_RC) * 100  %

    TC0->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;                  // Interrupt on RC compare match
    NVIC_EnableIRQ(TC0_IRQn);                                 // TC1 Interrupt enable

    TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_SWTRG | TC_CCR_CLKEN;  // Software trigger TC0 counter and enable
}

void TC0_Handler()
{
    static uint32_t Count = TIMER_RST_VAL;
    TC0->TC_CHANNEL[0].TC_SR;                                 // Read and clear status register
    Count--;
    if (Count == 0) {
        // PIOB->PIO_ODSR ^= PIO_ODSR_P27;                       // Toggle LED with a 1 Hz frequency
        PIOC->PIO_ODSR ^= PIO_ODSR_P24;
        Count = TIMER_RST_VAL;
    }
}

void loop()
{
  // put your main code here, to run repeatedly:
}