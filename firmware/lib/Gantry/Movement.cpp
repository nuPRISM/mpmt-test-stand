/* **************************** Local Includes ***************************** */
#include "Movement.h"
#include "Kinematics.h"
#include "Axis.h"
#include "Debug.h"

/* ************************ Shared Project Includes ************************ */
#include "shared_defs.h"

/* **************************** System Includes **************************** */
#include <stdint.h>

/**
 * @brief Instructs an axis to execute a relative movement
 * 
 * Attempts to move an axis a distance of total_counts encoder counts in the direction dir
 * according the velocities/acceleration defined by shape and unit conversions provided by units.
 * 
 * @param axis_id       The AxisId identifying the axis
 * @param dir           The direction to move (either DIR_POSITIVE or DIR_NEGATIVE)
 * @param total_counts  The total distance [encoder counts]
 * @param shape         Pointer to a MotionShape struct specifying the derivatives of the motion
 * @param units         Pointer to a MotionUnits struct specifying the conversion ratios between
 *                      encoder counts and motor steps
 * 
 * @return true if the axis successfully started executing the motion, otherwise false
 */
bool move_axis_rel(
    AxisId axis_id, Direction dir, uint32_t total_counts,
    MotionShape shape, MotionUnits units)
{
    // Convert from steps to counts
    uint32_t accel = shape.accel * units.counts_per_rev / units.steps_per_rev;
    uint32_t vel_start = shape.vel_start * units.counts_per_rev / units.steps_per_rev;
    uint32_t vel_hold = shape.vel_hold * units.counts_per_rev / units.steps_per_rev;

    // Generate a velocity profile for the correct axis
    VelProfile profile;
    bool valid_profile = generate_vel_profile(accel, vel_start, vel_hold, total_counts, &profile);
    if (!valid_profile) return false;

    AxisMotion motion = {
        .dir = dir,
        .vel_start = shape.vel_start,
        .vel_hold = shape.vel_hold,
        .accel = shape.accel,
        .counts_accel = (dir == DIR_POSITIVE ? (int32_t)profile.dist_accel : -1*(int32_t)profile.dist_accel),
        .counts_hold  = (dir == DIR_POSITIVE ? (int32_t)profile.dist_hold  : -1*(int32_t)profile.dist_hold),
        .counts_decel = (dir == DIR_POSITIVE ? (int32_t)profile.dist_decel : -1*(int32_t)profile.dist_decel),
    };

    // TODO actually return the AxisResult here
    return (axis_start(axis_id, &motion) == AXIS_OK);
}

bool move_axis_home_a(AxisId axis_id, MotionShape shape, MotionUnits units)
{
    return move_axis_rel(axis_id, DIR_NEGATIVE, INT32_MAX, shape, units);
}

bool move_axis_home_b(AxisId axis_id, MotionShape shape, MotionUnits units)
{
    return move_axis_rel(axis_id, DIR_POSITIVE, INT32_MAX, shape, units);
}
