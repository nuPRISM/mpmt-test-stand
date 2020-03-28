#ifndef AXIS_H
#define AXIS_H

#include <Arduino.h>
#include <stdint.h>

#include "shared_defs.h"

#define VELOCITY_HOMING 10000

/**
 * @struct AxisPins
 * 
 * @brief Specifies all of the I/O pins used by an axis
 */
typedef struct {
    // uint8_t pin_step;           //!< Arduino output pin connected to the STEP input of the motor driver
    Tc *tc_step;                //<! TC for step output (e.g. TC2)
    uint32_t tc_step_channel;   //<! TC channel number for step output (0 to 3)
    IRQn_Type tc_step_irq;      //<! TC IRQ number for step output (e.g. TC6)
    Pio *pio_step_bank;         //!< PIO bank for step output (e.g. PIOA, PIOB, PIOC etc.)
    EPioType pio_step_periph;   //!< PIO peripheral for step output (e.g. PIO_PERIPH_B)
    uint32_t pio_step_pin_mask; //!< PIO pin mask for step output (e.g. PIO_PC25B_TIOA6)
    uint8_t pin_dir;            //!< Arduino output pin connected to the DIR input of the motor driver
    uint8_t pin_enc_a;          //!< Arduino input pin connected to the encoder A output
    uint8_t pin_enc_b;          //!< Arduino input pin connected to the encoder B input
    uint8_t pin_ls_home;        //!< Arduino input pin connected to the home limit switch
    uint8_t pin_ls_far;         //!< Arduino input pin connected to the far limit switch
} AxisIO;

/**
 * @struct AxisMotion
 * 
 * @brief Fully specifies a motion for an axis to execute
 */
typedef struct {
    Direction dir;           //!< movement direction
    uint32_t vel_start;      //!< starting velocity [motor steps / s]
    uint32_t accel;          //!< acceleration [motor steps / s^2]
    int32_t counts_accel;    //!< number of encoder counts while accelerating
    int32_t counts_hold;     //!< number of encoder counts while moving at constant velocity
    int32_t counts_decel;    //!< number of encoder counts while decelerating
} AxisMotion;

typedef enum {
    AXIS_OK,
    AXIS_ERR_DIR_MISMATCH,
    AXIS_ERR_ZERO_DIST,
    AXIS_ERR_ALREADY_MOVING,
    AXIS_ERR_LS_HOME,
    AXIS_ERR_LS_FAR,
    AXIS_ERR_TOO_FAR_FORWARD,
    AXIS_ERR_TOO_FAR_BACKWARD
} AxisResult;

void axis_setup(AxisId axis_id, const AxisIO *io);
AxisResult axis_start(AxisId axis_id, AxisMotion *motion);
void axis_stop(AxisId axis_id);

uint32_t axis_get_position(AxisId axis_id);
bool axis_moving(AxisId axis_id);

void axis_dump_state(AxisId axis_id);

#endif // AXIS_H