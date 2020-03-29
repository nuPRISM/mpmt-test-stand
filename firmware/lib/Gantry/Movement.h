#ifndef MOVEMENT_H
#define MOVEMENT_H

/* ************************ Shared Project Includes ************************ */
#include "shared_defs.h"

/* **************************** System Includes **************************** */
#include <stdint.h>

#define VEL_START 500

bool axis_trapezoidal_move_rel(AxisId axis_id, Direction dir, uint32_t accel, uint32_t counts_accel, uint32_t counts_hold, uint32_t counts_decel);
// bool home_axis(Axis *axis);

#endif // MOVEMENT_H