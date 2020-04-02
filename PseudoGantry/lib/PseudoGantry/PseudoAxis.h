#ifndef PSEUDO_AXIS_H
#define PSEUDO_AXIS_H

#include "Arduino.h"
#include "PseudoEncoder.h"
#include "PseudoLimitSwitch.h"

typedef struct PseudoAxis
{
    PseudoEncoder encoder;
    uint64_t axis_length_counts;  // specified in counts
    uint64_t motor_position_current;
    uint32_t motor_dir_pin;
    PseudoLimitSwitch ls_home;
    PseudoLimitSwitch ls_far;
} PseudoAxis;

#endif // PSEUDO_AXIS_H