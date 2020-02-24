#ifndef KINEMATICS_H
#define KINEMATICS_H

#include "Arduino.h"
#include "Timer.h"
#include "Encoder.h"
#include "LimitSwitch.h"

#define VELOCITY_HOMING 200

enum status{pressed, depressed};
enum direction{positive, negative};
enum segment{accelerate, hold, decelerate};

typedef struct Axis
{
    int dir_pin;
    int step_pin;
    uint32_t accel;
    uint32_t vel_max;
    uint32_t vel;
    int32_t vel_profile_cur_trap[3]; // defined in counts accelerate, hold, decelerate
    int tragectory_segment;
    Encoder encoder;
    LimitSwitch ls_home;
    LimitSwitch ls_far_from_home;
    // necessary timer info
    Tc *timer;
    uint32_t channel_velocity;
    uint32_t channel_accel;
    IRQn_Type isr_velocity;
    IRQn_Type isr_accel;
    int dir;
    bool homing;
} Axis;

void axis_trapezoidal_move_rel(Axis *axis, uint32_t vel_max, uint32_t counts_accel, uint32_t counts_const, uint32_t counts_decel, int dir);

void axis_trapezoidal_move_tri(Axis *axis, uint32_t vel_max, uint32_t counts_accel, uint32_t counts_decel, int dir);

void axis_trapezoidal_move_abs(Axis *axis, uint32_t vel_max, uint32_t counts_accel, uint32_t counts_const, uint32_t counts_decel, int dir);

void home_axis(Axis *axis);

#endif