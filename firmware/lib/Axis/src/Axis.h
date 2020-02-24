#ifndef AXIS_H
#define AXIS_H

#include "Debug.h"
#include "Kinematics.h"
// -------------------------------- X AXIS SETUP -------------------------------- //
#define ACCEL_X                 100
#define VEL_MIN_X               100
#define VEL_MAX_X               1000
#define STEP_PIN_X              5 // need to know the register for fast operation pin 5 - PC25 
#define DIR_PIN_X               6 // need to know the register for fast operation pin 6 - PC24
#define ENCODER_PIN_A_X         7
#define ENCODER_PIN_B_X         8
#define LIMIT_SW_HOME_PIN_X     9
#define LIMIT_SW_FAR_PIN_X      10

Axis setup_axis_x();

extern Axis axis_x;
// isr to handle encoder of x axis
void isr_encoder_x();

// isr to handle limit switch
void isr_limit_switch_x();

// isr for operating x axis motor velocity
void TC3_Handler(void);

// isr for operating x axis motor acceleration 
void TC4_Handler(void);


// -------------------------------- Y AXIS SETUP -------------------------------- //
#define ACCEL_Y                 100
#define VEL_MIN_Y               100
#define VEL_MAX_Y               1000
#define STEP_PIN_Y              22 // need to know the register for fast operation pin 5 - PB26 
#define DIR_PIN_Y               23 // need to know the register for fast operation pin 6 - PA14
#define ENCODER_PIN_A_Y         24
#define ENCODER_PIN_B_Y         25
#define LIMIT_SW_HOME_PIN_Y     26
#define LIMIT_SW_FAR_PIN_Y      27

Axis setup_axis_y();

extern Axis axis_x;
// isr to handle encoder of x axis
void isr_encoder_y();

// isr to handle limit switch
void isr_limit_switch_y();

// isr for operating x axis motor velocity
void TC0_Handler(void);

// isr for operating x axis motor acceleration 
void TC1_Handler(void);

#endif