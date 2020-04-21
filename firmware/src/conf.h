#ifndef CONF_H
#define CONF_H

#include "mPMTTestStand.h"
#include "Timer.h"

const mPMTTestStandConfig conf = {
    // Serial Devices
    .serial_comm            = Serial,
    .serial_comm_baud_rate  = SERIAL_BAUD_RATE,
    // Gantry X-Axis Pins
    .io_axis_x = {
        // Step output will use TC2 Channel 0 which is mapped to IRQ TC6
        // TIOA output for this timer channel is on PC25 = Due pin 5
        .tc_step            = TC2,
        .tc_step_channel    = 0,
        .tc_step_irq        = TC_IRQN(AXIS_X_STEP_TC_IRQ), // NOTE: AXIS_X_STEP_TC_IRQ is defined in platformio.ini
        .pio_step           = PIOC,
        .pio_step_periph    = PIO_PERIPH_B,
        .pio_step_pin_mask  = PIO_PC25B_TIOA6, // Due pin 5
        .pin_dir            = 6,
        .pin_enc_a          = 7,
        .pin_enc_b          = 8,
        .pin_ls_home        = 9,
        .pin_ls_far         = 10,
        .dir_pos_level      = LOW,
        .ls_pressed_level   = LOW
    },
    // Gantry Y-Axis Pins
    .io_axis_y = {
        // Step output will use TC2 Channel 1 which is mapped to IRQ TC7
        // TIOA output for this timer channel is on PC28 = Due pin 3
        .tc_step            = TC2,
        .tc_step_channel    = 1,
        .tc_step_irq        = TC_IRQN(AXIS_Y_STEP_TC_IRQ), // NOTE: AXIS_Y_STEP_TC_IRQ is defined in platformio.ini
        .pio_step           = PIOC,
        .pio_step_periph    = PIO_PERIPH_B,
        .pio_step_pin_mask  = PIO_PC28B_TIOA7, // Due pin 3
        .pin_dir            = 23, // PA14
        .pin_enc_a          = 24,
        .pin_enc_b          = 25,
        .pin_ls_home        = 26,
        .pin_ls_far         = 27,
        .dir_pos_level      = LOW,
        .ls_pressed_level   = LOW
    },
    .axis_mech = {
        .counts_per_rev     = ENCODER_COUNTS_PER_REV,
        .steps_per_rev      = MOTOR_STEPS_PER_REV,
    },
    // Thermistor Pins
    .io_temp = {
        .pin_therm_amb      = A0,
        .pin_therm_motor_x  = A1,
        .pin_therm_motor_y  = A3,
        .pin_therm_mpmt     = A2,
        .pin_therm_optical  = A4,
    }
};

#endif // CONF_H