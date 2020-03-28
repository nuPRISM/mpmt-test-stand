/* **************************** Local Includes ***************************** */
#include "mPMTTestStand.h"
#include "Debug.h"

/* **************************** System Includes **************************** */
#include <Arduino.h>

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

const mPMTTestStandIO io = {
    // Serial Devices
    .serial_comm            = Serial,
    .serial_comm_baud_rate  = 115200,
    // Thermistor Pins
    .pin_therm_amb          = A0,
    .pin_therm_motor1       = A1,
    .pin_therm_mpmt         = A2,
    .pin_therm_motor2       = A3,
    .pin_therm_optical      = A4,
    // Gantry X-Axis Pins
    .io_axis_x = {
        // Step output will use TC2 Channel 0 which is mapped to IRQ TC6
        // TIOA output for this timer channel is on PC25 = Due pin 5
        .tc_step            = TC2,
        .tc_step_channel    = 0,
        .tc_step_irq        = TC6_IRQn,
        .pio_step_bank      = PIOC,
        .pio_step_periph    = PIO_PERIPH_B,
        .pio_step_pin_mask  = PIO_PC25B_TIOA6,
        .pin_dir            = 6, // PC24
        .pin_enc_a          = 7,
        .pin_enc_b          = 8,
        .pin_ls_home        = 9,
        .pin_ls_far         = 10,
    },
    // Gantry Y-Axis Pins
    .io_axis_y = {
        // Step output will use TC2 Channel 1 which is mapped to IRQ TC7
        // TIOA output for this timer channel is on PC28 = Due pin 3
        .tc_step            = TC2,
        .tc_step_channel    = 1,
        .tc_step_irq        = TC7_IRQn,
        .pio_step_bank      = PIOC,
        .pio_step_periph    = PIO_PERIPH_B,
        .pio_step_pin_mask  = PIO_PC28B_TIOA7,
        .pin_dir            = 23, // PA14
        .pin_enc_a          = 24,
        .pin_enc_b          = 25,
        .pin_ls_home        = 26,
        .pin_ls_far         = 27
    }
};

mPMTTestStand test_stand(io);

uint32_t blink_start;
bool blink_state;

void setup()
{
    DEBUG_INIT;
    pinMode(LED_BUILTIN, OUTPUT);

    test_stand.setup();

    blink_start = millis();
    blink_state = false;
}

void loop()
{
    test_stand.execute();

    // Blink an LED
    if ((millis() - blink_start) >= 500) {
        blink_state = !blink_state;
        digitalWrite(LED_BUILTIN, blink_state);
        blink_start = millis();
    }
}
