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

#include "ArduinoSerialDevice.h"
#include "TestStandCommController.h"
#include "Messages.h"

#define BAUD_RATE 115200

ArduinoSerialDevice serial_device(Serial);
TestStandCommController comm(serial_device);

void setup()
{
    // put your setup code here, to run once:
    pinMode(LED_BUILTIN, OUTPUT);

    serial_device.ser_connect(BAUD_RATE);
}

void handle_home()
{

}

void handle_move()
{
    uint16_t accel, hold_vel, dist;
    uint8_t axis, dir;

    uint8_t *data = comm.received_message().data;

    accel    = ((uint16_t)data[0] << 8) | data[1];
    hold_vel = ((uint16_t)data[2] << 8) | data[3];
    dist     = ((uint16_t)data[4] << 8) | data[5];

    axis = data[6];
    dir = data[7];

    // TODO call motor code
    // TODO delete this log message:
    comm.log(LL_INFO, "accel = %d, hold = %d, dist = %d, axis = %c, dir = %s",
        accel,
        hold_vel,
        dist,
        (axis == AXIS_X ? 'x' : 'y'),
        (dir == DIR_POSITIVE ? "pos" : "neg"));
}

int count = 0;

void loop()
{
    if (comm.check_for_message()) {
        switch (comm.received_message().id) {
            case MSG_ID_HOME:
                break;
            case MSG_ID_MOVE:
                handle_move();
                break;
        }
    }
}
