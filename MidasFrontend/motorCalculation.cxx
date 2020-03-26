#include "motorCalculation.h"

using namespace std;

const float encoder_cpr = 300.0; //Note: new gantry encoder is 500 counts
const float lead_screw_pitch_mm = 8.0;
const float gear_ratio = 26.0 + 103.0/121.0;
const float mm_cts_ratio =  lead_screw_pitch_mm/(encoder_cpr*gear_ratio);

/**
 * This file contains functionality to convert user input values 
 * based in milimeters (destination, velocity), and converts them 
 * to parameters needed for motor commands.
 * / 

/*
Returns the direction to move the gantry.

Parameters:
dest_mm: desired absolute position of gantry in millimeters
curr_pos_cts: current absolute position of gantry in counts.
              This will be a value returned from the motor encoder when requested.
*/
Direction get_direction(uint32_t curr_pos_cts, float dest_mm) {
    uint32_t dest_cts = mm_to_cts(dest_mm);
    uint32_t diff_cts = dest_cts - curr_pos_cts;

    if (diff_cts < 0){
        return DIR_NEGATIVE;
    } else{
        return DIR_POSITIVE; //defaults to positive if dest_cts = curr_pos_cts
    }
}

/*
Returns distance travelled in millimeters given encoder counts.
*/
float cts_to_mm(uint32_t counts){
    return round(counts * mm_cts_ratio);
}

/*
Returns encoder counts given value (val_mm) based in millimeters.
This includes: distance (mm), velocity (mm/s), acceleration (mm/s^2)
*/
uint32_t mm_to_cts(float val_mm){
    return round(val_mm / mm_cts_ratio);
}

/*
Returns encoder counts given distance in millimeters.
*/
uint32_t abs_distance_to_rel_cts(uint32_t curr_counts, float dest_mm){
    uint32_t abs_dist_cts = mm_to_cts(dest_mm);
    uint32_t rel_dist_cts = abs((int32_t)abs_dist_cts - (int32_t)curr_counts);
    return rel_dist_cts;
}
