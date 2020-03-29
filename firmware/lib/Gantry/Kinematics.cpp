#include "Kinematics.h"

#define MOTOR_MICROSTEP        4
#define MOTOR_STEPS_PER_REV    (200 * MOTOR_MICROSTEP)
// #define ENCODER_COUNTS_PER_REV 300
#define ENCODER_COUNTS_PER_REV MOTOR_STEPS_PER_REV

/**
 * @brief Calculate the number encoder counts you must travel while accelerating from v_min to reach v_max
 * 
 * @param accel The acceleration rate [motor steps / s^2]
 * @param v_min The starting velocity [motor steps / s]
 * @param v_max The final velocity    [motor steps / 2]
 * 
 * @return The number of encoder counts during which to accelerate
 */
static uint32_t calc_counts_accel(uint32_t accel, uint32_t v_min, uint32_t v_max)
{
    // Calculate number of motor steps during which to accelerate
    uint32_t steps_accel = ((v_max * v_max) - (v_min * v_min)) / (2 * accel);

    // Convert to number of encoder counts
    return (steps_accel * ENCODER_COUNTS_PER_REV / MOTOR_STEPS_PER_REV);
}

/**
 * @brief Generate a velocity profile based on a set of kinematic parameters
 * 
 * The velocity profile is a specification for the duration of each phase of motion (acceleration, holding, deceleration)
 * in terms of encoder counts.
 * 
 * @param accel        The acceleration rate [motor steps / s^2]
 * @param v_min        The starting velocity [motor steps / s]
 * @param v_max        The final velocity    [motor steps / 2]
 * @param counts_total The total number of encoder counts for the motion
 * @param profile_out  Pointer to a VelProfile where the final values will be placed
 * 
 * @return true if a valid velocity profile could be generated for the given parameters, otherwise false
 */
bool generate_vel_profile(uint32_t accel, uint32_t v_min, uint32_t v_max, uint32_t counts_total, VelProfile *profile_out)
{
    if (accel != 0 && v_max >= v_min) {
        uint32_t counts_accel = calc_counts_accel(accel, v_min, v_max);
        uint32_t counts_hold;
        
        if ((counts_accel * 2) > counts_total) {
            // Not enough distance to accelerate, hold, and decelerate
            // Instead follow a triangular profile (accelerate to halfway then decelerate)
            counts_accel = counts_total / 2;
            counts_hold = 0;
        }
        else {
            // counts_hold is however many counts are left after accel and decel
            counts_hold = (counts_total - (2 * counts_accel));
        }

        profile_out->counts_accel = counts_accel;
        profile_out->counts_hold = counts_hold;
        profile_out->counts_decel = counts_accel;
        return true;
    }
    else if (v_max == v_min) {
        profile_out->counts_accel = 0;
        profile_out->counts_hold = counts_total;
        profile_out->counts_decel = 0;
        return true;
    }
    else {
        return false;
    }
}
