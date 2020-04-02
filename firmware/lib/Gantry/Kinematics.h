#ifndef KINEMATICS_H
#define KINEMATICS_H

#include <stdint.h>

typedef struct {
    int32_t dist_accel; //!< Distance for accelerating
    int32_t dist_hold;  //!< Distance at constant velocity
    int32_t dist_decel; //!< Distance for decelerating
} VelProfile;

bool generate_vel_profile(
    bool neg,
    uint32_t accel, uint32_t v_start, uint32_t v_hold,
    uint32_t dist_total,
    VelProfile *profile_out);

#endif // KINEMATICS_H