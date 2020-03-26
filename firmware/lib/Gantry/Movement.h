#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "Axis.h"

void axis_trapezoidal_move_rel(Axis *axis, uint32_t counts_accel, uint32_t counts_hold, uint32_t counts_decel, Direction dir);
void home_axis(Axis *axis);

#endif // MOVEMENT_H