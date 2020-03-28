/* **************************** Local Includes ***************************** */
#include "Axis.h"
#include "Movement.h"
#include "Timer.h"

/* ************************ Shared Project Includes ************************ */
#include "Debug.h"

/*****************************************************************************/
/*                                  DEFINES                                  */
/*****************************************************************************/

#define TC_IRQN_I(_x)          TC##_x##_IRQn
/** Generates the IRQn value corresponding to a TC IRQ number */
#define TC_IRQN(_x)            TC_IRQN_I(_x)

#define TC_ISR_I(_x)           void TC##_x##_Handler()
/** Generates the ISR function name corresponding to a TC IRQ number */
#define TC_ISR(_x)             TC_ISR_I(_x)

/** TC IRQ number for x-axis velocity timer */
#define IRQ_X_AXIS_VELOCITY    3
/** TC IRQ number for x-axis acceleration timer */
#define IRQ_X_AXIS_ACCEL       4

/** TC IRQ number for y-axis velocity timer */
#define IRQ_Y_AXIS_VELOCITY    0
/** TC IRQ number for y-axis acceleration timer */
#define IRQ_Y_AXIS_ACCEL       1

/** Maximum allowed velocity for an axis */
#define VEL_MAX                25000

/*****************************************************************************/
/*                                 TYPEDEFS                                  */
/*****************************************************************************/

typedef enum {
    VEL_SEG_ACCELERATE,
    VEL_SEG_HOLD,
    VEL_SEG_DECELERATE
} VelSeg;

/**
 * @struct AxisInterrupts
 * 
 * @brief Specifies all of the interrupts and ISRs used by the axis
 */
typedef struct {
    Tc * const timer;                 //!< Timer counter
    const uint32_t channel_accel;     //!< Timer counter channel for the acceleration interrupt
    const IRQn_Type irq_accel;        //!< IRQ number for the acceleration interrupt

    void (*const isr_encoder)(void);  //!< ISR for the encoder interrupt
    void (*const isr_ls_home)(void);  //!< ISR for the home limit switch interrupt
    void (*const isr_ls_far)(void);   //!< ISR for the far limit switch interrupt
} AxisInterrupts;

/**
 * @struct AxisState
 * 
 * @brief Specifies the current state of an axis
 */
typedef struct {
    volatile bool moving;              //!< true if the axis is currently moving

    volatile bool ls_home;             //!< true if the home limit switch is currently pressed
    volatile bool ls_far;              //!< true if the far limit switch is currently pressed

    volatile uint32_t velocity;        //!< current velocity of the axis
    volatile VelSeg velocity_segment;  //!< current velcoity segment of the axis
    volatile uint32_t encoder_current; //!< current position of the axis in encoder counts
    volatile uint32_t encoder_target;  //!< position at which the next segment transition will occur
    volatile Direction dir;            //!< current direction of motion of the axis
} AxisState;

/**
 * @struct Axis
 * 
 * @brief Top-level struct for encapsulating all elements of an axis
 */
struct Axis {
    AxisIO io;                   //!< All the I/O pins used by an axis
    const AxisInterrupts interrupts; //!<
    AxisMotion motion;
    AxisState state;
};

// ISR Declarations
void isr_encoder_x();
void isr_ls_home_x();
void isr_ls_far_x();
void isr_encoder_y();
void isr_ls_home_y();
void isr_ls_far_y();

/*****************************************************************************/
/*                             AXIS DECLARATIONS                             */
/*****************************************************************************/

static Axis axis_x = {
    .io = {},
    .interrupts = {
        .timer            = TC1,
        .channel_accel    = 1,
        .irq_accel        = TC_IRQN(IRQ_X_AXIS_ACCEL),
        .isr_encoder      = isr_encoder_x,
        .isr_ls_home      = isr_ls_home_x,
        .isr_ls_far       = isr_ls_far_x
    },
    .motion = {},
    .state = {}
};

static Axis axis_y = {
    .io = {},
    .interrupts = {
        .timer            = TC0,
        .channel_accel    = 1,
        .irq_accel        = TC_IRQN(IRQ_Y_AXIS_ACCEL),
        .isr_encoder      = isr_encoder_y,
        .isr_ls_home      = isr_ls_home_y,
        .isr_ls_far       = isr_ls_far_y
    },
    .motion = {},
    .state = {}
};

/*****************************************************************************/
/*                             PRIVATE FUNCTIONS                             */
/*****************************************************************************/

/**
 * @brief Configure the pin modes for all of the axis pins
 * 
 * @param axis Pointer to the Axis to configure
 */
static void setup_pins(Axis *axis)
{
    configure_pwm_timer(
        axis->io.tc_step,
        axis->io.tc_step_channel,
        axis->io.tc_step_irq,
        axis->io.pio_step_bank,
        axis->io.pio_step_periph,
        axis->io.pio_step_pin_mask);

    pinMode(axis->io.pin_dir,     OUTPUT);
    pinMode(axis->io.pin_enc_a,   INPUT);
    pinMode(axis->io.pin_enc_b,   INPUT);
    pinMode(axis->io.pin_ls_home, INPUT_PULLUP);
    pinMode(axis->io.pin_ls_far,  INPUT_PULLUP);
}

