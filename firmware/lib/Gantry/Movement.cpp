#include "Movement.h"

#include "Debug.h"
#include "Timer.h"

void axis_trapezoidal_move_rel(Axis *axis, uint32_t counts_accel, uint32_t counts_hold, uint32_t counts_decel, Direction dir)
{
    if (counts_accel == 0 && counts_hold == 0 && counts_decel == 0) return;
    axis->dir = dir;
    digitalWrite(axis->dir_pin, dir);

    if (dir == POSITIVE) {
        axis->vel_profile_cur_trap[0] = counts_accel;
        axis->vel_profile_cur_trap[1] = counts_hold;
        axis->vel_profile_cur_trap[2] = counts_decel;
    }
    else if (dir == NEGATIVE) {
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
    uint32_t counts_accel;
    // check if the limit switched is pressed at home
    if (digitalRead(axis->ls_home.pin) == PRESSED) {
        axis_trapezoidal_move_rel(axis, counts_accel, UINT32_MAX, counts_accel, POSITIVE); // move until limit switch is depressed
        return;
    }
    else {
        axis_trapezoidal_move_rel(axis, counts_accel, UINT32_MAX, counts_accel, NEGATIVE);
        return;
    }
}

void stop_axis(Axis *axis)
{
    NVIC_DisableIRQ(axis->irq_accel);
    NVIC_DisableIRQ(axis->irq_velocity);
}