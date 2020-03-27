#ifndef AXIS_H
#define AXIS_H

#include "Kinematics.h"
#include "LimitSwitch.h"
#include "Encoder.h"

#include "shared_defs.h"

#define VELOCITY_HOMING 10000

typedef enum {
    VEL_SEG_ACCELERATE,
    VEL_SEG_HOLD,
    VEL_SEG_DECELERATE
} VelocitySegment;

typedef enum {
    PRESSED = LOW,
    RELEASED = HIGH
} LimitSwitchStatus;

// AxisPins
//   - should be configured at setup time and never modified after
typedef struct {
    uint8_t pin_step;
    Pio *pin_step_pio_bank;
    RwReg pin_step_pio_mask;
    uint8_t pin_dir;
    uint8_t pin_enc_a;
    uint8_t pin_enc_b;
    uint8_t pin_ls_home;
    uint8_t pin_ls_far;
} AxisPins;

typedef struct Axis Axis_t;

// setup functions
void setup_axis(Axis_t *axis, AxisPins pins);

 // start/stop functions
void start_axis(Axis_t *axis);
void stop_axis(Axis_t *axis);

extern Axis_t axis_x;
extern Axis_t axis_y;

#endif // AXIS_H