/**
 * @brief Attaches interrupts to the encoder and limit switch pins for the axis
 * 
 * @param axis Pointer to the Axis to use
 */
static void setup_interrupts(Axis *axis)
{
    configure_timer_interrupt(
        axis->interrupts.timer,
        axis->interrupts.channel_accel,
        axis->interrupts.irq_accel
    );

    attachInterrupt(digitalPinToInterrupt(axis->io.pin_enc_a),
                    axis->interrupts.isr_encoder,
                    RISING);
    delay(1000);
    attachInterrupt(digitalPinToInterrupt(axis->io.pin_ls_home),
                    axis->interrupts.isr_ls_home,
                    CHANGE);
    delay(1000);
    attachInterrupt(digitalPinToInterrupt(axis->io.pin_ls_far),
                    axis->interrupts.isr_ls_far,
                    CHANGE);
}

static void reset_axis(Axis *axis)
{
    axis->state = (AxisState){ 0 };
}

static void setup_axis(Axis *axis, const AxisIO *io)
{
    axis->io = (*io);

    setup_pins(axis);
    setup_interrupts(axis);

    // reset in case encoder was accidentally triggered by noise
    // on initialization
    reset_axis(axis);
}

AxisResult start_axis(Axis *axis, AxisMotion *motion)
{
    // Reject if the counts don't match the dir
    if (motion->dir == DIR_POSITIVE && (motion->counts_accel < 0 || motion->counts_hold < 0 || motion->counts_decel < 0)) return AXIS_ERR_DIR_MISMATCH;
    if (motion->dir == DIR_NEGATIVE && (motion->counts_accel > 0 || motion->counts_hold > 0 || motion->counts_decel > 0)) return AXIS_ERR_DIR_MISMATCH;

    uint32_t total_counts_abs = abs(motion->counts_accel + motion->counts_hold + motion->counts_decel);
    // Reject a zero-distance motion
    if (total_counts_abs == 0) return AXIS_ERR_ZERO_DIST;

    // Reject if we're already moving - must call stop_axis first
    if (axis->state.moving) return AXIS_ERR_ALREADY_MOVING;

    // Reject if we've hit the far limit switch and are trying to move forward
    if (motion->dir == DIR_POSITIVE && axis->state.ls_far) return AXIS_ERR_LS_FAR;
    // Reject if we've hit the home limit switch and are trying to move backward
    if (motion->dir == DIR_NEGATIVE && axis->state.ls_home) return AXIS_ERR_LS_HOME;

    // Reject if we're trying to move too far backwards
    if (total_counts_abs > axis->state.encoder_current && motion->dir == DIR_NEGATIVE) return AXIS_ERR_TOO_FAR_BACKWARD;
    // TODO define far limits and reject if trying to move too far forwards

    // Configure state
    axis->state.moving = true;
    axis->state.velocity = motion->vel_start;
    axis->state.velocity_segment = VEL_SEG_ACCELERATE;
    axis->state.encoder_target = axis->state.encoder_current + motion->counts_accel;
    axis->state.dir = motion->dir;

    // Save motion spec
    axis->motion = (*motion);

    // Set direction pin
    digitalWrite(axis->io.pin_dir, (axis->state.dir == DIR_POSITIVE ? LOW : HIGH));

    // Start velocity PWM timer
    reset_pwm_timer(axis->io.tc_step, axis->io.tc_step_channel, axis->state.velocity);

    // Start acceleration timer interrupt
    reset_timer_interrupt(
        axis->interrupts.timer,
        axis->interrupts.channel_accel,
        axis->interrupts.irq_accel,
        axis->motion.accel);

    return AXIS_OK;
}

static inline void stop_axis(Axis *axis)  __attribute__((always_inline));
static inline void stop_axis(Axis *axis)
{
    stop_pwm_timer(axis->io.tc_step, axis->io.tc_step_channel);
    stop_timer_interrupt(axis->interrupts.timer, axis->interrupts.channel_accel, axis->interrupts.irq_accel);

    axis->state.moving = false;
    axis->state.velocity = 0;
    axis->state.encoder_target = axis->state.encoder_current;
}

static Axis *get_axis(AxisId axis_id)
{
    if (axis_id == AXIS_X) return &axis_x;
    else if (axis_id == AXIS_Y) return &axis_y;
    return nullptr;
}

/*****************************************************************************/
/*                            COMMON ISR HANDLERS                            */
/*****************************************************************************/

// Encoder
static inline void handle_isr_encoder(Axis *axis)  __attribute__((always_inline));
static inline void handle_isr_encoder(Axis *axis)
{
    if (axis->state.dir == DIR_POSITIVE) axis->state.encoder_current++;
    else if (axis->state.dir == DIR_NEGATIVE) axis->state.encoder_current--;
}

// Limit Switches
static inline void handle_isr_ls_home(Axis *axis)  __attribute__((always_inline));
static inline void handle_isr_ls_home(Axis *axis)
{
    stop_axis(axis);
    axis->state.ls_home = digitalRead(axis->io.pin_ls_home);
}

