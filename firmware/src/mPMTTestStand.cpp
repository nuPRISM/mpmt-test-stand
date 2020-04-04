/* **************************** Local Includes ***************************** */
#include "mPMTTestStand.h"
#include "Debug.h"

/* ************************ Shared Project Includes ************************ */
#include "macros.h"

mPMTTestStand::mPMTTestStand(const mPMTTestStandConfig& conf) :
    conf(conf),
    comm_dev(conf.io.serial_comm),
    comm(this->comm_dev),
    thermistor_ambient(conf.io.pin_therm_amb),
    thermistor_motor1(conf.io.pin_therm_motor_x),
    thermistor_mpmt(conf.io.pin_therm_mpmt),
    thermistor_motor2(conf.io.pin_therm_motor_y),
    thermistor_optical(conf.io.pin_therm_optical)
{
    this->x_state = axis_get_state(AXIS_X);
    this->y_state = axis_get_state(AXIS_Y);
    
    this->status = STATUS_IDLE;
}

void mPMTTestStand::setup()
{
    DEBUG_PRINTLN("\n\n==================================================");
    DEBUG_PRINTLN("mPMT Test Stand");
    DEBUG_PRINTLN("==================================================\n");

    // Thermistor pin configuration
    analogReadResolution(12); // enable 12 bit resolution mode in Arduino Due. Default is 10 bit.
    pinMode(this->conf.io.pin_therm_amb,     INPUT);
    pinMode(this->conf.io.pin_therm_motor_x, INPUT);
    pinMode(this->conf.io.pin_therm_mpmt,    INPUT);
    pinMode(this->conf.io.pin_therm_motor_y, INPUT);
    pinMode(this->conf.io.pin_therm_optical, INPUT);

    // Connect serial communications
    this->comm_dev.ser_connect(this->conf.io.serial_comm_baud_rate);

    // Setup gantry axis control
    axis_setup(AXIS_X, &(this->conf.io.io_axis_x), &(this->conf.gantry.axis_mech));
    axis_setup(AXIS_Y, &(this->conf.io.io_axis_y), &(this->conf.gantry.axis_mech));

    // Wait until we can successfully ping the host
    while (this->comm.ping() != SERIAL_OK) {
        delay(100);
        DEBUG_PRINTLN("Waiting for host...");
    }
    DEBUG_PRINTLN("Host connected!");

    this->status = STATUS_IDLE;
}

/**
 * @brief Handles the first part of the homing routine (part A)
 * 
 * Drives both axes in the negative direction until the home limit
 * switch is pressed.
 */
void mPMTTestStand::handle_home_a()
{
    AxisMotionSpec motion = {
        .dir          = AXIS_DIR_NEGATIVE,
        .total_counts = INT32_MAX,
        .accel        = this->conf.gantry.accel_home_a,
        .vel_start    = this->conf.gantry.vel_start,
        .vel_hold     = this->conf.gantry.vel_home_a
    };

    axis_start(AXIS_X, &motion);
    axis_start(AXIS_Y, &motion);
    this->home_a_done = false;
    this->status = STATUS_HOMING;
}

/**
 * @brief Handles the second part of the homing routine (part B)
 * 
 * Drives both axes in the positive direction until the home limit
 * switch is released.
 */
void mPMTTestStand::handle_home_b()
{
    AxisMotionSpec motion = {
        .dir          = AXIS_DIR_POSITIVE,
        .total_counts = INT32_MAX,
        .accel        = this->conf.gantry.accel_home_b,
        .vel_start    = this->conf.gantry.vel_start,
        .vel_hold     = this->conf.gantry.vel_home_b
    };
    axis_start(AXIS_X, &motion);
    axis_start(AXIS_Y, &motion);
    this->home_a_done = true;
    this->status = STATUS_HOMING;
}

void mPMTTestStand::handle_move()
{
    // Process command arguments
    uint32_t accel, vel_hold, dist;
    AxisId axis_id;
    AxisDirection dir;

    uint8_t *data = this->comm.received_message().data;

    accel    = NTOHL(data);
    vel_hold = NTOHL(data + 4);
    dist     = NTOHL(data + 8);
    axis_id  = (AxisId)data[12];
    dir      = (AxisDirection)data[13];

    AxisMotionSpec motion = {
        .dir          = dir,
        .total_counts = dist,
        .accel        = accel,
        .vel_start    = this->conf.gantry.vel_start,
        .vel_hold     = vel_hold
    };

    if (axis_start(axis_id, &motion) == AXIS_OK) {
        this->status = STATUS_MOVING;
    }
}

