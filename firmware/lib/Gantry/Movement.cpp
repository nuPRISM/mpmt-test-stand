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

#define ACCEL_HOMING_A 8000
#define VEL_HOMING_A   10000
#define ACCEL_HOMING_B 8000
#define VEL_HOMING_B   10000

bool move_axis_rel(AxisId axis_id, Direction dir, uint32_t accel, uint32_t hold_vel, uint32_t dist)
{
    // Generate a velocity profile for the correct axis
    VelProfile profile;
    if (!generate_vel_profile(accel, VEL_START, hold_vel, dist, &profile)) return false;

    AxisMotion motion = {
        .dir = dir,
        .vel_start = VEL_START,
        .accel = accel,
        .counts_accel = (dir == DIR_POSITIVE ? (int32_t)profile.counts_accel : -1*(int32_t)profile.counts_accel),
        .counts_hold  = (dir == DIR_POSITIVE ? (int32_t)profile.counts_hold  : -1*(int32_t)profile.counts_hold),
        .counts_decel = (dir == DIR_POSITIVE ? (int32_t)profile.counts_decel : -1*(int32_t)profile.counts_decel),
    };

    AxisResult result = axis_start(axis_id, &motion);
    return (result == AXIS_OK);
}

bool move_axis_home_a(AxisId axis_id)
{
    return move_axis_rel(axis_id, DIR_NEGATIVE, ACCEL_HOMING_A, VEL_HOMING_A, INT32_MAX);
}

bool move_axis_home_b(AxisId axis_id)
{
    return move_axis_rel(axis_id, DIR_POSITIVE, ACCEL_HOMING_B, VEL_HOMING_B, INT32_MAX);
}
