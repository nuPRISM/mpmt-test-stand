#include "Arduino.h"

#include "Debug.h"

#include "ArduinoSerialDevice.h"
#include "TestStandCommController.h"
#include "Messages.h"
#include "Axis.h"
#include "Movement.h"
#include "Kinematics.h"

#define BAUD_RATE 115200

ArduinoSerialDevice serial_device(Serial);
TestStandCommController comm(serial_device);

void setup()
{
    // put your setup code here, to run once:
    pinMode(LED_BUILTIN, OUTPUT);

    DEBUG_INIT;
    serial_device.ser_connect(BAUD_RATE);

    setup_axis(&axis_x_config, &axis_x);
    setup_axis(&axis_y_config, &axis_y);

    delay(1000);
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

    Axis *axis_ptr = (axis == AXIS_X ? &axis_x : &axis_y);
    VelProfile profile;
    generate_vel_profile(accel, axis_x.vel_min, hold_vel, dist, &profile);

    axis_trapezoidal_move_rel(axis_ptr, profile.counts_accel, profile.counts_hold, profile.counts_decel, (Direction)dir);
    
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
