/* **************************** Local Includes ***************************** */
#include "Axis.h"
#include "Kinematics.h"
#include "Timer.h"

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

/** Maximum allowed velocity for an axis [motor steps / second] */
#define VEL_MAX                25000

/** Percentage of time the velocity PWM signal is ON */
#define VEL_DUTY_CYCLE         25

/**
 * The IRQ numbers for the step timers for each axis must be defined at compile time
 * so the correct TC?_Handler functions can be defined
 */

#ifndef AXIS_X_STEP_TC_IRQ
#error "AXIS_X_STEP_TC_IRQ must be defined"
#endif //AXIS_X_VEL_TC_IRQ

#ifndef AXIS_Y_STEP_TC_IRQ
#error "AXIS_Y_STEP_TC_IRQ must be defined"
#endif //AXIS_Y_VEL_TC_IRQ

/*****************************************************************************/
/*                                 TYPEDEFS                                  */
/*****************************************************************************/

/**
 * @struct AxisInterrupts
 * 
 * @brief Specifies all of the internal interrupts and ISRs used by the axis
 */
typedef struct {
    Tc * const timer;                 //!< Timer counter
    const uint32_t channel_accel;     //!< Timer counter channel for the acceleration interrupt
    const IRQn_Type irq_accel;        //!< IRQ number for the acceleration interrupt

    void (*const isr_encoder)(void);  //!< ISR for the encoder interrupt
    void (*const isr_ls_home)(void);  //!< ISR for the home limit switch interrupt
    void (*const isr_ls_far)(void);   //!< ISR for the far limit switch interrupt
} AxisInterrupts;

typedef struct {
    AxisMotionSpec spec;              //!< Specification for the current motion
    VelProfile profile;               //!< Profile for the current motion [encoder counts]
} AxisMotion;

/**
 * @struct Axis
 * 
 * @brief Top-level struct for encapsulating all elements of an axis
 */
typedef struct {
    AxisIO io;                         //!< I/O pins and peripherals
    AxisMech mech;                     //!< Mechanical parameters
    const AxisInterrupts interrupts;   //!< Interrupt configurations
    AxisMotion motion;                 //!< Information about the current motion
    AxisState state;                   //!< Current state of the axis
} Axis;

/*****************************************************************************/
/*                             AXIS DECLARATIONS                             */
/*****************************************************************************/

/* ******************************** X AXIS ********************************* */

void isr_encoder_x();
void isr_ls_home_x();
void isr_ls_far_x();

