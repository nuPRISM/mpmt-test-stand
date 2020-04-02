#ifndef KINEMATICS_H
#define KINEMATICS_H

#include <stdint.h>

typedef struct {
    uint32_t dist_accel;
    uint32_t dist_hold;
    uint32_t dist_decel;
} VelProfile;

bool generate_vel_profile(uint32_t accel, uint32_t v_start, uint32_t v_hold, uint32_t dist_total, VelProfile *profile_out);

#endif // KINEMATICS_H