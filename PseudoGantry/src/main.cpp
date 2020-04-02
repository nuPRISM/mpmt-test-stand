#include <Arduino.h>
#include <math.h>
#include "PseudoAxis.h"

// pin assignments
#define MOTOR_AXIS_X 2 // cannot change on uno, only these two pins support hardware interrupts
#define MOTOR_AXIS_Y 3 // cannot change on uno, only these two pins support hardware interrupts

#define MOTOR_DIR_X 4
#define MOTOR_DIR_Y 5

#define ENCODER_OUT_X 6
#define ENCODER_OUT_Y 7

#define LIMIT_SW_OUT_HOME_X 8
#define LIMIT_SW_OUT_FAR_X 9

#define LIMIT_SW_OUT_HOME_Y 10
#define LIMIT_SW_OUT_FAR_Y 11

// steps to counts constants
// only supports a mode where number of steps > number of counts
#define STEPS 8
#define COUNTS 5

// gentry length definition in counts
#define AXIS_LENGTH_COUNTS_X 1000000
#define AXIS_LENGTH_COUNTS_Y 1000000

//
#define MOTOR_START_POSITION_X 0
#define MOTOR_START_POSITION_Y 0

PseudoEncoder pseudo_encoder_x;
PseudoEncoder pseudo_encoder_y;
PseudoAxis pseudo_axis_x;
PseudoAxis pseudo_axis_y;

void isr_motor_pulse_x()
{
  isr_motor_pulse(&pseudo_axis_x);
}

void isr_motor_pulse_y()
{
  isr_motor_pulse(&pseudo_axis_y);
}

void setup()
{
  pseudo_encoder_x = {
      .motor_pulse_pin = MOTOR_AXIS_X,
      .channel_a_out = ENCODER_OUT_X};
  pseudo_encoder_y = {
      .motor_pulse_pin = MOTOR_AXIS_Y,
      .channel_a_out = ENCODER_OUT_Y};

  set_up_encoder(&pseudo_encoder_x, &isr_motor_pulse_x);
  set_up_encoder(&pseudo_encoder_y, &isr_motor_pulse_y);

  pseudo_axis_x = {
      .encoder = pseudo_encoder_x,
      .axis_length_counts = AXIS_LENGTH_COUNTS_X,
      .motor_position_current = MOTOR_START_POSITION_X,
      .motor_position_default = MOTOR_START_POSITION_X,
      .motor_dir_pin = MOTOR_DIR_X,
      .skip_counter = 0,
      .steps_for_ratio = STEPS,
      .counts_for_ratio = COUNTS,
      .changes_to_skip = 2 * (STEPS - COUNTS),
  };

  pseudo_axis_y = {
      .encoder = pseudo_encoder_y,
      .axis_length_counts = AXIS_LENGTH_COUNTS_Y,
      .motor_position_current = MOTOR_START_POSITION_Y,
      .motor_position_default = MOTOR_START_POSITION_Y,
      .motor_dir_pin = MOTOR_DIR_Y,
      .skip_counter = 0,
      .steps_for_ratio = STEPS,
      .counts_for_ratio = COUNTS,
      .changes_to_skip = 2 * (STEPS - COUNTS),
  };

  delay(1000);

  reset_pseudo_axis(&pseudo_axis_x);
  reset_pseudo_axis(&pseudo_axis_y);
}

void loop()
{
  // put your main code here, to run repeatedly:
}