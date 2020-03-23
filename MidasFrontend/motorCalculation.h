#ifndef MOTOR_CALCULATION_H
#define MOTOR_CALCULATION_H

#include <cmath>
#include <iostream>

#include "Messages.h" 
#include "shared_defs.h"

const float encoder_cpr = 300.0;
const float lead_screw_pitch_mm = 8.0;
const float gear_ratio = 26.0 + 103.0/121.0;
const float mm_cts_ratio =  lead_screw_pitch_mm/(encoder_cpr*gear_ratio);

Direction get_direction(uint32_t curr_pos_cts, float dest_mm);
float cts_to_mm(uint32_t counts);
uint32_t mm_to_cts(float distance);
uint32_t abs_distance_to_rel_cts(uint32_t curr_counts, float dest_mm);

#endif // MOTOR_CALCULATION_H