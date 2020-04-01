#ifndef MOVEMENT_H
#define MOVEMENT_H

/* ************************ Shared Project Includes ************************ */
#include "shared_defs.h"

/* **************************** System Includes **************************** */
#include <stdint.h>

typedef struct {
    uint32_t accel;          //!< The acceleration rate [motor steps / time^2], cannot be zero unless v_hold == v_start
    uint32_t vel_start;      //!< The starting velocity [motor steps / time] (also the ending velocity)
    uint32_t vel_hold;       //!< The holding velocity  [motor steps / time], must satisfy v_hold >= v_start
} MotionShape;

typedef struct {
    uint32_t counts_per_rev; //!< The number of encoder counts in one revolution of the motor
    uint32_t steps_per_rev;  //!< The number of motor steps in one revolution of the motor
} MotionUnits;

bool move_axis_rel(
    AxisId axis_id, Direction dir, uint32_t total_counts,
    MotionShape shape, MotionUnits units);

bool move_axis_home_a(AxisId axis_id, MotionShape shape, MotionUnits units);
bool move_axis_home_b(AxisId axis_id, MotionShape shape, MotionUnits units);

#endif // MOVEMENT_H