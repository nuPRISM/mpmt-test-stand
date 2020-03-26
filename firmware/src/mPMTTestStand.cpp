#include "mPMTTestStand.h"

#include "Axis.h"
#include "Movement.h"

#include "Debug.h"
#include "macros.h"

mPMTTestStand::mPMTTestStand(const mPMTTestStandIO& io) :
    io(io),
    comm_dev(io.serial_comm),
    comm(this->comm_dev),
    thermistor_ambient(io.pin_therm_amb),
    thermistor_motor1(io.pin_therm_motor1),
    thermistor_mpmt(io.pin_therm_mpmt),
    thermistor_motor2(io.pin_therm_motor2),
    thermistor_optical(io.pin_therm_optical)
{
    // Nothing else to do
};

void mPMTTestStand::setup()
{
    DEBUG_PRINTLN("mPMT Test Stand");

    // Thermistor pin configuration
    analogReadResolution(12); // enable 12 bit resolution mode in Arduino Due. Default is 10 bit.
    pinMode(this->io.pin_therm_amb,     INPUT);
    pinMode(this->io.pin_therm_motor1,  INPUT);
    pinMode(this->io.pin_therm_mpmt,    INPUT);
    pinMode(this->io.pin_therm_motor2,  INPUT);
    pinMode(this->io.pin_therm_optical, INPUT);

    // Connect serial communications
    this->comm_dev.ser_connect(this->io.serial_comm_baud_rate);

    // Setup gantry axis control
    setup_axis(&axis_x_config, &axis_x);
    setup_axis(&axis_y_config, &axis_y);

    // Wait until we can successfully ping the host
    while (!(this->comm.ping())) {
        delay(100);
        DEBUG_PRINTLN("Waiting for host...");
    }
    DEBUG_PRINTLN("Host connected!");
};

void mPMTTestStand::handle_home()
{
    
};

void mPMTTestStand::handle_move()
{
    // Process command arguments
    uint32_t accel, hold_vel, dist;
    uint8_t axis, dir;

    uint8_t *data = this->comm.received_message().data;

    accel    = NTOHL(data);
    hold_vel = NTOHL(data + 4);
    dist     = NTOHL(data + 8);
    axis     = data[12];
    dir      = data[13];

    // Generate a velocity profile for the correct axis
    Axis *axis_ptr = (axis == AXIS_X ? &axis_x : &axis_y);
    VelProfile profile;
    if (generate_vel_profile(accel, axis_ptr->vel_min, hold_vel, dist, &profile)) {
        // Start the movement if the velocity profile is valid
        axis_trapezoidal_move_rel(axis_ptr, profile.counts_accel, profile.counts_hold, profile.counts_decel, (Direction)dir);
    }
    
    // TODO delete this log message:
    this->comm.log(LL_INFO, "cts_a = %d, cts_h = %d, dist = %d, axis = %c, dir = %s",
        profile.counts_accel,
        profile.counts_hold,
        dist,
        (axis == AXIS_X ? 'x' : 'y'),
        (dir == DIR_POSITIVE ? "pos" : "neg"));
};

void mPMTTestStand::execute()
{
    // Check for any messages
    if (this->comm.check_for_message()) {
        switch (this->comm.received_message().id) {
            case MSG_ID_HOME:
                handle_home();
                break;
            case MSG_ID_MOVE:
                handle_move();
                break;
            default:
                break;
        }
    }
};