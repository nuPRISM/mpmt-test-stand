#include "Axis.h"
#include "Timer.h"
#include "Movement.h"

// Arduino Due Timer Interrupt Configurations
/*
 * ISR/IRQ    TC    Channel    Due pins
 * --------------------------------------
 * TC0        TC0   0          2, 13
 * TC1        TC0   1          60, 61
 * TC2        TC0   2          58
 * TC3        TC1   0          none
 * TC4        TC1   1          none
 * TC5        TC1   2          none
 * TC6        TC2   0          4, 5
 * TC7        TC2   1          3, 10
 * TC8        TC2   2          11, 12
*/

/*****************************************************************************/
/*                                  DEFINES                                  */
/*****************************************************************************/

// Macros to generate function names and constants based on IRQ values
#define TC_IRQN_I(_x)          TC##_x##_IRQn
#define TC_IRQN(_x)            TC_IRQN_I(_x)

#define TC_ISR_I(_x)           void TC##_x##_Handler()
#define TC_ISR(_x)             TC_ISR_I(_x)

// IRQs for each axis
#define IRQ_X_AXIS_VELOCITY    3
#define IRQ_X_AXIS_ACCEL       4

#define IRQ_Y_AXIS_VELOCITY    0
#define IRQ_Y_AXIS_ACCEL       1

/*****************************************************************************/
/*                                 TYPEDEFS                                  */
/*****************************************************************************/

// AxisInterrupts
//   - should be configured at declaration time and never modified after
//     (all fields are const)
typedef struct {
    // Timer Interrupts
    Tc * const timer;
    const uint32_t channel_velocity;
    const uint32_t channel_accel;
    const IRQn_Type irq_velocity;
    const IRQn_Type irq_accel;

    // External Interrupt ISRs
    void (*const isr_encoder)(void);
    void (*const isr_limit_switch)(void);
} AxisInterrupts;

// AxisMotion
//   - nothing in this type of struct should change during a movement
//   - it should only be modified at the start of a movement
typedef struct {
    uint32_t accel;
    uint32_t vel_min;
    uint32_t vel_max;
    int32_t counts_accel;
    int32_t counts_hold;
    int32_t counts_decel;
} AxisMotion;

// AxisState
//   - this type of struct will be continuously updated during a movement
typedef struct {
    // General
    volatile bool moving;
    volatile bool homing;
    volatile bool fault;
    // Limit Switches
    volatile LimitSwitchStatus ls_home;
    volatile LimitSwitchStatus ls_far;
    // Motion
    volatile uint32_t velocity;
    volatile VelocitySegment velocity_segment;
    volatile uint32_t encoder_current;
    volatile uint32_t encoder_target;
    volatile Direction dir;
} AxisState;

// Axis
//   - top-level wrapper struct
struct Axis {
    AxisPins pins;
    const AxisInterrupts interrupts;
    AxisMotion motion;
    AxisState state;
};

// External Interrupt Declarations
void isr_encoder_x();
void isr_limit_switch_x();
void isr_encoder_y();
void isr_limit_switch_y();


/*****************************************************************************/
/*                             AXIS DECLARATIONS                             */
/*****************************************************************************/

static Axis axis_x = {
    .pins = {},
    .interrupts = {
        .timer            = TC1,
        .channel_velocity = 0,
        .channel_accel    = 1,
        .irq_velocity     = TC_IRQN(IRQ_X_AXIS_VELOCITY),
        .irq_accel        = TC_IRQN(IRQ_X_AXIS_ACCEL),
        .isr_encoder      = isr_encoder_x,
        .isr_limit_switch = isr_limit_switch_x
    },
    .motion = {},
    .state = {}
};

static Axis axis_y = {
    .pins = {},
    .interrupts = {
        .timer            = TC0,
        .channel_velocity = 0,
        .channel_accel    = 1,
        .irq_velocity     = TC_IRQN(IRQ_Y_AXIS_VELOCITY),
        .irq_accel        = TC_IRQN(IRQ_Y_AXIS_ACCEL),
        .isr_encoder      = isr_encoder_y,
        .isr_limit_switch = isr_limit_switch_y
    },
    .motion = {},
    .state = {}
};

/*****************************************************************************/
/*                              SETUP FUNCTIONS                              */
/*****************************************************************************/

static void setup_pins(Axis *axis)
{
    pinMode(axis->pins.pin_step,    OUTPUT);
    pinMode(axis->pins.pin_dir,     OUTPUT);
    pinMode(axis->pins.pin_enc_a,   INPUT);
    pinMode(axis->pins.pin_enc_b,   INPUT);
    pinMode(axis->pins.pin_ls_home, INPUT_PULLUP);
    pinMode(axis->pins.pin_ls_far,  INPUT_PULLUP);
}

static void setup_interrupts(Axis *axis)
{
    attachInterrupt(digitalPinToInterrupt(axis->pins.pin_enc_a), axis->interrupts.isr_encoder, RISING);
    delay(1000);
    attachInterrupt(digitalPinToInterrupt(axis->pins.pin_ls_home), axis->interrupts.isr_limit_switch, CHANGE);
    delay(1000);
    attachInterrupt(digitalPinToInterrupt(axis->pins.pin_ls_far), axis->interrupts.isr_limit_switch, CHANGE);
}

static void reset_axis(Axis *axis)
{
    axis->state = {};
}

/*****************************************************************************/
/*                             PUBLIC FUNCTIONS                              */
/*****************************************************************************/

void setup_axis(Axis_t *axis, AxisPins pins)
{
    axis->pins = pins;

    setup_pins(axis);
    setup_interrupts(axis);
    
    // reseting axis in case encoder has be triggered or a limit switch due to noise on initilization
    reset_axis(axis);
}

