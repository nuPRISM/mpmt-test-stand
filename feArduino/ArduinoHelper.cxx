#include "ArduinoHelper.h"
#include "TestStandMessages.h"

#include "Gantry.h"
#include "TempMeasure.h"

#include "LinuxSerialDevice.h"
#include "TestStandCommHost.h"

#include "shared_defs.h"

#include <stdio.h>
#include <math.h>

/*****************************************************************************/
/*                                 CONSTANTS                                 */
/*****************************************************************************/

// Gantry Mechanical Properties
const float pulley_diameter_mm   = 17.0;
const float dist_per_rev_mm      = 2.0 * M_PI * (pulley_diameter_mm / 2.0);
const float mm_cts_ratio         = dist_per_rev_mm / ENCODER_COUNTS_PER_REV;
const float mm_steps_ratio       = dist_per_rev_mm / MOTOR_STEPS_PER_REV;

// Operational Bounds
const float gantry_x_min_mm      = 0.0;
const float gantry_x_max_mm      = 1200.0; // max rail is 1219 mm
const float gantry_y_min_mm      = 0.0;
const float gantry_y_max_mm      = 1200.0;
const float gantry_vel_min_mm_s  = 0.0;    // [mm/s]
const float gantry_vel_max_mm_s  = 10.0;   // [mm/s]

const char * serial_result_msgs[] = {
    [SERIAL_OK]                  = "Send or receive completed successfully",
    [SERIAL_OK_NO_MSG]           = "No message was received, but that was expected",
    [SERIAL_ERR_NO_MSG]          = "No message was received and one was expected",
    [SERIAL_ERR_TIMEOUT]         = "A message was not received within the allotted timeout",
    [SERIAL_ERR_MSG_IN_PROGRESS] = "A partial message has been received when another operation started",
    [SERIAL_ERR_SEND_FAILED]     = "Message failed to send",
    [SERIAL_ERR_NO_ACK]          = "After sending a message, the response was not an ACK",
    [SERIAL_ERR_ACK_FAILED]      = "After receiving a message, failed to send an ACK",
    [SERIAL_ERR_WRONG_MSG]       = "An unexpected message was received",
    [SERIAL_ERR_DATA_LENGTH]     = "Wrong length of data was received",
    [SERIAL_ERR_DATA_CORRUPT]    = "Received serial data was corrupted"
};

const char * axis_result_msgs[] = {
    [AXIS_OK]                  = "Axis movement started OK",
    [AXIS_ERR_ALREADY_MOVING]  = "Axis is already moving",
    [AXIS_ERR_LS_HOME]         = "Trying to move backward while HOME limit switch is pressed",
    [AXIS_ERR_LS_FAR]          = "Trying to move forward while FAR limit switch is pressed",
    [AXIS_ERR_INVALID]         = "The parameters resulted in an invalid motion profile"
};

/*****************************************************************************/
/*                             PRIVATE VARIABLES                             */
/*****************************************************************************/

static LinuxSerialDevice device;
static TestStandCommHost comm(device);

/*****************************************************************************/
/*                             PRIVATE FUNCTIONS                             */
/*****************************************************************************/

static bool validate_move_params(float *dest_mm, float *vel_mm_s)
{
    if (dest_mm[AXIS_X] < gantry_x_min_mm || dest_mm[AXIS_X] > gantry_x_max_mm) {
        cm_msg(
            MERROR,
            "validate_move_params",
            "Destination on x-axis should be between %f mm and %f mm inclusive.",
            gantry_x_min_mm, gantry_x_max_mm);
        return false;
    }

    if (dest_mm[AXIS_Y] < gantry_y_min_mm || dest_mm[AXIS_Y] > gantry_y_max_mm) {
        cm_msg(
            MERROR,
            "validate_move_params",
            "Destination on y-axis should be between %f mm and %f mm inclusive.",
            gantry_y_min_mm, gantry_y_max_mm);
        return false;
    }

    if (vel_mm_s[AXIS_X] < gantry_vel_min_mm_s || vel_mm_s[AXIS_X] > gantry_vel_max_mm_s
        || vel_mm_s[AXIS_Y] < gantry_vel_min_mm_s || vel_mm_s[AXIS_Y] > gantry_vel_max_mm_s) {
        cm_msg(
            MERROR,
            "validate_move_params",
            "Velocity should be between %f mm/s and %f mm/s inclusive.",
            gantry_vel_min_mm_s, gantry_vel_max_mm_s);
        return false;
    }

    return true;
}

static int32_t mm_to_cts(float val_mm)
{
    return round(val_mm / mm_cts_ratio);
}

static float cts_to_mm(int32_t val_cts)
{
    return (val_cts * mm_cts_ratio);
}

static uint32_t mm_to_steps(float val_mm)
{
    return round(val_mm / mm_steps_ratio);
}

static AxisDirection get_direction(int32_t displacement)
{
    return (displacement < 0 ? AXIS_DIR_NEGATIVE : AXIS_DIR_POSITIVE);
}

