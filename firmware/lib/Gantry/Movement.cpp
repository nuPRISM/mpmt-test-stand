/* **************************** Local Includes ***************************** */
#include "Movement.h"
#include "Axis.h"

/* ************************ Shared Project Includes ************************ */
#include "shared_defs.h"

/* **************************** System Includes **************************** */
#include <stdint.h>

bool axis_trapezoidal_move_rel(AxisId axis_id, Direction dir, uint32_t accel, uint32_t counts_accel, uint32_t counts_hold, uint32_t counts_decel)
{
    AxisMotion motion = {
        .dir = dir,
        .vel_start = VEL_START,
        .accel = accel,
        .counts_accel = (dir == DIR_POSITIVE ? (int32_t)counts_accel : -1*(int32_t)counts_accel),
        .counts_hold  = (dir == DIR_POSITIVE ? (int32_t)counts_hold  : -1*(int32_t)counts_hold),
        .counts_decel = (dir == DIR_POSITIVE ? (int32_t)counts_decel : -1*(int32_t)counts_decel),
    };

    AxisResult result = axis_start(axis_id, &motion);
    return (result == AXIS_OK);
}

// void home_axis(Axis *axis)
// {
//     axis->homing = true;
//     // math to calculate number of accel counts
//     uint32_t counts_accel = calc_counts_accel(axis->accel, axis->vel_min, VELOCITY_HOMING);
//     // check if the limit switched is pressed at home
//     LimitSwitchStatus status = (LimitSwitchStatus)digitalRead(axis->ls_home.pin);
//     DEBUG_PRINT_VAL("LS STATUS", status);
//     if (status == PRESSED) {
//         DEBUG_PRINT_VAL("DRIVING IN POSITIVE", 0);
//         axis->encoder.current = 0;
//         axis_trapezoidal_move_rel(axis, counts_accel, UINT32_MAX/2, counts_accel, DIR_POSITIVE); // move until limit switch is released
//         return;
//     }
//     else {
//         DEBUG_PRINT_VAL("DRIVING IN NEGATIVE", 0);
//         axis->encoder.current = UINT32_MAX;
//         axis_trapezoidal_move_rel(axis, counts_accel, UINT32_MAX/2, counts_accel, DIR_NEGATIVE);
//         return;
//     }
// }