static inline void handle_isr_ls_far(Axis *axis)  __attribute__((always_inline));
static inline void handle_isr_ls_far(Axis *axis)
{
    stop_axis(axis);
    axis->state.ls_far = digitalRead(axis->io.pin_ls_far);
}

static inline void handle_accel_isr(Axis *axis) __attribute__((always_inline));
static inline void handle_accel_isr(Axis *axis)
{
    switch (axis->state.velocity_segment) {
        case VEL_SEG_ACCELERATE:
        {
            // Increment the velocity until the target number of counts or we reach holding velocity
            if (((axis->state.encoder_current < axis->state.encoder_target) != axis->state.dir)
                && (axis->state.velocity < VEL_MAX)) {
                axis->state.velocity++;
                reset_pwm_timer(axis->io.tc_step, axis->io.tc_step_channel, axis->state.velocity);
            }
            else {
                axis->state.velocity_segment = VEL_SEG_HOLD;

                // Set new target encoder count
                int32_t error_counts = axis->state.encoder_target - axis->state.encoder_current;
                axis->state.encoder_target = axis->state.encoder_current
                                             + axis->motion.counts_hold
                                             + error_counts;
            }
            break;
        }

        case VEL_SEG_HOLD:
        {
            // Do nothing (i.e. keep the same velocity) until we reach the target encoder count
            if ((axis->state.encoder_current < axis->state.encoder_target) == axis->state.dir) {
                // Move on to deceleration
                axis->state.velocity_segment = VEL_SEG_DECELERATE;

                // Set new target encoder count
                int32_t error_counts = axis->state.encoder_target - axis->state.encoder_current;
                axis->state.encoder_target = axis->state.encoder_current
                                             + axis->motion.counts_decel
                                             + error_counts;
            }
            break;
        }

        case VEL_SEG_DECELERATE:
        {
            // Decrement velocity until the target number of counts or we reach the starting velocity again
            if (((axis->state.encoder_current < axis->state.encoder_target) != axis->state.dir)
                && (axis->state.velocity > axis->motion.vel_start)) {
                axis->state.velocity--;
                reset_pwm_timer(axis->io.tc_step, axis->io.tc_step_channel, axis->state.velocity);
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

void isr_ls_home_x()
{
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();
    // If interrupts come faster than 10ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 100) {
        handle_isr_ls_home(&axis_x);
    }
    last_interrupt_time = interrupt_time;
}

void isr_ls_far_x()
{
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();
    // If interrupts come faster than 10ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 100) {
        handle_isr_ls_far(&axis_x);
    }
    last_interrupt_time = interrupt_time;
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

void isr_ls_home_y()
{
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();
    // If interrupts come faster than 10ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 100) {
        handle_isr_ls_home(&axis_y);
    }
    last_interrupt_time = interrupt_time;
}

void isr_ls_far_y()
{
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();
    // If interrupts come faster than 10ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 100) {
        handle_isr_ls_far(&axis_y);
    }
    last_interrupt_time = interrupt_time;
}

TC_ISR(IRQ_Y_AXIS_ACCEL)
{
    // Acknowledge interrupt
    TC_GetStatus(axis_y.interrupts.timer, axis_y.interrupts.channel_accel);

    handle_accel_isr(&axis_y);
}

/*****************************************************************************/
/*                             PUBLIC FUNCTIONS                              */
/*****************************************************************************/

void axis_setup(AxisId axis_id, const AxisIO *io)
{
    setup_axis(get_axis(axis_id), io);
}

AxisResult axis_start(AxisId axis_id, AxisMotion *motion)
{
    return start_axis(get_axis(axis_id), motion);
}

void axis_stop(AxisId axis_id)
{
    stop_axis(get_axis(axis_id));
}

uint32_t axis_get_position(AxisId axis_id)
{
    return get_axis(axis_id)->state.encoder_current;
}

bool axis_moving(AxisId axis_id)
{
    return get_axis(axis_id)->state.moving;
}

void axis_dump_state(AxisId axis_id)
{
    Axis *axis = get_axis(axis_id);
    DEBUG_PRINTLN("----------------------------------------");
    DEBUG_PRINT_VAL("AXIS            ", (axis_id == AXIS_X ? "X" : "Y"));
    DEBUG_PRINT_VAL("moving          ", axis->state.moving);
    DEBUG_PRINT_VAL("ls_home         ", axis->state.ls_home);
    DEBUG_PRINT_VAL("ls_far          ", axis->state.ls_far);
    DEBUG_PRINT_VAL("velocity        ", axis->state.velocity);
    DEBUG_PRINT_VAL("velocity_segment", axis->state.velocity_segment);
    DEBUG_PRINT_VAL("encoder_current ", axis->state.encoder_current);
    DEBUG_PRINT_VAL("encoder_target  ", axis->state.encoder_target);
    DEBUG_PRINT_VAL("dir             ", axis->state.dir);
    DEBUG_PRINTLN("----------------------------------------");
}