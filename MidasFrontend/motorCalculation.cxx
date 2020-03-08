#include <cmath>
#include <iostream> 

const float encoder_cpr = 300.0;
const float lead_screw_pitch_mm = 8.0;
const float mm_cts_ratio = encoder_cpr/lead_screw_pitch_mm;
const int negative = 0;
const int positive = 1;
//retreive values from memory
float curr_pos_cts;
//user inputs
float dest_mm, vel_mm_s, accel_mm_s2;

/*
TODO: ERROR HANDLING
* user inputs zero acceleration --> put default acceleration
* user inputs NEGATIVE anything
*/


/*
0 - positive direction
1 - negative direction
TODO: what to do when difference_mm = 0
*/
bool get_direction(float dest_mm, float curr_post_cts) {
    float difference_mm = dest_mm - curr_pos_cts/mm_cts_ratio;
    if (difference_mm < 0.0){
        return negative;
    } else{
        return positive;
    }
}
// finding the number of counts to hold constant velocity
int get_holding_velocity_cts(float curr_pos_cts, float dest_mm, float vel_mm_s, float accel_mm_s2)
{
    int counts = mm_cts_ratio * (dest_mm - curr_pos_cts/mm_cts_ratio - 1.0/accel_mm_s2 * pow(vel_mm_s,2.0));
    return abs(counts);
}

// find number of counts to accelerate or decelerate
int get_acceleration_cts(float vel_mm_s, float accel_mm_s2)
{
    int counts = mm_cts_ratio * (0.50) * pow(vel_mm_s,2.0) / accel_mm_s2;
    return counts;
}

int main ()
{
    float curr_pos_x_cts = 0.0;
    float dest_x_mm = 8.0;
    float vel_x_mm_s = 8.0;
    float accel_x_mm_s2 = 1.0;

    int counts_accel = get_acceleration_cts(vel_mm_s, accel_x_mm_s2);
    int counts_holding = get_holding_velocity_cts(curr_pos_x_cts, dest_x_mm, vel_x_mm_s, accel_x_mm_s2);
    std::cout << counts_accel << '\n';
    std::cout << counts_holding << '\n';
}
