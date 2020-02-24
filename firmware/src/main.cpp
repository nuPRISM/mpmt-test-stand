#include "Arduino.h"
#include "Axis.h"

/*
TODO:
    - Add ansolute movement routines (might be done at PC level)
    - Quadrature encoder ISR MIGHT need to be rewritten to use encoder direction rather than relying on dir command value
        -- check direction values (positive HIGH or LOW)
    - Add error handling and error raising for impossible encoder counts - negative values etc
      might not be necessary since we have limit switched - needs more thought
    - Add mode where only motor counts are used, in case we dont need encoders
*/

void setup()
{   
    setup_encoder_interrupts();
    setup_ls_interrupts();

    debug();
    // for testing only
    axis_trapezoidal_move_rel(&axis_x, 0, 5, 5, 5, negative);
    // axis_trapezoidal_move_rel(&axis_y, 5, 5, 5, positive);
    // constant_max_vel(&axis_x, 5);
}


void loop()
{

}


