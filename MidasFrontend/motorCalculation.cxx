#include "motorCalculation.h"

using namespace std;

//retreive values from memory
float curr_pos_cts;
//user inputs
float dest_mm, vel_mm_s, accel_mm_s2;

/*
Returns the direction to move the gantry.

Parameters:
dest_mm: desired absolute position of gantry in millimeters
curr_pos_mm: current absolute position of gantry in millimeters 
*/
Direction get_direction(float dest_mm, uint32_t curr_pos_cts) {
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
Returns encoder counts given value based in millimeters.
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
    uint32_t rel_dist_cts = abs(abs_dist_cts - curr_counts);
    return rel_dist_cts;
}

