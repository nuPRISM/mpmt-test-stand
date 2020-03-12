#include "Kinematics.h"

#define MOTOR_MICROSTEP        4
#define MOTOR_STEPS_PER_REV    (200 * MOTOR_MICROSTEP)
#define ENCODER_COUNTS_PER_REV 300

static uint32_t calc_counts_accel(uint32_t accel, uint32_t v_min, uint32_t v_max)
{
    // Calculate number of motor steps during which to accelerate
    uint32_t steps_accel = ((v_max * v_max) - (v_min * v_min)) / (2 * accel);

    // Convert to number of encoder counts
    return (steps_accel * ENCODER_COUNTS_PER_REV / MOTOR_STEPS_PER_REV);
}

bool generate_vel_profile(uint32_t accel, uint32_t v_min, uint32_t v_max, uint32_t counts_total, VelProfile *profile_out)
{
    if (accel == 0) return false;

    uint32_t counts_accel = calc_counts_accel(accel, v_min, v_max);
    uint32_t counts_hold;
    
    if ((counts_accel * 2) > counts_total) {
        // Not enough distance to accelerate hold and decelerate
        // Instead follow a triangular profile (accelerate to halfway then decelerate)
        counts_accel = counts_total / 2;
        counts_hold = 0;
    }
    else {
        counts_hold = (counts_total - (2 * counts_accel));
    }

    profile_out->counts_accel = counts_accel;
    profile_out->counts_hold = counts_hold;
    profile_out->counts_accel = counts_accel;

    return true;
}