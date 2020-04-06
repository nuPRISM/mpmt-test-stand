#include "ArduinoHelper.h"
#include "TestStandMessages.h"
#include "Gantry.h"

#include "LinuxSerialDevice.h"
#include "TestStandCommHost.h"

#include "shared_defs.h"

#include "midas.h"

#include <stdio.h>
#include <math.h>

#define BAUD_RATE           115200
#define MSG_RECEIVE_TIMEOUT 5000

LinuxSerialDevice device;
TestStandCommHost comm(device);

// Gantry Mechanical Properties
const float pulley_diameter_mm   = 17;
const float dist_per_rev_mm      = 2 * M_PI * (pulley_diameter_mm / 2);
const float mm_cts_ratio         = dist_per_rev_mm / ENCODER_COUNTS_PER_REV;
const float mm_steps_ratio       = dist_per_rev_mm / MOTOR_STEPS_PER_REV;

// Operational Bounds
const float gantry_x_min_mm      = 0.0;
const float gantry_x_max_mm      = 1200.0; // max rail is 1219 mm
const float gantry_y_min_mm      = 0.0;
const float gantry_y_max_mm      = 1200.0;
const float gantry_vel_min_mm_s  = 0.0; 
const float gantry_vel_max_mm_s  = 10.0;

/*****************************************************************************/
/*                             PRIVATE FUNCTIONS                             */
/*****************************************************************************/

static bool validate_move_params(float *dest_mm, float *vel_mm_s)
{
    if (dest_mm[AXIS_X] < gantry_x_min_mm || dest_mm[AXIS_X] > gantry_x_max_mm) {
        cm_msg(
            MERROR,
            "validate_move_params",
            "Destination on x-axis should be between %f mm and %f mm inclusive.\n",
            gantry_x_min_mm, gantry_x_max_mm);
        return false;
    }

    if (dest_mm[AXIS_Y] < gantry_y_min_mm || dest_mm[AXIS_Y] > gantry_y_max_mm) {
        cm_msg(
            MERROR,
            "validate_move_params",
            "Destination on y-axis should be between %f mm and %f mm inclusive.\n",
            gantry_y_min_mm, gantry_y_max_mm);
        return false;
    }

    if (vel_mm_s[AXIS_X] < gantry_vel_min_mm_s || vel_mm_s[AXIS_X] > gantry_vel_max_mm_s
        || vel_mm_s[AXIS_Y] < gantry_vel_min_mm_s || vel_mm_s[AXIS_Y] > gantry_vel_max_mm_s) {
        cm_msg(
            MERROR,
            "validate_move_params",
            "Velocity should be between %f mm/s and %f mm/s inclusive.\n",
            gantry_vel_min_mm_s, gantry_vel_max_mm_s);
        return false;
    }

    return true;
}

static int32_t mm_to_cts(float val_mm) {
    return round(val_mm / mm_cts_ratio);
}

static uint32_t mm_to_steps(float val_mm) {
    return round(val_mm / mm_steps_ratio);
}

static AxisDirection get_direction(int32_t displacement) {
    return (displacement < 0 ? AXIS_DIR_NEGATIVE : AXIS_DIR_POSITIVE);
}

static bool handle_serial_result(SerialResult res) {
    if (res == SERIAL_OK) return true;

    printf("Serial Error: %d\n", res);
    return false;
}

static bool handle_axis_result(AxisResult res) {
    if (res == AXIS_OK) return true;

    printf("Axis Error: %d\n", res);
    return false;
}

static bool attempt_move_axis(AxisId axis, int32_t cur_pos_counts, float dest_mm, float vel_mm_s)
{
    // Target position
    int32_t target_counts = mm_to_cts(dest_mm);

    // Relative displacement
    int32_t disp_counts = (target_counts - cur_pos_counts);

    // Direction
    AxisDirection dir = get_direction(disp_counts);

    // Velocity
    uint32_t vel_steps_s = mm_to_steps(vel_mm_s);

    // Send command
    SerialResult ser_res;
    AxisResult axis_res;
    ser_res = comm.move(axis, dir, vel_steps_s, abs(disp_counts), &axis_res, MSG_RECEIVE_TIMEOUT);

    // Handle results
    return (handle_serial_result(ser_res) && handle_axis_result(axis_res));
}

/*****************************************************************************/
/*                             PUBLIC FUNCTIONS                              */
/*****************************************************************************/

/**
 * @brief Establishes a serial connection to the Arduino
 * 
 * This includes opening the serial device as well as waiting to receive a
 * ping message from the Arduino to validate that it is running
 * 
 * @return true if the connection was successfully established, otherwise false
 */
bool arduino_connect(char *device_file)
{
    // Open the serial device
    device.set_device_file(device_file);
    if (!device.ser_connect(BAUD_RATE)) return false;
    device.ser_flush();
    
    printf("Waiting for Arduino...");
    while (!(comm.check_for_message() && comm.received_message().id == MSG_ID_PING)) {
        // Wait for ping
    }
    // There might be more ping messages sitting in the buffer, so flush them all out
    device.ser_flush();
    printf("Connected!\n");

    return true;
}

/**
 * @brief Disconnects from the Arduino
 */
void arduino_disconnect()
{
    device.ser_disconnect();
}

/**
 * @brief Attempts to command the Arduino to move to the provided destination at the
 *        provided velocity
 * 
 * @param dest_mm   Pointer to two floats (the absolute x and y coordinates in mm)
 * @param vel_mm_s  Pointer to two floats (the x and y velocities in mm/s)
 */
void arduino_attempt_move(float *dest_mm, float *vel_mm_s)
{
    if (!validate_move_params(dest_mm, vel_mm_s)) return;

    // Get current position
    PositionMsgData cur_pos_counts;
    if (!handle_serial_result(comm.get_position(&cur_pos_counts, MSG_RECEIVE_TIMEOUT))) return;

    // Attempt movement
    bool x_success = attempt_move_axis(AXIS_X, cur_pos_counts.x_counts, dest_mm[AXIS_X], vel_mm_s[AXIS_X]);
    bool y_success = attempt_move_axis(AXIS_Y, cur_pos_counts.y_counts, dest_mm[AXIS_Y], vel_mm_s[AXIS_Y]);

    // Handle results
    if (x_success && y_success) {
        printf("Moving to position (%f mm, %f mm)\n", dest_mm[AXIS_X], dest_mm[AXIS_Y]);
        printf("Moving with velocity (%f mm/s, %f mm/s)\n", vel_mm_s[AXIS_X], vel_mm_s[AXIS_Y]);
    }
    else {
        // Stop both axis if anything went wrong
        handle_serial_result(comm.stop());
    }
}

/**
 * @brief Tells the Arduino to start the homing routine
 */
void arduino_run_home()
{
    handle_serial_result(comm.home());
}
