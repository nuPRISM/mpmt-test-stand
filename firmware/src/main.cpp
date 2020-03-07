#include "Arduino.h"
#include "Axis.h"
#include "Kinematics.h"
#include "Debug.h"

/*
TODO:
    - Add conversion from distance to counts based on given holding velocity, acceleration
    - Address the PR comments
    - Test interrupt priority
    - Simplify the code where possible
    - Add ansolute movement routines (might be done at PC level)
    - Quadrature encoder ISR MIGHT need to be rewritten to use encoder direction rather than relying on dir command value
        -- check direction values (positive HIGH or LOW)
    - Add error handling and error raising for impossible encoder counts - negative values etc
      might not be necessary since we have limit switched - needs more thought
    - Add mode where only motor counts are used, in case we dont need encoders
*/

void setup()
{   
    // setup_encoder_interrupts();
    // setup_ls_interrupts();

    debug();
    print("setup ", 0);
    // // for testing only
    setup_axis(&axis_x_config, &axis_x);
    setup_axis(&axis_y_config, &axis_y);
    // axis_trapezoidal_move_rel(&axis_x, 800, 1000, 1000, 1000, POSITIVE);
}

int count = 0;

void loop()
{   
    axis_trapezoidal_move_rel(&axis_x, 800, 1000, 1000, 1000, POSITIVE);
    delay(1000);
    // digitalWrite(2, HIGH);
    print("loop ", count);
    delay(1000);
    // digitalWrite(2, LOW);
    count++;
}


