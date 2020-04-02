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
    this->x_state = axis_get_state(AXIS_X);
    this->y_state = axis_get_state(AXIS_Y);
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
    move_axis_home_a(AXIS_X);
    move_axis_home_a(AXIS_Y);
    this->home_a_done = false;
    this->status = STATUS_HOMING;
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

    if (move_axis_rel(axis, dir, accel, hold_vel, dist)) {
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

void mPMTTestStand::handle_get_data()
{
    DataId data_id = (DataId)((this->comm.received_message().data)[0]);
    switch (data_id) {
        case DATA_MOTOR:
        {
            uint32_t axis_x_pos = axis_get_position(AXIS_X);
            uint32_t axis_y_pos = axis_get_position(AXIS_Y);

            uint8_t data[2*4];
            HTONL(data, axis_x_pos);
            HTONL(data + 4, axis_y_pos);
            this->comm.data(data, sizeof(data));
            break;
        }
        case DATA_TEMP:
        {
            // TODO
            break;
        }
    }
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
                    this->status = STATUS_HOMING;
                    move_axis_home_b(AXIS_X);
                    move_axis_home_b(AXIS_Y);
                    this->home_a_done = true;
                }
                else {
                    // If both axis stopped moving but one of the home limit switches isn't pressed
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
