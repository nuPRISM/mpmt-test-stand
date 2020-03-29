/* **************************** Local Includes ***************************** */
#include "Axis.h"
#include "Movement.h"
#include "Timer.h"

/* ************************ Shared Project Includes ************************ */
#include "Debug.h"

/*****************************************************************************/
/*                                  DEFINES                                  */
/*****************************************************************************/

/** TC IRQ number for x-axis velocity timer */
#define IRQ_X_AXIS_VELOCITY    3
/** TC IRQ number for x-axis acceleration timer */
#define IRQ_X_AXIS_ACCEL       4

/** TC IRQ number for y-axis velocity timer */
#define IRQ_Y_AXIS_VELOCITY    0
/** TC IRQ number for y-axis acceleration timer */
#define IRQ_Y_AXIS_ACCEL       1

/** Frequency of the slow clock */
#define SCLK_FREQ              32768

/** Minimum pulse duration to pass debouncing */
#define DEBOUNCE_FILTER_MS     50

/** Macro to determine if we've reached a target encoder count based on direction */
#define REACHED_TARGET(_dir, _cur, _tar)             \
    (((_dir) == DIR_POSITIVE && (_cur) >= (_tar)) || \
    ((_dir) == DIR_NEGATIVE && (_cur) <= (_tar)))

/** Maximum allowed velocity for an axis */
#define VEL_MAX                25000

/*****************************************************************************/
/*                                 TYPEDEFS                                  */
/*****************************************************************************/

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
 * @struct Axis
 * 
 * @brief Top-level struct for encapsulating all elements of an axis
 */
struct Axis {
    AxisIO io;                         //!< I/O pins and peripherals
    const AxisInterrupts interrupts;   //!< Interrupt configurations
    AxisMotion motion;                 //!< Profile for the current motion
    AxisState state;                   //!< Current state of the axis
};

/*****************************************************************************/
/*                             AXIS DECLARATIONS                             */
/*****************************************************************************/

/* ******************************** X AXIS ********************************* */

void isr_encoder_x();
void isr_ls_home_x();
void isr_ls_far_x();

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

/* ******************************** Y AXIS ********************************* */

void isr_encoder_y();
void isr_ls_home_y();
void isr_ls_far_y();

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

// Forward Declarations
static void reset_axis(Axis *axis);

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
        axis->io.pio_step,
        axis->io.pio_step_periph,
        axis->io.pio_step_pin_mask);

    pinMode(axis->io.pin_dir,     OUTPUT);
    pinMode(axis->io.pin_enc_a,   INPUT);
    pinMode(axis->io.pin_enc_b,   INPUT);
    pinMode(axis->io.pin_ls_home, INPUT_PULLUP);
    pinMode(axis->io.pin_ls_far,  INPUT_PULLUP);
}

/**
 * @brief Enables the hardware debouncing filter on the specified pin
 * 
 * @param pin       The pin to be deboucned
 * @param filter_ms The minimum pulse duration to pass debouncing
 */
static void setup_debouncing(uint32_t pin, uint32_t filter_ms)
{
    const PinDescription *pin_desc = &g_APinDescription[pin];

    // Enable input filtering
    pin_desc->pPort->PIO_IFER |= pin_desc->ulPin;

    // Enable debouncing filter (filter pulses with a duration < Tdiv_slclk/2)
    pin_desc->pPort->PIO_DIFSR |= pin_desc->ulPin;

    // Set DIV: Tdiv_slclk = 2*(DIV+1)*Tslow_clock
    pin_desc->pPort->PIO_SCDR = (SCLK_FREQ * filter_ms / 1000) - 1;
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

    // Debounce and attach interrupt to HOME limit switch
    setup_debouncing(axis->io.pin_ls_home, DEBOUNCE_FILTER_MS);
    attachInterrupt(digitalPinToInterrupt(axis->io.pin_ls_home),
                    axis->interrupts.isr_ls_home,
                    CHANGE);

    // Debounce and attach interrupt to FAR limit switch
    setup_debouncing(axis->io.pin_ls_far, DEBOUNCE_FILTER_MS);
    attachInterrupt(digitalPinToInterrupt(axis->io.pin_ls_far),
                    axis->interrupts.isr_ls_far,
                    CHANGE);
}

