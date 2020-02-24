#include "Arduino.h"
#include "Debug.h"
#include "Kinematics.h"
#include "Axis.h"

/*
TODO:
    - Add limit switch interrupt handling routines
    - Add homing routine
    - Add ansolute movement routines (might be done at PC level)
    - Quadrature encoder ISR needs to be rewritten and direction needs to be incorporated
        -- check direction values (positive HIGH or LOW)
    - Add error handling and error raising for impossible encoder counts - negative values etc
      might not be necessary since we have limit switched - needs more thought
    - Add mode where only motor counts are used, in case we dont need encoders
*/

void setup_encoder_interrupts()
{
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A_X), isr_encoder_x, RISING);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A_Y), isr_encoder_y, RISING);
    attachInterrupt(digitalPinToInterrupt(LIMIT_SW_HOME_PIN_X), isr_limit_switch_x, CHANGE);
    attachInterrupt(digitalPinToInterrupt(LIMIT_SW_HOME_PIN_Y), isr_limit_switch_y, CHANGE);
    attachInterrupt(digitalPinToInterrupt(LIMIT_SW_FAR_PIN_X), isr_limit_switch_x, CHANGE);
    attachInterrupt(digitalPinToInterrupt(LIMIT_SW_FAR_PIN_Y), isr_limit_switch_y, CHANGE);
}

void setup()
{   
    debug();
    // for testing only
    setup_encoder_interrupts();
    axis_trapezoidal_move_rel(&axis_x, 0, 5, 5, 5, negative);
    // axis_trapezoidal_move_rel(&axis_y, 5, 5, 5, positive);
    // constant_max_vel(&axis_x, 5);
}


void loop()
{

}


