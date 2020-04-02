#ifndef AXIS_H
#define AXIS_H

/* ************************ Shared Project Includes ************************ */
#include "shared_defs.h"

/* **************************** System Includes **************************** */
#include <Arduino.h>

/*****************************************************************************/
/*                                 TYPEDEFS                                  */
/*****************************************************************************/

/**
 * @struct AxisPins
 * 
 * @brief Specifies all of the I/O pins used by an axis
 */
typedef struct {
    Tc *tc_step;                       //!< TC for step output (e.g. TC2)
    uint32_t tc_step_channel;          //!< TC channel number for step output (0 to 3)
    IRQn_Type tc_step_irq;             //!< TC IRQ number for step output (e.g. TC6)
    Pio *pio_step;                     //!< PIO bank for step output (e.g. PIOA, PIOB, PIOC etc.)
    EPioType pio_step_periph;          //!< PIO peripheral for step output (e.g. PIO_PERIPH_B)
    uint32_t pio_step_pin_mask;        //!< PIO pin mask for step output (e.g. PIO_PC25B_TIOA6)
    uint8_t pin_dir;                   //!< Arduino output pin connected to the DIR input of the motor driver
    uint8_t pin_enc_a;                 //!< Arduino input pin connected to the encoder A output
    uint8_t pin_enc_b;                 //!< Arduino input pin connected to the encoder B input
    uint8_t pin_ls_home;               //!< Arduino input pin connected to the home limit switch
    uint8_t pin_ls_far;                //!< Arduino input pin connected to the far limit switch
} AxisIO;

/**
 * @struct AxisMech
 * 
 * @brief Specifies mechanical parameters of the axis
 */
typedef struct {
    uint32_t counts_per_rev;           //!< The number of encoder counts in one revolution of the motor
    uint32_t steps_per_rev;            //!< The number of motor steps in one revolution of the motor
} AxisMech;

/**
 * @struct AxisMotionSpec
 * 
 * @brief Fully specifies a motion for an axis to execute
 */
typedef struct {
    Direction dir;                     //!< Movement direction
    uint32_t total_counts;             //!< The total distance [encoder counts]
    uint32_t accel;                    //!< Acceleration       [motor steps / s^2]
    uint32_t vel_start;                //!< Starting velocity  [motor steps / s]
    uint32_t vel_hold;                 //!< Holding velocity   [motor steps / s]
} AxisMotionSpec;

/**
 * @enum VelSeg
 * 
 * @brief Enumeration of the segments of a trapezoidal velocity profile
 */
typedef enum {
    VEL_SEG_ACCELERATE,                //!< Accelerating up to the holding velocity
    VEL_SEG_HOLD,                      //!< Staying constant at the holding velocity
    VEL_SEG_DECELERATE                 //!< Decelerating down to a minimum velocity
} VelSeg;

/**
 * @struct AxisState
 * 
 * @brief Specifies the current state of an axis
 * 
 * All fields are declared volatile since they may be modified from ISRs
 */
typedef struct {
    volatile bool moving;              //!< true if the axis is currently moving

    volatile bool ls_home_pressed;     //!< true if the home limit switch is currently pressed
    volatile bool ls_far_pressed;      //!< true if the far limit switch is currently pressed

    volatile uint32_t velocity;        //!< Current velocity of the axis
    volatile uint32_t next_velocity;   //!< Pending velocity to set for the axis
    volatile VelSeg velocity_segment;  //!< Current velcoity segment of the axis
    volatile int32_t encoder_current;  //!< Current position of the axis in encoder counts
    volatile int32_t encoder_target;   //!< Position at which the next segment transition will occur
    volatile Direction dir;            //!< Current direction of motion of the axis
} AxisState;

/**
 * @enum AxisResult
 * 
 * @brief Possible return values for a call to axis_start
 */
typedef enum {
    AXIS_OK,                           //!< Axis movement started OK
    AXIS_ERR_ALREADY_MOVING,           //!< Axis is already moving
    AXIS_ERR_LS_HOME,                  //!< Trying to move backward while HOME limit switch is pressed
    AXIS_ERR_LS_FAR,                   //!< Trying to move forward while FAR limit switch is pressed
    AXIS_ERR_INVALID                   //!< The parameters resulted in an invalid motion profile
} AxisResult;

/*****************************************************************************/
/*                             PUBLIC FUNCTIONS                              */
/*****************************************************************************/

void axis_setup(AxisId axis_id, const AxisIO *io, const AxisMech *mech);
AxisResult axis_start(AxisId axis_id, AxisMotionSpec *motion);
void axis_stop(AxisId axis_id);
void axis_reset(AxisId axis_id);
const AxisState *axis_get_state(AxisId axis_id);

#endif // AXIS_H
