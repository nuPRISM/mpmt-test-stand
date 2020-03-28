#include <Arduino.h>
#include "PseudoAxis.h"

#define MOTOR_AXIS_X 2 // cannot change on uno, only these two pins support hardware interrupts
#define MOTOR_AXIS_Y 3 // cannot change on uno, only these two pins support hardware interrupts
#define ENCODER_X_OUT 4
#define ENCODER_Y_OUT 5

#define AXIS_LENGTH_X 1000000
#define AXIS_LENGTH_Y 1000000 

PseudoEncoder pseudo_encoder_x;
PseudoEncoder pseudo_encoder_y;
PseudoAxis pseudo_axis_x;
PseudoAxis pseudo_axis_y;

void isr_encoder_x()
{
  pseudo_encoder_x.channel_a_out ^= HIGH;
}
void isr_encoder_y()
{
  pseudo_encoder_y.channel_a_out ^= HIGH;
}

void setup() {
  pseudo_encoder_x = {MOTOR_AXIS_Y, ENCODER_X_OUT};
  pseudo_encoder_y = {MOTOR_AXIS_Y, ENCODER_Y_OUT};
  set_up_encoder(&pseudo_encoder_x, &isr_encoder_x);
  set_up_encoder(&pseudo_encoder_y, &isr_encoder_y);

  pseudo_axis_x = {pseudo_encoder_x, AXIS_LENGTH_X, 0};
  pseudo_axis_y = {pseudo_encoder_y, AXIS_LENGTH_Y, 0};
}

void loop() {
  // put your main code here, to run repeatedly:
}