/* **************************** Local Includes ***************************** */
#include "Movement.h"
#include "Kinematics.h"
#include "Axis.h"
#include "Debug.h"

/* ************************ Shared Project Includes ************************ */
#include "shared_defs.h"

/* **************************** System Includes **************************** */
#include <stdint.h>

#define VEL_START 500

#define ACCEL_HOMING 8000
#define VEL_HOMING   10000

bool move_axis_rel(AxisId axis_id, Direction dir, uint32_t accel, uint32_t hold_vel, uint32_t dist)
{
    // Generate a velocity profile for the correct axis
    VelProfile profile;
    if (!generate_vel_profile(accel, VEL_START, hold_vel, dist, &profile)) return false;

    DEBUG_PRINT_VAL("cts_a", profile.counts_accel);
    DEBUG_PRINT_VAL("cts_h", profile.counts_hold);

    AxisMotion motion = {
        .dir = dir,
        .vel_start = VEL_START,
        .accel = accel,
        .counts_accel = (dir == DIR_POSITIVE ? (int32_t)profile.counts_accel : -1*(int32_t)profile.counts_accel),
        .counts_hold  = (dir == DIR_POSITIVE ? (int32_t)profile.counts_hold  : -1*(int32_t)profile.counts_hold),
        .counts_decel = (dir == DIR_POSITIVE ? (int32_t)profile.counts_decel : -1*(int32_t)profile.counts_decel),
    };

    AxisResult result = axis_start(axis_id, &motion);
    DEBUG_PRINT_VAL("AxisResult", result);
    return (result == AXIS_OK);
}

bool move_axis_home(AxisId axis_id)
{
    return move_axis_rel(axis_id, DIR_NEGATIVE, ACCEL_HOMING, VEL_HOMING, INT32_MAX);
}

// void home_axis(Axis *axis)
// {
//     axis->homing = true;
//     // math to calculate number of accel counts
//     uint32_t counts_accel = calc_counts_accel(axis->accel, axis->vel_min, VELOCITY_HOMING);
//     // check if the limit switched is pressed at home
//     LimitSwitchStatus status = (LimitSwitchStatus)digitalRead(axis->ls_home.pin);
//     DEBUG_PRINT_VAL("LS STATUS", status);
//     if (status == PRESSED) {
//         DEBUG_PRINT_VAL("DRIVING IN POSITIVE", 0);
//         axis->encoder.current = 0;
//         axis_trapezoidal_move_rel(axis, counts_accel, UINT32_MAX/2, counts_accel, DIR_POSITIVE); // move until limit switch is released
//         return;
//     }
//     else {
//         DEBUG_PRINT_VAL("DRIVING IN NEGATIVE", 0);
//         axis->encoder.current = UINT32_MAX;
//         axis_trapezoidal_move_rel(axis, counts_accel, UINT32_MAX/2, counts_accel, DIR_NEGATIVE);
//         return;
//     }
// }