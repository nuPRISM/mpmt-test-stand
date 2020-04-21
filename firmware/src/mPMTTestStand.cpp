/* **************************** Local Includes ***************************** */
#include "mPMTTestStand.h"
// Serial Communication
#include "Messages.h"
#include "TestStandMessages.h"
// Gantry
#include "Gantry.h"
// Temperature DAQ
#include "ThermistorArray.h"
// Other
#include "Debug.h"

mPMTTestStand::mPMTTestStand(const mPMTTestStandConfig &conf, Calibration cal) :
    conf(conf),
    cal(cal),
    comm_dev(conf.serial_comm),
    comm(this->comm_dev),
    thermistors(conf.io_temp, this->cal.cal_temp)
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

    // Setup gantry axis control
    axis_setup(AXIS_X, &(this->conf.io_axis_x), &(this->conf.axis_mech));
    axis_setup(AXIS_Y, &(this->conf.io_axis_y), &(this->conf.axis_mech));

    // Thermistor pin configuration
    this->thermistors.setup();

    // Connect serial communications
    this->comm_dev.ser_connect(this->conf.serial_comm_baud_rate);

    // Wait until we can successfully ping the host
    while (this->comm.ping() != SERIAL_OK) {
        delay(100);
        DEBUG_PRINTLN("Waiting for host...");
    }
    DEBUG_PRINTLN("Host connected!");

    this->status = STATUS_IDLE;
}

void mPMTTestStand::handle_echo()
{
    this->comm.recv_echo();
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
        .accel        = this->cal.cal_gantry.accel,
        .vel_start    = this->cal.cal_gantry.vel_start,
        .vel_hold     = this->cal.cal_gantry.vel_home
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
        .accel        = this->cal.cal_gantry.accel,
        .vel_start    = this->cal.cal_gantry.vel_start,
        .vel_hold     = this->cal.cal_gantry.vel_home
    };
    axis_start(AXIS_X, &motion);
    axis_start(AXIS_Y, &motion);
    this->home_a_done = true;
    this->status = STATUS_HOMING;
}

void mPMTTestStand::handle_move()
{
    MoveMsgData data;
    AxisResult res;
    if (this->comm.recv_move(&data)) {
        AxisMotionSpec motion = {
            .dir          = (AxisDirection)data.dir,
            .total_counts = data.dist_counts,
            .accel        = this->cal.cal_gantry.accel,
            .vel_start    = this->cal.cal_gantry.vel_start,
            .vel_hold     = data.vel_hold
        };

        res = axis_start((AxisId)data.axis, &motion);

        if (res == AXIS_OK) {
            this->status = STATUS_MOVING;
        }
    }
    else {
        res = AXIS_ERR_INVALID;
    }
    this->comm.axis_result(res);
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
    TempData temp_data;
    this->thermistors.read_temp_data(temp_data);
    this->comm.temp(&temp_data);
}

void mPMTTestStand::handle_calibrate()
{
    this->comm.recv_calibrate(&this->cal);
}

#ifdef DEBUG
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

void mPMTTestStand::debug_dump_state()
{
    DEBUG_PRINT_VAL("STATUS", this->status);
    this->debug_dump_axis(AXIS_X);
    this->debug_dump_axis(AXIS_Y);
}

void mPMTTestStand::debug_dump_calibration()
{
    DEBUG_PRINTLN("----------------------------------------");
    DEBUG_PRINTLN("CALIBRATION:");
    DEBUG_PRINT_VAL("accel    ", this->cal.cal_gantry.accel);
    DEBUG_PRINT_VAL("vel_start", this->cal.cal_gantry.vel_start);
    DEBUG_PRINT_VAL("vel_home ", this->cal.cal_gantry.vel_home);
    DEBUG_PRINTLN("");
    DEBUG_PRINT_VAL("c1       ", this->cal.cal_temp.all.c1);
    DEBUG_PRINT_VAL("c2       ", this->cal.cal_temp.all.c2);
    DEBUG_PRINT_VAL("c3       ", this->cal.cal_temp.all.c3);
    DEBUG_PRINT_VAL("resistor ", this->cal.cal_temp.all.resistor);
    DEBUG_PRINTLN("----------------------------------------");
}
#endif // DEBUG

void mPMTTestStand::execute()
{
    DEBUG_PERIODIC(
        this->debug_dump_state();
        this->debug_dump_calibration(),
        1000);

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
        switch (id) {
            case MSG_ID_ECHO:           this->handle_echo();           break;
            case MSG_ID_HOME:           this->handle_home_a();         break;
            case MSG_ID_MOVE:           this->handle_move();           break;
            case MSG_ID_STOP:           this->handle_stop();           break;
            case MSG_ID_GET_STATUS:     this->handle_get_status();     break;
            case MSG_ID_GET_POSITION:   this->handle_get_position();   break;
            case MSG_ID_GET_AXIS_STATE: this->handle_get_axis_state(); break;
            case MSG_ID_GET_TEMP:       this->handle_get_temp();       break;
            case MSG_ID_CALIBRATE:      this->handle_calibrate();      break;
            default:                                                   break;
        }
    }
}
