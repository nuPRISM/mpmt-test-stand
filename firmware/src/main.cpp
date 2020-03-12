#include "Arduino.h"
#include "Axis.h"
#include "Kinematics.h"
#include "Debug.h"

/*
TODO:
    - Add conversion from distance to counts based on given holding velocity, acceleration
    - Test interrupt priority
    - Add error handling and error raising for impossible encoder counts - negative values etc
      might not be necessary since we have limit switched - needs more thought
    - Add mode where only motor counts are used, in case we dont need encoders
    - Check division by zero
*/

void setup()
{   
    // setup_encoder_interrupts();
    // setup_ls_interrupts();

    debug();
    print("setup ", 0);
    delay(2000);
    // // for testing only
    setup_axis(&axis_x_config, &axis_x);
    delay(2000);
    axis_trapezoidal_move_rel(&axis_x, 100, 100, 100, POSITIVE);
}

int count = 0;

void loop()
{   
    delay(1000);
    // digitalWrite(2, HIGH);
    print("loop ", count);
    delay(1000);
    // digitalWrite(2, LOW);
    count++;
}


