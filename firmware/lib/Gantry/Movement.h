#ifndef MOVEMENT_H
#define MOVEMENT_H

/* ************************ Shared Project Includes ************************ */
#include "shared_defs.h"

/* **************************** System Includes **************************** */
#include <stdint.h>

bool move_axis_rel(AxisId axis_id, Direction dir, uint32_t accel, uint32_t hold_vel, uint32_t dist);
bool move_axis_home_a(AxisId axis_id);
bool move_axis_home_b(AxisId axis_id);

#endif // MOVEMENT_H