#ifndef AXIS_H
#define AXIS_H

#define VELOCITY_HOMING 200

#include "Kinematics.h"
#include "LimitSwitch.h"
#include "Encoder.h"

typedef enum {PRESSED, DEPRESSED} Status;
typedef enum {POSITIVE, NEGATIVE} Direction;
typedef enum {ACCELERATE, HOLD, DECELERATE} Segment;

typedef struct {
    uint32_t dir_pin, step_pin, encoder_pin_a, encoder_pin_b, ls_home, ls_far;
    uint32_t acceleration, vel_max, vel_min;
    Tc *timer;
    uint32_t channel_velocity, channel_accel;
    IRQn_Type isr_velocity, isr_accel;
    void (*isr_encoder)(void);
    void (*isr_limit_switch)(void);
} AxisConfig;

typedef struct Axis
{
    uint32_t dir_pin;
    uint32_t step_pin;
    uint32_t accel;
    uint32_t vel_min;
    uint32_t vel_max;
    uint32_t vel;
    int32_t vel_profile_cur_trap[3] = {0, 0, 0}; // defined in counts accelerate, hold, decelerate
    Segment tragectory_segment;
    Encoder encoder;
    LimitSwitch ls_home;
    LimitSwitch ls_far_from_home;
    // necessary timer info
    Tc *timer;
    uint32_t channel_velocity;
    uint32_t channel_accel;
    IRQn_Type isr_velocity;
    IRQn_Type isr_accel;
    Direction dir;
    bool homing;
} Axis;

void setup_axis(AxisConfig *axis_config, Axis *axis);
// -------------------------------- X AXIS -------------------------------- //

extern AxisConfig axis_x_config;
extern Axis axis_x;
// isr to handle encoder of x axis
void isr_encoder_x();

// isr to handle limit switch
void isr_limit_switch_x();

// -------------------------------- Y AXIS -------------------------------- //

extern AxisConfig axis_y_config;
extern Axis axis_y;
// isr to handle encoder of x axis
void isr_encoder_y();

// isr to handle limit switch
void isr_limit_switch_y();

// isr for operating x axis motor velocity
void TC0_Handler(void);

// isr for operating x axis motor acceleration 
void TC1_Handler(void);

// SETUP INTERRUPTS
void setup_encoder_interrupts();

void setup_ls_interrupts();

// MOVEMENT
void axis_trapezoidal_move_rel(Axis *axis, uint32_t vel_max, uint32_t counts_accel, uint32_t counts_const, uint32_t counts_decel, Direction dir);

void axis_trapezoidal_move_tri(Axis *axis, uint32_t vel_max, uint32_t counts_accel, uint32_t counts_decel, Direction dir);

void axis_trapezoidal_move_abs(Axis *axis, uint32_t vel_max, uint32_t counts_accel, uint32_t counts_const, uint32_t counts_decel, Direction dir);

void home_axis(Axis *axis);

#endif