static Axis axis_x = {
    .io = {},
    .mech = {},
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
    .mech = {},
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
    // HOME limit switch should trigger on both edges since it is used for initial position calibration
    setup_debouncing(axis->io.pin_ls_home, DEBOUNCE_FILTER_MS);
    attachInterrupt(digitalPinToInterrupt(axis->io.pin_ls_home),
                    axis->interrupts.isr_ls_home,
                    CHANGE);

    // Debounce and attach interrupt to FAR limit switch
    // FAR limit switch should only trigger when pressed
    setup_debouncing(axis->io.pin_ls_far, DEBOUNCE_FILTER_MS);
    attachInterrupt(digitalPinToInterrupt(axis->io.pin_ls_far),
                    axis->interrupts.isr_ls_far,
                    (axis->io.ls_pressed_state == LOW ? FALLING : RISING));
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
static void setup_axis(Axis *axis, const AxisIO *io, const AxisMech *mech)
{
    axis->io = (*io);
    axis->mech = (*mech);

    setup_pins(axis);
    setup_interrupts(axis);

    // reset in case encoder was accidentally triggered by noise on initialization
    reset_axis(axis);
}

static AxisResult validate_motion(Axis *axis, AxisMotionSpec *motion)
{
    // Reject if we're already moving - must call stop_axis first
    if (axis->state.moving) return AXIS_ERR_ALREADY_MOVING;

    // Reject if we've hit the far limit switch and are trying to move forward
    if (motion->dir == DIR_POSITIVE && axis->state.ls_far_pressed) return AXIS_ERR_LS_FAR;
    // Reject if we've hit the home limit switch and are trying to move backward
    if (motion->dir == DIR_NEGATIVE && axis->state.ls_home_pressed) return AXIS_ERR_LS_HOME;

    return AXIS_OK;
}

/**
 * @brief Request an axis to start executing the provided motion
 * 
 * The combination of the current axis state and the provided motion must all be valid
 * for the axis to start moving. If anything is invalid an error code will be returned.
 * 
 * @param axis   Pointer to the axis to start moving
 * @param motion Pointer to an AxisMotionSpec struct specifying the motion to execute
 * 
 * @return The result of the request to start moving (either AXIS_OK or an appropriate
 *         error code)
 */
static AxisResult start_axis(Axis *axis, AxisMotionSpec *motion)
{
    // Validate the motion
    AxisResult validation = validate_motion(axis, motion);
    if (validation != AXIS_OK) return validation;

    // Convert from steps to counts
    uint32_t accel = motion->accel * axis->mech.counts_per_rev / axis->mech.steps_per_rev;
    uint32_t vel_start = motion->vel_start * axis->mech.counts_per_rev / axis->mech.steps_per_rev;
    uint32_t vel_hold = motion->vel_hold * axis->mech.counts_per_rev / axis->mech.steps_per_rev;

    // Generate velocity profile
    bool valid_profile = generate_vel_profile(
        (motion->dir == DIR_NEGATIVE),
        accel, vel_start, vel_hold,
        motion->total_counts,
        &(axis->motion.profile));
    if (!valid_profile) return AXIS_ERR_INVALID;

    // Save motion spec
    axis->motion.spec = (*motion);

    // Configure state
    axis->state.moving = true;
    axis->state.velocity = axis->motion.spec.vel_start;
    axis->state.next_velocity = axis->state.velocity;
    axis->state.velocity_segment = VEL_SEG_ACCELERATE;
    axis->state.encoder_target = axis->state.encoder_current + axis->motion.profile.dist_accel;
    axis->state.dir = axis->motion.spec.dir;

    // Drive direction pin
    digitalWrite(axis->io.pin_dir, (axis->state.dir == DIR_POSITIVE ? LOW : HIGH));

    // Start velocity PWM timer
    reset_pwm_timer(
        axis->io.tc_step,
        axis->io.tc_step_channel,
        axis->io.tc_step_irq,
        axis->state.velocity,
        VEL_DUTY_CYCLE);

    // Start acceleration timer interrupt
    reset_timer_interrupt(
        axis->interrupts.timer,
        axis->interrupts.channel_accel,
        axis->interrupts.irq_accel,
        axis->motion.spec.accel);

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
    stop_timer(axis->io.tc_step, axis->io.tc_step_channel, axis->io.tc_step_irq);
    stop_timer(axis->interrupts.timer, axis->interrupts.channel_accel, axis->interrupts.irq_accel);

    axis->state.moving = false;
    axis->state.velocity = 0;
    axis->state.next_velocity = 0;
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
}

/**
 * @brief Common HOME limit switch ISR handler
 * 
 * @param axis Pointer to the Axis whose limit switch triggered the interrupt
 */
static __attribute__((always_inline)) inline void handle_isr_ls_home(Axis *axis)
{
    stop_axis(axis);
    axis->state.ls_home_pressed = (digitalRead(axis->io.pin_ls_home) == axis->io.ls_pressed_state);
}

/**
 * @brief Common FAR limit switch ISR handler
 * 
 * @param axis Pointer to the Axis whose limit switch triggered the interrupt
 */
static __attribute__((always_inline)) inline void handle_isr_ls_far(Axis *axis)
{
    stop_axis(axis);
    axis->state.ls_far_pressed = (digitalRead(axis->io.pin_ls_far) == axis->io.ls_pressed_state);
}

static __attribute__((always_inline)) inline void handle_isr_step(Axis *axis)
{
    // Reset the timer with a new velocity if it has changed
    if (axis->state.next_velocity != axis->state.velocity) {
        axis->state.velocity = axis->state.next_velocity;
        reset_pwm_timer(
            axis->io.tc_step,
            axis->io.tc_step_channel,
            axis->io.tc_step_irq,
            axis->state.velocity,
            VEL_DUTY_CYCLE);
    }
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
            // Accelerate until we reach the holding velocity (however many counts that takes)
            if (axis->state.next_velocity < axis->motion.spec.vel_hold) {
                axis->state.next_velocity++;
            }
            else {
                axis->state.velocity_segment = VEL_SEG_HOLD;

                int32_t error_counts = axis->state.encoder_target - axis->state.encoder_current;
                axis->state.encoder_target = axis->state.encoder_current
                                             + axis->motion.profile.dist_hold
                                             + error_counts;
            }
            break;
        }
        case VEL_SEG_HOLD:
        {
            // Don't do anything (i.e. keep velocity constant) until we reach the target encoder count
            if (REACHED_TARGET(axis->state.dir, axis->state.encoder_current, axis->state.encoder_target)) {
                axis->state.velocity_segment = VEL_SEG_DECELERATE;

                int32_t error_counts = axis->state.encoder_target - axis->state.encoder_current;
                axis->state.encoder_target = axis->state.encoder_current
                                             + axis->motion.profile.dist_decel
                                             + error_counts;
            }
            break;
        }
        case VEL_SEG_DECELERATE:
        {
            if (!REACHED_TARGET(axis->state.dir, axis->state.encoder_current, axis->state.encoder_target)) {
                // Only decrement velocity if we're still above the starting velocity
                if (axis->state.next_velocity > axis->motion.spec.vel_start) {
                    axis->state.next_velocity--;
                }
            }
            else {
                // Stop moving once we reach the final target encoder count
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
    handle_isr_ls_home(&axis_x);
}

void isr_ls_far_x()
{
    handle_isr_ls_far(&axis_x);
}

TC_ISR(AXIS_X_STEP_TC_IRQ)
{
    TC_GetStatus(axis_x.io.tc_step, axis_x.io.tc_step_channel);
    handle_isr_step(&axis_x);
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

TC_ISR(AXIS_Y_STEP_TC_IRQ)
{
    TC_GetStatus(axis_y.io.tc_step, axis_y.io.tc_step_channel);
    handle_isr_step(&axis_y);
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
void axis_setup(AxisId axis_id, const AxisIO *io, const AxisMech *mech)
{
    setup_axis(get_axis(axis_id), io, mech);
}

/**
 * @see start_axis(Axis *axis, AxisMotion *motion)
 */
AxisResult axis_start(AxisId axis_id, AxisMotionSpec *motion)
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

/**
 * @brief Returns a read-only (i.e. const) pointer to the state information for the given axis
 * 
 * @param axis_id The AxisId identifying the axis
 * 
 * @return A const pointer to the axis's state
 */
const AxisState *axis_get_state(AxisId axis_id)
{
    return &get_axis(axis_id)->state;
}
