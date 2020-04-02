#include <Arduino.h>
#include "PseudoAxis.h"
#include "PseudoLimitSwitch.h"

// pin assignments
#define MOTOR_AXIS_X 2 // cannot change on uno, only these two pins support hardware interrupts
#define MOTOR_AXIS_Y 3 // cannot change on uno, only these two pins support hardware interrupts

#define MOTOR_DIR_X  4
#define MOTOR_DIR_Y  5

#define ENCODER_OUT_X 6
#define ENCODER_OUT_Y 7

#define LIMIT_SW_OUT_HOME_X 8
#define LIMIT_SW_OUT_FAR_X  9

#define LIMIT_SW_OUT_HOME_Y 10
#define LIMIT_SW_OUT_FAR_Y  11

// steps to encoder
#define STEPS  8
#define COUNTS 5

// gentry length definition
#define AXIS_LENGTH_X 1000000
#define AXIS_LENGTH_Y 1000000

//
#define MOTOR_START_POSITION_X 0
#define MOTOR_START_POSITION_Y 0

PseudoEncoder pseudo_encoder_x;
PseudoEncoder pseudo_encoder_y;
PseudoAxis pseudo_axis_x;
PseudoAxis pseudo_axis_y;

volatile int encoder_count_x = 0;
void isr_motor_pulse_x()
{
  bool pin_status = digitalRead(pseudo_encoder_x.channel_a_out);
  digitalWrite(pseudo_encoder_x.channel_a_out, pin_status ^ HIGH);
  bool direction = digitalRead(pseudo_axis_x.motor_dir_pin);
  
  if (direction) {
    if (pseudo_axis_x.motor_position_current < AXIS_LENGTH_X) {
      pseudo_axis_x.motor_position_current++;
      // check if limit switch was pressed before this move 
      // checks whether the homing routine is being run
      if (!pseudo_axis_x.ls_home.status) { // if !PRESSED where PRESSED = 0
        // sets limit switch to unpressed
        pseudo_axis_x.ls_home.status = UNRESSED;
        // using digital write so it can work on Due ot Uno, execution time ~ 3 microseconds
        digitalWrite(pseudo_axis_x.ls_home.output_pin, UNRESSED);
        
        // resets currrent position to 0
        pseudo_axis_x.motor_position_current = 0;
      }
    }
    else {
      // cant exceed max length - press limit switch
      pseudo_axis_x.ls_far.status = PRESSED;
      
      digitalWrite(pseudo_axis_x.ls_far.output_pin, PRESSED);
    }
  }
  else if (!direction ) {
    if (pseudo_axis_x.motor_position_current > 0) {
      pseudo_axis_x.motor_position_current--;

      if (!pseudo_axis_x.ls_far.status) { // if !PRESSED where PRESSED = 0
        // sets limit switch to unpressed
        pseudo_axis_x.ls_far.status = UNRESSED;
        
        digitalWrite(pseudo_axis_x.ls_far.output_pin, UNRESSED);
        
        // resets currrent position to max length of gentry
        pseudo_axis_x.motor_position_current = AXIS_LENGTH_X;
      }
    }
    else  {
      pseudo_axis_x.ls_home.status = PRESSED;
      digitalWrite(pseudo_axis_x.ls_home.output_pin, PRESSED);
    } 
  }
}

volatile int encoder_count_y = 0;
void isr_motor_pulse_y()
{
  bool pin_status = digitalRead(pseudo_encoder_y.channel_a_out);
  digitalWrite(pseudo_encoder_y.channel_a_out, pin_status ^ HIGH);
  bool direction = digitalRead(pseudo_axis_y.motor_dir_pin);
  
  if (direction) {
    if (pseudo_axis_y.motor_position_current < AXIS_LENGTH_Y) {
      pseudo_axis_y.motor_position_current++;
      // check if limit switch was pressed before this move 
      // checks whether the homing routine is being run
      if (!pseudo_axis_y.ls_home.status) { // if !PRESSED where PRESSED = 0
        // sets limit switch to unpressed
        pseudo_axis_y.ls_home.status = UNRESSED;
        // using digital write so it can work on Due ot Uno, execution time ~ 3 microseconds
        digitalWrite(pseudo_axis_y.ls_home.output_pin, UNRESSED);
        
        // resets currrent position to 0
        pseudo_axis_y.motor_position_current = 0;
      }
    }
    else {
      // cant exceed max length - press limit switch
      pseudo_axis_y.ls_far.status = PRESSED;
      
      digitalWrite(pseudo_axis_y.ls_far.output_pin, PRESSED);
    }
  }
  else if (!direction ) {
    if (pseudo_axis_y.motor_position_current > 0) {
      pseudo_axis_y.motor_position_current--;

      if (!pseudo_axis_y.ls_far.status) { // if !PRESSED where PRESSED = 0
        // sets limit switch to unpressed
        pseudo_axis_y.ls_far.status = UNRESSED;
        
        digitalWrite(pseudo_axis_y.ls_far.output_pin, UNRESSED);
        
        // resets currrent position to max length of gentry
        pseudo_axis_y.motor_position_current = AXIS_LENGTH_Y;
      }
    }
    else  {
      pseudo_axis_y.ls_home.status = PRESSED;
      digitalWrite(pseudo_axis_y.ls_home.output_pin, PRESSED);
    } 
  }
}

void setup() {
  pseudo_encoder_x = {
    .motor_pulse_pin = MOTOR_AXIS_X, 
    .channel_a_out = ENCODER_OUT_X
  };
  pseudo_encoder_y = {
    .motor_pulse_pin = MOTOR_AXIS_Y, 
    .channel_a_out = ENCODER_OUT_Y
  };

  set_up_encoder(&pseudo_encoder_x, &isr_motor_pulse_x);
  set_up_encoder(&pseudo_encoder_y, &isr_motor_pulse_y);

  pseudo_axis_x = {
    .encoder = pseudo_encoder_x, 
    .axis_length_counts = AXIS_LENGTH_X,
    .motor_position_current = MOTOR_START_POSITION_X,
    .motor_dir_pin = MOTOR_DIR_X
  };

  pseudo_axis_y = {
    .encoder = pseudo_encoder_y, 
    .axis_length_counts = AXIS_LENGTH_Y,
    .motor_position_current = MOTOR_START_POSITION_Y,
    .motor_dir_pin = MOTOR_DIR_Y
  };
}

void loop() {
  // put your main code here, to run repeatedly:
}