void mPMTTestStand::handle_stop()
{
    axis_stop(AXIS_X);
    axis_stop(AXIS_Y);
    this->status = STATUS_IDLE;
}

void mPMTTestStand::handle_get_status()
{
    this->comm.status(this->status);
}

void mPMTTestStand::handle_get_position()
{
    this->comm.position(this->x_state->encoder_current, this->y_state->encoder_current);
}

void mPMTTestStand::handle_get_axis_state()
{
    // TODO
}

void mPMTTestStand::handle_get_temp()
{
    // TODO
}

void mPMTTestStand::debug_dump_axis(AxisId axis_id)
{
    const AxisState *state = (axis_id == AXIS_X ? this->x_state : this->y_state);
    DEBUG_PRINTLN("----------------------------------------");
    DEBUG_PRINT_VAL("AXIS", axis_id == AXIS_X ? "X" : "Y");
    DEBUG_PRINT_VAL("moving          ", state->moving);
    DEBUG_PRINT_VAL("ls_home_pressed ", state->ls_home_pressed);
    DEBUG_PRINT_VAL("ls_far_pressed  ", state->ls_far_pressed);
    DEBUG_PRINT_VAL("velocity        ", state->velocity);
    DEBUG_PRINT_VAL("next_velocity   ", state->next_velocity);
    DEBUG_PRINT_VAL("velocity_segment", state->velocity_segment);
    DEBUG_PRINT_VAL("encoder_current ", state->encoder_current);
    DEBUG_PRINT_VAL("encoder_target  ", state->encoder_target);
    DEBUG_PRINT_VAL("dir             ", state->dir);
    DEBUG_PRINTLN("----------------------------------------");
}

void mPMTTestStand::debug_dump()
{
    DEBUG_PRINT_VAL("STATUS", this->status);
    this->debug_dump_axis(AXIS_X);
    this->debug_dump_axis(AXIS_Y);
}

void mPMTTestStand::execute()
{
    PERIODIC(this->debug_dump(), 1000);

    // Update status
    switch (this->status) {
        case STATUS_IDLE:
            break;
        case STATUS_MOVING:
            if (this->x_state->moving || this->y_state->moving) {
                this->status = STATUS_MOVING;
            }
            else if (this->x_state->ls_home_pressed || this->x_state->ls_far_pressed ||
                     this->y_state->ls_home_pressed || this->x_state->ls_far_pressed) {
                this->status = STATUS_FAULT;
            }
            else {
                this->status = STATUS_IDLE;
            }
            break;
        case STATUS_HOMING:
            if (!this->home_a_done) {
                // HOME A
                if (this->x_state->moving || this->y_state->moving) {
                    this->status = STATUS_HOMING;
                }
                else if (this->x_state->ls_home_pressed && this->y_state->ls_home_pressed) {
                    handle_home_b();
                }
                else {
                    // If both axes stopped moving but one of the home limit switches isn't pressed
                    // something has gone wrong so enter FAULT state
                    this->status = STATUS_FAULT;
                }
            }
            else {
                // HOME B
                if (this->x_state->moving || this->y_state->moving) {
                    this->status = STATUS_HOMING;
                }
                else {
                    axis_reset(AXIS_X);
                    axis_reset(AXIS_Y);
                    this->status = STATUS_IDLE;
                    this->home_a_done = false;
                }
            }
            break;
        case STATUS_FAULT:
            // Do nothing
            break;
    }

    // Check for any messages
    if (this->comm.check_for_message() == SERIAL_OK) {
        uint8_t id = this->comm.received_message().id;
        DEBUG_PRINT_VAL("Received Message w/ ID", id);
        switch (id) {
            case MSG_ID_HOME:
                this->handle_home_a();
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
            case MSG_ID_GET_POSITION:
                this->handle_get_position();
                break;
            case MSG_ID_GET_AXIS_STATE:
                this->handle_get_axis_state();
                break;
            case MSG_ID_GET_TEMP:
                this->handle_get_temp();
                break;
            default:
                break;
        }
    }
}
