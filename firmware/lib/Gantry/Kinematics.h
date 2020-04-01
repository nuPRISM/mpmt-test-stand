#ifndef KINEMATICS_H
#define KINEMATICS_H

#include <stdint.h>

typedef struct {
    uint32_t dist_accel;
    uint32_t dist_hold;
    uint32_t dist_decel;
} VelProfile;

// uint32_t calc_expected_velocity(uint32_t accel, uint32_t v_min, uint32_t elapsed_counts);
bool generate_vel_profile(uint32_t accel, uint32_t v_start, uint32_t v_hold, uint32_t dist_total, VelProfile *profile_out);

#endif // KINEMATICS_H