/**
 * @brief Initializes all components of an axis
 * 
 * Sets pin modes, configures and enables interrupts and resets to a default state.
 * 
 * @param axis Pointer to the Axis to initialize
 * @param io   Pointer to an AxisIO struct with the I/O specifications
 *             for this axis
 */
static void setup_axis(Axis *axis, const AxisIO *io)
{
    axis->io = (*io);

    setup_pins(axis);
    setup_interrupts(axis);

    // reset in case encoder was accidentally triggered by noise on initialization
    reset_axis(axis);
}

/**
 * @brief Request an axis to start executing the provided motion
 * 
 * The combination of the current axis state and the provided motion must all be valid
 * for the axis to start moving. If anything is invalid an error code will be returned.
 * 
 * @param axis   Pointer to the axis to start moving
 * @param motion Pointer to an AxisMotion struct specifying the motion to execute
 * 
 * @return The result of the request to start moving (either AXIS_OK or an appropriate
 *         error code)
 */
static AxisResult start_axis(Axis *axis, AxisMotion *motion)
{
    // Reject if the counts don't match the dir
    if (motion->dir == DIR_POSITIVE &&
        (motion->counts_accel < 0 || motion->counts_hold < 0 || motion->counts_decel < 0)) {
           return AXIS_ERR_DIR_MISMATCH;
    }
    if (motion->dir == DIR_NEGATIVE &&
        (motion->counts_accel > 0 || motion->counts_hold > 0 || motion->counts_decel > 0)) {
           return AXIS_ERR_DIR_MISMATCH;
    }

    uint32_t total_counts_abs = abs(motion->counts_accel
                                    + motion->counts_hold
                                    + motion->counts_decel);
    // Reject a zero-distance motion
    if (total_counts_abs == 0) return AXIS_ERR_ZERO_DIST;

    // Reject if we're already moving - must call stop_axis first
    if (axis->state.moving) return AXIS_ERR_ALREADY_MOVING;

    // Reject if we've hit the far limit switch and are trying to move forward
    if (motion->dir == DIR_POSITIVE && axis->state.ls_far_pressed) return AXIS_ERR_LS_FAR;
    // Reject if we've hit the home limit switch and are trying to move backward
    if (motion->dir == DIR_NEGATIVE && axis->state.ls_home_pressed) return AXIS_ERR_LS_HOME;

    // Configure state
    axis->state.moving = true;
    axis->state.velocity = motion->vel_start;
    axis->state.velocity_segment = (motion->counts_accel == 0 ? VEL_SEG_HOLD : VEL_SEG_ACCELERATE);
    axis->state.encoder_target = axis->state.encoder_current + motion->counts_accel
                                 + (motion->counts_accel == 0 ? motion->counts_hold : 0);
    axis->state.dir = motion->dir;

    // Save motion spec
    axis->motion = (*motion);

    // Drive direction pin
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

/**
 * @brief Immediately stops all motion of an axis
 * 
 * This function will always be inlined so it can be called from ISRs.
 * 
 * @param axis Pointer to the axis to stop
 */
static __attribute__((always_inline)) inline void stop_axis(Axis *axis)
{
    stop_pwm_timer(axis->io.tc_step, axis->io.tc_step_channel);
    stop_timer_interrupt(
        axis->interrupts.timer,
        axis->interrupts.channel_accel,
        axis->interrupts.irq_accel);

    axis->state.moving = false;
    axis->state.velocity = 0;
}

/**
 * @brief Resets an axis's state to default values
 * 
 * This will also stop any current motion and reset the encoder counter to 0.
 * 
 * @param axis Pointer to the Axis to reset
 */
static void reset_axis(Axis *axis)
{
    stop_axis(axis);

    axis->state.moving = false;
    axis->state.velocity = 0;
    axis->state.encoder_current = 0;
    axis->state.encoder_target = 0;
    axis->state.dir = DIR_POSITIVE;
}

/**
 * @brief Get a pointer to the Axis struct corresponding to an AxisId
 * 
 * @param axis_id The AxisId identifying the axis
 * 
 * @return A pointer to the requested Axis struct
 */
static Axis *get_axis(AxisId axis_id)
{
    if (axis_id == AXIS_X) return &axis_x;
    else if (axis_id == AXIS_Y) return &axis_y;
    return nullptr;
}

/*****************************************************************************/
/*                            COMMON ISR HANDLERS                            */
/*****************************************************************************/

/**
 * NOTE: All of these functions are declared always_inline (GCC attribute)
 *       since they are called from ISRs and we want to avoid the function
 *       call overhead
 */

/**
 * @brief Common encoder ISR handler
 * 
 * @param axis Pointer to the Axis whose encoder triggered the interrupt
 */
static __attribute__((always_inline)) inline void handle_isr_encoder(Axis *axis)
{
    if (axis->state.dir == DIR_POSITIVE) axis->state.encoder_current++;
    else if (axis->state.dir == DIR_NEGATIVE) axis->state.encoder_current--;
 
    if (!axis->state.moving) return;

    if (REACHED_TARGET(axis->state.dir, axis->state.encoder_current, axis->state.encoder_target)) {
        switch (axis->state.velocity_segment) {
            case VEL_SEG_ACCELERATE:
            {
                // ACCELERATE -> HOLD
                axis->state.velocity_segment = VEL_SEG_HOLD;

                // Set new target encoder count
                int32_t error_counts = axis->state.encoder_target - axis->state.encoder_current;
                axis->state.encoder_target = axis->state.encoder_current
                                             + axis->motion.counts_hold
                                             + error_counts;

                if (!REACHED_TARGET(
                        axis->state.dir,
                        axis->state.encoder_current,
                        axis->state.encoder_target)) {
                    // Only break if we haven't already reached the next target
                    // Otherwise fall through to next case
                    break;
                }
            }
            case VEL_SEG_HOLD:
            {
                // HOLD -> DECELERATE
                axis->state.velocity_segment = VEL_SEG_DECELERATE;

                // Set new target encoder count
                int32_t error_counts = axis->state.encoder_target - axis->state.encoder_current;
                axis->state.encoder_target = axis->state.encoder_current
                                             + axis->motion.counts_decel
                                             + error_counts;

                if (!REACHED_TARGET(
                        axis->state.dir,
                        axis->state.encoder_current,
                        axis->state.encoder_target)) {
                    // Only break if we haven't already reached the next target
                    // Otherwise fall through to next case
                    break;
                }
            }
            case VEL_SEG_DECELERATE:
            {
                stop_axis(axis);
                break;
            }
        }
    }
}

/**
 * @brief Common HOME limit switch ISR handler
 * 
 * @param axis Pointer to the Axis whose limit switch triggered the interrupt
 */
static __attribute__((always_inline)) inline void handle_isr_ls_home(Axis *axis)
{
    // Limit switch reads LOW when pressed
    axis->state.ls_home_pressed = digitalRead(axis->io.pin_ls_home) == LOW;
    if (axis->state.moving && axis->state.ls_home_pressed) stop_axis(axis);
}

/**
 * @brief Common FAR limit switch ISR handler
 * 
 * @param axis Pointer to the Axis whose limit switch triggered the interrupt
 */
static __attribute__((always_inline)) inline void handle_isr_ls_far(Axis *axis)
{
    // Limit switch reads LOW when pressed
    axis->state.ls_far_pressed = digitalRead(axis->io.pin_ls_far) == LOW;
    if (axis->state.moving && axis->state.ls_far_pressed) stop_axis(axis);
}

/**
 * @brief Common acceleration timer ISR handler
 * 
 * @param axis Pointer to the Axis whose acceleration timer triggered the interrupt
 */
static __attribute__((always_inline)) inline void handle_isr_accel(Axis *axis)
{
    if (!axis->state.moving) return;

    switch (axis->state.velocity_segment) {
        case VEL_SEG_ACCELERATE:
        {
            // Increment the velocity (no further than VEL_MAX)
            if (axis->state.velocity < VEL_MAX) {
                axis->state.velocity++;
                reset_pwm_timer(
                    axis->io.tc_step,
                    axis->io.tc_step_channel,
                    axis->state.velocity);
            }
            break;
        }
        case VEL_SEG_HOLD:
        {
            // Do nothing (i.e. keep the same velocity)
            break;
        }
        case VEL_SEG_DECELERATE:
        {
            // Decrement velocity (no further than vel_start)
            if (axis->state.velocity > axis->motion.vel_start) {
                axis->state.velocity--;
                reset_pwm_timer(
                    axis->io.tc_step,
                    axis->io.tc_step_channel,
                    axis->state.velocity);
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
    handle_isr_ls_home(&axis_x);
}

void isr_ls_far_x()
{
    handle_isr_ls_far(&axis_x);
}

TC_ISR(IRQ_X_AXIS_ACCEL)
{
    // Acknowledge interrupt
    TC_GetStatus(axis_x.interrupts.timer, axis_x.interrupts.channel_accel);
    handle_isr_accel(&axis_x);
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
    handle_isr_ls_home(&axis_y);
}

void isr_ls_far_y()
{
    handle_isr_ls_far(&axis_y);
}

TC_ISR(IRQ_Y_AXIS_ACCEL)
{
    // Acknowledge interrupt
    TC_GetStatus(axis_y.interrupts.timer, axis_y.interrupts.channel_accel);
    handle_isr_accel(&axis_y);
}

/*****************************************************************************/
/*                             PUBLIC FUNCTIONS                              */
/*****************************************************************************/

/**
 * @see setup_axis(Axis *axis, const AxisIO *io)
 */
void axis_setup(AxisId axis_id, const AxisIO *io)
{
    setup_axis(get_axis(axis_id), io);
}

/**
 * @see start_axis(Axis *axis, AxisMotion *motion)
 */
AxisResult axis_start(AxisId axis_id, AxisMotion *motion)
{
    return start_axis(get_axis(axis_id), motion);
}

/**
 * @see stop_axis(Axis *axis)
 */
void axis_stop(AxisId axis_id)
{
    stop_axis(get_axis(axis_id));
}

/**
 * @see reset_axis(Axis *axis)
 */
void axis_reset(AxisId axis_id)
{
    reset_axis(get_axis(axis_id));
}


uint32_t axis_get_position(AxisId axis_id)
{
    return get_axis(axis_id)->state.encoder_current;
}

const AxisState *axis_get_state(AxisId axis_id)
{
    return &get_axis(axis_id)->state;
}

void axis_dump_state(AxisId axis_id)
{
    Axis *axis = get_axis(axis_id);
    DEBUG_PRINTLN("----------------------------------------");
    DEBUG_PRINT_VAL("AXIS            ", (axis_id == AXIS_X ? "X" : "Y"));
    DEBUG_PRINT_VAL("moving          ", axis->state.moving);
    DEBUG_PRINT_VAL("ls_home_pressed ", axis->state.ls_home_pressed);
    DEBUG_PRINT_VAL("ls_far_pressed  ", axis->state.ls_far_pressed);
    DEBUG_PRINT_VAL("velocity        ", axis->state.velocity);
    DEBUG_PRINT_VAL("velocity_segment", axis->state.velocity_segment);
    DEBUG_PRINT_VAL("encoder_current ", axis->state.encoder_current);
    DEBUG_PRINT_VAL("encoder_target  ", axis->state.encoder_target);
    DEBUG_PRINT_VAL("dir             ", axis->state.dir);
    DEBUG_PRINTLN("----------------------------------------");
}