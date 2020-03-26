#ifndef MOTOR_CALCULATION_H
#define MOTOR_CALCULATION_H

#include <cmath>
#include <iostream>

#include "shared_defs.h"

Direction get_direction(uint32_t curr_pos_cts, float dest_mm);
float cts_to_mm(uint32_t counts);
uint32_t mm_to_cts(float val_mm);
uint32_t abs_distance_to_rel_cts(uint32_t curr_counts, float dest_mm);

#endif // MOTOR_CALCULATION_H
