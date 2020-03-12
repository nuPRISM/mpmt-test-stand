#include "Arduino.h"
#include "Axis.h"
#include "Movement.h"
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
    DEBUG_INIT;
    DEBUG_PRINT("setup ", 0);
    // // for testing only
    setup_axis(&axis_x_config, &axis_x);

    // Example
    delay(2000);
    axis_trapezoidal_move_rel(&axis_x, 10000, 10000, 10000, POSITIVE);
}

int count = 0;

void loop()
{   
    delay(1000);
    // digitalWrite(2, HIGH);
    DEBUG_PRINT("loop ", count);
    delay(1000);
    // digitalWrite(2, LOW);
    count++;
}


