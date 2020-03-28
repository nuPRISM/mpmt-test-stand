/* **************************** Local Includes ***************************** */
#include "mPMTTestStand.h"
#include "Kinematics.h"
#include "Movement.h"
#include "Debug.h"

/* ************************ Shared Project Includes ************************ */
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
}

void mPMTTestStand::setup()
{
    DEBUG_PRINTLN("\n\n==================================================");
    DEBUG_PRINTLN("mPMT Test Stand");
    DEBUG_PRINTLN("==================================================\n");

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
    axis_setup(AXIS_X, &(this->io.io_axis_x));
    axis_setup(AXIS_Y, &(this->io.io_axis_y));

    // Wait until we can successfully ping the host
    while (!(this->comm.ping())) {
        delay(100);
        DEBUG_PRINTLN("Waiting for host...");
    }
    DEBUG_PRINTLN("Host connected!");

    this->status = STATUS_IDLE;
}

void mPMTTestStand::handle_home()
{
    this->status = STATUS_HOMING;

    // These are both non-blocking
    // TODO
    // home_axis(&axis_x);
    // home_axis(&axis_y);

    DEBUG_PRINTLN("HOMING");
}

void mPMTTestStand::handle_move()
{
    // Process command arguments
    uint32_t accel, hold_vel, dist;
    AxisId axis;
    Direction dir;

    uint8_t *data = this->comm.received_message().data;

    accel    = NTOHL(data);
    hold_vel = NTOHL(data + 4);
    dist     = NTOHL(data + 8);
    axis     = (AxisId)data[12];
    dir      = (Direction)data[13];

    // Generate a velocity profile for the correct axis
    VelProfile profile;
    if (generate_vel_profile(accel, VEL_START, hold_vel, dist, &profile)) {
        // Start the movement if the velocity profile is valid
        axis_trapezoidal_move_rel(axis, dir, accel, profile.counts_accel, profile.counts_hold, profile.counts_decel);
    }

    DEBUG_PRINTLN("MOVING");
    DEBUG_PRINT_VAL("    accel", accel);
    DEBUG_PRINT_VAL("    hold_vel", hold_vel);
    DEBUG_PRINT_VAL("    dist", dist);
    DEBUG_PRINT_VAL("    axis", axis);
    DEBUG_PRINT_VAL("    dir", dir);
    DEBUG_PRINT_VAL("    cts_a", profile.counts_accel);
    DEBUG_PRINT_VAL("    cts_h", profile.counts_hold);
}

void mPMTTestStand::handle_stop()
{
    axis_stop(AXIS_X);
    axis_stop(AXIS_Y);

    DEBUG_PRINTLN("STOPPING");
}

void mPMTTestStand::handle_get_status()
{
    this->comm.status(this->status);
}

void mPMTTestStand::handle_get_data()
{
    DataId data_id = (DataId)((this->comm.received_message().data)[0]);
    switch (data_id) {
        case DATA_MOTOR:
            uint8_t data[2*4];
            HTONL(data, axis_get_position(AXIS_X));
            HTONL(data + 4, axis_get_position(AXIS_Y));
            this->comm.data(data, sizeof(data));
            break;

        case DATA_TEMP:
            // TODO
            break;
    }
}

void mPMTTestStand::execute()
{
    PERIODIC(axis_dump_state(AXIS_X), 1000);

    // Update status
    Status old_status = this->status;
    if (axis_moving(AXIS_X) || axis_moving(AXIS_Y)) {
        // TODO handle homing
        this->status = STATUS_MOVING;
        // this->status = (axis_x.homing || axis_y.homing) ? STATUS_HOMING : STATUS_MOVING;
    }
    // TODO
    // else if (axis_x.fault || axis_y.fault) {
    //     this->status = STATUS_FAULT;
    // }
    else {
        this->status = STATUS_IDLE;
    }

    if (this->status != old_status) {
        DEBUG_PRINT_VAL("Status updated to", this->status);
    }

    // Check for any messages
    if (this->comm.check_for_message()) {
        uint8_t id = this->comm.received_message().id;
        DEBUG_PRINT_VAL("Received Message w/ ID", id);
        switch (id) {
            case MSG_ID_HOME:
                this->handle_home();
                break;
            case MSG_ID_MOVE:
                this->handle_move();
                break;
            case MSG_ID_STOP:
                this->handle_stop();
                break;
            case MSG_ID_GET_STATUS:
                this->handle_get_status();
                break;
            case MSG_ID_GET_DATA:
                this->handle_get_data();
                break;
            default:
                break;
        }
    }
}