#include "Kinematics.h"

void axis_trapezoidal_move_rel(Axis *axis, uint32_t vel_max, uint32_t counts_accel, uint32_t counts_const, uint32_t counts_decel, int dir)
{   
    axis->dir = dir;
    digitalWrite(axis->dir_pin, dir);

    if (vel_max < axis->vel_max && vel_max != 0) {
        axis->vel_max = vel_max;
    }

    if (dir == positive) {
        axis->vel_profile_cur_trap[0] = counts_accel;
        axis->vel_profile_cur_trap[1] = counts_const;
        axis->vel_profile_cur_trap[2] = counts_decel;
    }
    else if (dir == negative) {
        axis->vel_profile_cur_trap[0] = - counts_accel;
        axis->vel_profile_cur_trap[1] = - counts_const;
        axis->vel_profile_cur_trap[2] = - counts_decel;
    }
    
    axis->encoder.desired = axis->encoder.current + axis->vel_profile_cur_trap[0];
    
    // print("Desired count: ", axis->encoder.desired);
    // print("Current count: ", axis->encoder.current);
    start_timer(axis->timer, axis->channel_velocity, axis->isr_velocity, axis->vel);
    start_timer_accel(axis->timer, axis->channel_accel, axis->isr_accel, axis->accel);
}

void axis_trapezoidal_move_tri(Axis *axis, uint32_t vel_max, uint32_t counts_accel, uint32_t counts_decel, int dir)
{   
    axis_trapezoidal_move_rel(axis, vel_max, counts_accel, 0, counts_decel, dir);
}


void axis_trapezoidal_move_abs(Axis *axis, uint32_t vel_max, uint32_t counts_accel, uint32_t counts_const, uint32_t counts_decel, int dir)
{

}

void home_axis(Axis *axis)
{   
    axis->homing = 1;
    // check if the limit switched is pressed at home
    if (digitalRead(axis->ls_home.pin) == pressed) {
        axis_trapezoidal_move_rel(axis, VELOCITY_HOMING, 300, 1000000, 300, positive); // move until limit switch is depressed
        return;
    }
    else {
        axis_trapezoidal_move_rel(axis, VELOCITY_HOMING, 300, 1000000, 300, negative);
        return;
    }
}