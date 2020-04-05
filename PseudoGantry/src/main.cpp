#include <Arduino.h>
#include "PseudoAxis.h"

// pins and constants for testing pseudo gantry
#define PWM_FREQ        245.
#define PWM_PIN         11.
#define STEPS_TO_TAKE   800.
float delay_millis = ((1000./PWM_FREQ/2.)*STEPS_TO_TAKE);


#define BAUDRATE 250000
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

// gantry length definition in counts
#define AXIS_LENGTH_COUNTS_X 2000
#define AXIS_LENGTH_COUNTS_Y 2000

//
#define MOTOR_START_POSITION_X 0
#define MOTOR_START_POSITION_Y 0

PseudoEncoder pseudo_encoder_x;
PseudoEncoder pseudo_encoder_y;
PseudoAxis pseudo_axis_x;
PseudoAxis pseudo_axis_y;

void isr_motor_pulse_x()
{
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = micros();
    if (interrupt_time - last_interrupt_time > 100) {
          isr_motor_pulse(&pseudo_axis_x);
    }
    last_interrupt_time = interrupt_time;
}

void isr_motor_pulse_y()
{
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = micros();
    if (interrupt_time - last_interrupt_time > 100) {
          isr_motor_pulse(&pseudo_axis_y);
    }
    last_interrupt_time = interrupt_time;
}

void setup()
{
  pseudo_encoder_x = {
      .motor_pulse_pin = MOTOR_AXIS_X,
      .channel_a_pin = ENCODER_OUT_X
      };
  pseudo_encoder_y = {
      .motor_pulse_pin = MOTOR_AXIS_Y,
      .channel_a_pin = ENCODER_OUT_Y
      };

  set_up_encoder(&pseudo_encoder_x, &isr_motor_pulse_x);
  set_up_encoder(&pseudo_encoder_y, &isr_motor_pulse_y);

  pseudo_axis_x = {
      .axis_name = "X",
      .encoder = pseudo_encoder_x,
      .axis_length_counts = AXIS_LENGTH_COUNTS_X,
      .motor_position_current = MOTOR_START_POSITION_X,
      .motor_position_default = MOTOR_START_POSITION_X,
      .motor_dir_pin = MOTOR_DIR_X,
      .skip_counter = 0,
      .steps_for_ratio = STEPS,
      .counts_for_ratio = COUNTS,
      .changes_to_skip = 2 * (STEPS - COUNTS),
      .ls_home = {
        .output_pin = LIMIT_SW_OUT_HOME_X,
        .status = UNPRESSED
      },
      .ls_far = {
        .output_pin = LIMIT_SW_OUT_FAR_X,
        .status = UNPRESSED
      }
      };

  pseudo_axis_y = {
      .axis_name = "Y",
      .encoder = pseudo_encoder_y,
      .axis_length_counts = AXIS_LENGTH_COUNTS_Y,
      .motor_position_current = MOTOR_START_POSITION_Y,
      .motor_position_default = MOTOR_START_POSITION_Y,
      .motor_dir_pin = MOTOR_DIR_Y,
      .skip_counter = 0,
      .steps_for_ratio = STEPS,
      .counts_for_ratio = COUNTS,
      .changes_to_skip = 2 * (STEPS - COUNTS),
      .ls_home = {
        .output_pin = LIMIT_SW_OUT_HOME_Y,
        .status = UNPRESSED
      },
      .ls_far = {
        .output_pin = LIMIT_SW_OUT_FAR_Y,
        .status = UNPRESSED
      }
      };

  delay(1000);

  reset_pseudo_axis(&pseudo_axis_x);
  reset_pseudo_axis(&pseudo_axis_y);

  // setup serial for monitoring
  Serial.begin(BAUDRATE);

  // pretend to be a motor
  // pinMode(MOTOR_DIR_X, INPUT_PULLUP);
  // pinMode(PWM_PIN, OUTPUT);
}

void loop()
{
  // put your main code here, to run repeatedly:
  dump_data(&pseudo_axis_x);
  dump_data(&pseudo_axis_y);
  delay(1000);
  // analogWrite(PWM_PIN, 122);
  // delay(delay_millis);
  // analogWrite(PWM_PIN, 0);
}