static bool handle_serial_result(SerialResult res)
{
    if (res == SERIAL_OK) return true;

    cm_msg(MERROR, "handle_serial_result", "Serial Error (%d): %s\n", res, serial_result_msgs[res]);
    return false;
}

static bool handle_axis_result(AxisId axis, AxisResult res)
{
    if (res == AXIS_OK) return true;

    cm_msg(MERROR, "handle_axis_result", "Axis Error (%d) on %c axis: %s\n", res, (axis == AXIS_X ? 'X' : 'Y'), axis_result_msgs[res]);
    return false;
}

static bool move_axis(AxisId axis, int32_t cur_pos_counts, float dest_mm, float vel_mm_s)
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
    ser_res = comm.move(axis, dir, vel_steps_s, abs(disp_counts), &axis_res, MSG_RECEIVE_TIMEOUT_MS);

    // Handle results
    return (handle_serial_result(ser_res) && handle_axis_result(axis, axis_res));
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
 * @param device_file Path to the serial port's device file (e.g. /dev/ttyACM0)
 * 
 * @return true if the connection was successfully established, otherwise false
 */
bool arduino_connect(char *device_file)
{
    // Open the serial device
    device.set_device_file(device_file);
    if (!device.ser_connect(SERIAL_BAUD_RATE)) return false;
    device.ser_flush();

    printf("Waiting for Arduino...");
    while (!(comm.check_for_message() && comm.received_message().id == MSG_ID_PING));
    // There might be more ping messages sitting in the buffer, so flush them all out
    device.ser_flush();
    printf("Connected!\n");

    // Verify link
    printf("Verifying link...");
    if (!handle_serial_result(comm.link_check(MSG_RECEIVE_TIMEOUT_MS))) return false;
    printf("SUCCESS\n");

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
bool arduino_move(float *dest_mm, float *vel_mm_s)
{
    if (!validate_move_params(dest_mm, vel_mm_s)) return false;

    // Get current position
    PositionMsgData cur_pos_counts;
    if (!handle_serial_result(comm.get_position(&cur_pos_counts, MSG_RECEIVE_TIMEOUT_MS))) return false;

    // Attempt movement
    bool x_success = move_axis(AXIS_X, cur_pos_counts.x_counts, dest_mm[AXIS_X], vel_mm_s[AXIS_X]);
    bool y_success = move_axis(AXIS_Y, cur_pos_counts.y_counts, dest_mm[AXIS_Y], vel_mm_s[AXIS_Y]);

    // Handle results
    if (x_success && y_success) {
        cm_msg(MINFO, "arduino_move", "Moving to position (%.2f mm, %.2f mm) with velocity (%.2f mm/s, %.2f mm/s)",
                dest_mm[AXIS_X], dest_mm[AXIS_Y],
                vel_mm_s[AXIS_X], vel_mm_s[AXIS_Y]);
        return true;
    }
    // Error messages will have been printed by move_axis
    return false;
}

/**
 * @brief Tells the Arduino to start the homing routine
 */
bool arduino_run_home()
{
    return handle_serial_result(comm.home());
}

/**
 * @brief Tells the Arduino to cease all motor functions
 */
bool arduino_stop()
{
    return handle_serial_result(comm.stop());
}

/**
 * @brief Retrieves the current status of the Arduino
 * 
 * @param status_out Pointer to where the status should be stored
 * 
 * @return true if the status was retrieved successfully, otherwise false
 */
bool arduino_get_status(DWORD *status_out)
{
    Status status;
    if (!handle_serial_result(comm.get_status(&status, MSG_RECEIVE_TIMEOUT_MS))) return false;
    *status_out = status;
    return true;
}

/**
 * @brief Retrieves the current position of the gantry from the Arduino
 * 
 * @param motor_x_mm_out Pointer to where X coordinate in mm should be stored
 * @param motor_y_mm_out Pointer to where Y coordinate in mm should be stored
 * 
 * @return true if the position was retrieved successfully, otherwise false
 */
bool arduino_get_position(float *gantry_x_mm_out, float *gantry_y_mm_out)
{
    // Retrieve current position
    PositionMsgData pos_counts;
    if (!handle_serial_result(comm.get_position(&pos_counts, MSG_RECEIVE_TIMEOUT_MS))) return false;
    // Convert to mm
    *gantry_x_mm_out = cts_to_mm(pos_counts.x_counts);
    *gantry_y_mm_out = cts_to_mm(pos_counts.y_counts);
    return true;
}

/**
 * @brief Retrieves the latest temperature readings from the Arduino
 * 
 * @param temp_out Pointer to a struct where the read temperatures will be stored
 * 
 * @return true if the temperatures were retrieved successfully, otherwise false
 */
bool arduino_get_temp(TempData *temp_out)
{
    return handle_serial_result(comm.get_temp(temp_out, MSG_RECEIVE_TIMEOUT_MS));
}
