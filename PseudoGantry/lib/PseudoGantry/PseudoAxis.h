#ifndef PSEUDO_AXIS_H
#define PSEUDO_AXIS_H

#include "Arduino.h"

typedef struct PseudoEncoder
{
    uint32_t motor_pulse_pin;
    volatile uint32_t channel_a_out;
} PseudoEncoder;

typedef struct PseudoLimitSwitch
{
    uint32_t output_pin;
    uint32_t status;
} PseudoLimitSwitch;

typedef enum {
    PRESSED = LOW,
    UNRESSED = HIGH
} LimitSwitchStatus;

typedef struct PseudoAxis
{   
    String axis_name;
    PseudoEncoder encoder;
    uint32_t axis_length_counts;
    uint32_t motor_position_current;
    uint32_t motor_position_default;
    uint32_t motor_dir_pin;
    volatile int skip_counter;
    int steps_for_ratio;                // values necesarry to determine steps vs counts ration
    int counts_for_ratio;               // values necesarry to determine steps vs counts ration
    int changes_to_skip;
    PseudoLimitSwitch ls_home;
    PseudoLimitSwitch ls_far;
} PseudoAxis;

void reset_pseudo_axis(PseudoAxis *pseudo_axis);
void set_up_encoder(PseudoEncoder *encoder, void (*isr_motor_pulse)(void));
void isr_motor_pulse(PseudoAxis *pseudo_axis);
bool toggle_encoder_output(PseudoAxis *pseudo_axis);
void dump_data(PseudoAxis *pseudo_axis);

#endif // PSEUDO_AXIS_H