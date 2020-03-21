#include "Movement.h"

#include "Debug.h"
#include "Timer.h"

#include "shared_defs.h"

void axis_trapezoidal_move_rel(Axis *axis, uint32_t counts_accel, uint32_t counts_hold, uint32_t counts_decel, Direction dir)
{   
    axis->tragectory_segment = VEL_SEG_ACCELERATE;
    axis->vel = axis->vel_min;
    if (counts_accel == 0 && counts_hold == 0 && counts_decel == 0) return;
    axis->dir = dir;
    digitalWrite(axis->dir_pin, dir);

    if (dir == DIR_POSITIVE) {
        axis->vel_profile_cur_trap[0] = counts_accel;
        axis->vel_profile_cur_trap[1] = counts_hold;
        axis->vel_profile_cur_trap[2] = counts_decel;
    }
    else if (dir == DIR_NEGATIVE) {
        axis->vel_profile_cur_trap[0] = - counts_accel;
        axis->vel_profile_cur_trap[1] = - counts_hold;
        axis->vel_profile_cur_trap[2] = - counts_decel;
    }

    axis->encoder.desired = axis->encoder.current + axis->vel_profile_cur_trap[0];

    DEBUG_PRINT("Desired count: ", axis->encoder.desired);
    DEBUG_PRINT("Current count: ", axis->encoder.current);
    start_timer(axis->timer, axis->channel_velocity, axis->irq_velocity, axis->vel);
    start_timer_accel(axis->timer, axis->channel_accel, axis->irq_accel, axis->accel);
}

void home_axis(Axis *axis)
{
    axis->homing = 1;
    // math to calculate number of accel counts
    uint32_t counts_accel = calc_counts_accel(axis->accel, axis->vel_min, VELOCITY_HOMING);
    // check if the limit switched is pressed at home
    LimitSwitchStatus status = (LimitSwitchStatus)digitalRead(axis->ls_home.pin);
    DEBUG_PRINT("LS STATUS: ", status);
    if (status == PRESSED) {
        DEBUG_PRINT("DRIVING IN POSITIVE", 0);
        axis->encoder.current = 0;
        axis_trapezoidal_move_rel(axis, counts_accel, UINT32_MAX/2, counts_accel, DIR_POSITIVE); // move until limit switch is released
        return;
    }
    else {
        DEBUG_PRINT("DRIVING IN NEGATIVE", 0);
        axis->encoder.current = UINT32_MAX;
        axis_trapezoidal_move_rel(axis, counts_accel, UINT32_MAX/2, counts_accel, DIR_NEGATIVE);
        return;
    }
}

void stop_axis(Axis *axis)
{
    NVIC_DisableIRQ(axis->irq_accel);
    NVIC_DisableIRQ(axis->irq_velocity);
}