void start_axis(Axis_t *axis)
{
    axis->state.moving = true;
    start_timer(
        axis->interrupts.timer,
        axis->interrupts.channel_velocity,
        axis->interrupts.irq_velocity,
        axis->state.velocity);
    start_timer_accel(
        axis->interrupts.timer,
        axis->interrupts.channel_accel,
        axis->interrupts.irq_accel,
        axis->motion.accel);
}

void stop_axis(Axis_t *axis)
{
    NVIC_DisableIRQ(axis->interrupts.irq_accel);
    NVIC_DisableIRQ(axis->interrupts.irq_velocity);
    axis->state.moving = false;
}

/*****************************************************************************/
/*                            COMMON ISR HANDLERS                            */
/*****************************************************************************/

static inline void handle_isr_encoder(Axis *axis)  __attribute__((always_inline));
static inline void handle_isr_encoder(Axis *axis)
{
    if (axis->state.dir == DIR_POSITIVE) axis->state.encoder_current++;
    else if (axis->state.dir == DIR_NEGATIVE) axis->state.encoder_current--;
}

static inline void handle_isr_limit_switch(Axis *axis)  __attribute__((always_inline));
static inline void handle_isr_limit_switch(Axis *axis)
{
    // TODO
}

static inline void handle_velocity_isr(Axis *axis) __attribute__((always_inline));
static inline void handle_velocity_isr(Axis *axis)
{
    axis->pins.pin_step_pio_bank->PIO_ODSR ^= axis->pins.pin_step_pio_mask;
    reset_timer(
        axis->interrupts.timer,
        axis->interrupts.channel_velocity,
        axis->interrupts.irq_velocity,
        axis->state.velocity);
}

static inline void handle_accel_isr(Axis *axis) __attribute__((always_inline));
static inline void handle_accel_isr(Axis *axis)
{
    switch (axis->state.velocity_segment) {
        case VEL_SEG_ACCELERATE:
        {
            if (((axis->state.encoder_current < axis->state.encoder_target) != axis->state.dir) && (axis->state.velocity < axis->motion.vel_max)) {
                axis->state.velocity++;
            }
            else {
                axis->state.velocity_segment = VEL_SEG_HOLD;
                int delta = axis->state.encoder_target - axis->state.encoder_current;
                axis->state.encoder_target = axis->state.encoder_current + axis->motion.counts_hold + delta;
            }
            break;
        }

        case VEL_SEG_HOLD:
        {
            if ((axis->state.encoder_current < axis->state.encoder_target) == axis->state.dir) {
                axis->state.velocity_segment = VEL_SEG_DECELERATE;
                int delta = axis->state.encoder_target - axis->state.encoder_current;
                axis->state.encoder_target = axis->state.encoder_current + axis->motion.counts_decel + delta;
            }
            break;
        }

        case VEL_SEG_DECELERATE:
        {
            if (((axis->state.encoder_current < axis->state.encoder_target) != axis->state.dir) && (axis->state.velocity > axis->motion.vel_min)) {
                axis->state.velocity--;
            }
            else {
                stop_axis(axis);
            }
            break;
        }
    }
}

/*****************************************************************************/
/*                                X-AXIS ISRS                                */
/*****************************************************************************/

void isr_encoder_x()
{
    handle_isr_encoder(&axis_x);
}

// isr to handle limit switch
void isr_limit_switch_x()
{
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();
    // If interrupts come faster than 10ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 100) {
        stop_axis(&axis_x);
        LimitSwitchStatus status = (LimitSwitchStatus)digitalRead(LIMIT_SW_HOME_PIN_X);
        if (axis_x.homing) {
            if (status == RELEASED) {
                axis_x.encoder.current = 0;
                axis_x.homing = false;
            }
            else if (status == PRESSED) {
                home_axis(&axis_x);
            }
        }
        else {
            axis_x.fault = true;
        }
    }
    last_interrupt_time = interrupt_time;
}

TC_ISR(IRQ_X_AXIS_VELOCITY)
{
    // Acknowledge interrupt
    TC_GetStatus(axis_x.interrupts.timer, axis_x.interrupts.channel_velocity);

    handle_velocity_isr(&axis_x);
}

TC_ISR(IRQ_X_AXIS_ACCEL)
{
    // Acknowledge interrupt
    TC_GetStatus(axis_x.interrupts.timer, axis_x.interrupts.channel_accel);

    handle_accel_isr(&axis_x);
}

/*****************************************************************************/
/*                                Y-AXIS ISRS                                */
/*****************************************************************************/

void isr_encoder_y()
{
    handle_isr_encoder(&axis_y);
}

void isr_limit_switch_y()
{   
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();
    // If interrupts come faster than 10ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 50) {
        stop_axis(&axis_x);

        LimitSwitchStatus status = (LimitSwitchStatus)digitalRead(LIMIT_SW_HOME_PIN_Y);
        if (axis_y.homing) {
            if (status == RELEASED) {
                axis_y.encoder.current = 0;
                axis_y.homing = false;
            }
            else if (status == PRESSED) {
                home_axis(&axis_y);
            }
        }
        else {
            axis_y.fault = true;
        }
    }
    last_interrupt_time = interrupt_time;
}

TC_ISR(IRQ_Y_AXIS_VELOCITY)
{
    // Acknowledge interrupt
    TC_GetStatus(axis_y.interrupts.timer, axis_y.interrupts.channel_velocity);

    handle_velocity_isr(&axis_y);
}

TC_ISR(IRQ_Y_AXIS_ACCEL)
{
    // Acknowledge interrupt
    TC_GetStatus(axis_y.interrupts.timer, axis_y.interrupts.channel_accel);

    handle_accel_isr(&axis_y);
}