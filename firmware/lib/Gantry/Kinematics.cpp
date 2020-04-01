#include "Kinematics.h"

#include <math.h>

// /**
//  * @brief Calculate the square of the expected velocity after accelerating for a specified distance
//  * 
//  * @param accel          The acceleration rate [motor steps / s^2]
//  * @param v_min          The starting velocity [motor steps / s]
//  * @param elapsed_counts The elapsed number of encoder counts
//  */
// uint32_t calc_expected_velocity(uint32_t accel, uint32_t v_min, uint32_t elapsed_counts)
// {
//     uint32_t elapsed_steps = elapsed_counts * MOTOR_STEPS_PER_REV / ENCODER_COUNTS_PER_REV;
//     return (uint32_t)sqrt((2 * accel * elapsed_steps) + (v_min * v_min));
// }

/**
 * @brief Calculate the distance you must travel while accelerating at rate a from v_0 to reach v_f
 * 
 * @param a   The acceleration rate [distance / time^2], cannot be zero
 * @param v_0 The initial velocity  [distance / time]
 * @param v_f The final velocity    [distance / time], must satisfy v_f >= v_0
 * 
 * @return The number of encoder counts during which to accelerate
 */
static uint32_t calc_dist_accel(uint32_t a, uint32_t v_0, uint32_t v_f)
{
    return ((v_f * v_f) - (v_0 * v_0)) / (2 * a);
}

/**
 * @brief Generate a trapezoidal velocity profile based on a set of kinematic parameters
 * 
 * The velocity profile is a specification for the duration of each phase of motion (acceleration, holding, deceleration).
 * 
 * The units for the calculation will match the units used for the parameters (all parameters must use consistent units).
 * See the description of each parameter for details (units specified in square brackets).
 * 
 * @param accel       The acceleration rate [distance / time^2], cannot be zero unless v_hold == v_start
 * @param v_start     The starting velocity [distance / time] (also the ending velocity)
 * @param v_hold      The holding velocity  [distance / time], must satisfy v_hold >= v_start
 * @param dist_total  The total distance    [distance]
 * @param profile_out Pointer to a VelProfile where the final values will be placed
 * 
 * @return true if a valid velocity profile could be generated for the given parameters, otherwise false
 */
bool generate_vel_profile(
    uint32_t accel, uint32_t v_start, uint32_t v_hold,
    uint32_t dist_total,
    VelProfile *profile_out)
{
    if (accel != 0 && v_hold >= v_start) {
        uint32_t dist_accel = calc_dist_accel(accel, v_start, v_hold);

        uint32_t dist_hold;
        if ((dist_accel * 2) > dist_total) {
            // Not enough distance to accelerate, hold, and decelerate
            // Instead follow a triangular profile (accelerate to halfway then decelerate)
            dist_accel = dist_total / 2;
            dist_hold = 0;
        }
        else {
            // counts_hold will be however many counts are left after accel and decel
            dist_hold = (dist_total - (2 * dist_accel));
        }

        profile_out->dist_accel = dist_accel;
        profile_out->dist_hold = dist_hold;
        profile_out->dist_decel = dist_accel;
        return true;
    }
    else if (v_hold == v_start) {
        // start and hold velocities are the same --> no acceleration or deceleration required
        profile_out->dist_accel = 0;
        profile_out->dist_hold = dist_total;
        profile_out->dist_decel = 0;
        return true;
    }
    else {
        return false;
    }
}
