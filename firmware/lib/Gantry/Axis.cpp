#include "Axis.h"
#include "Debug.h"
#include "Timer.h"
#include "Movement.h"

// X AXIS
#define ACCEL_X                 8000
#define VEL_MIN_X               500
#define VEL_MAX_X               25000
#define STEP_PIN_X              5 // need to know the register for fast operation pin 5 - PC25 
#define DIR_PIN_X               6 // need to know the register for fast operation pin 6 - PC24
#define ENCODER_PIN_A_X         7
#define ENCODER_PIN_B_X         8
#define LIMIT_SW_HOME_PIN_X     9
#define LIMIT_SW_FAR_PIN_X      10

// Y AXIS
#define ACCEL_Y                 8000
#define VEL_MIN_Y               500
#define VEL_MAX_Y               25000
#define STEP_PIN_Y              22 // need to know the register for fast operation pin 22 - PB26 
#define DIR_PIN_Y               23 // need to know the register for fast operation pin 23 - PA14
#define ENCODER_PIN_A_Y         24
#define ENCODER_PIN_B_Y         25
#define LIMIT_SW_HOME_PIN_Y     26
#define LIMIT_SW_FAR_PIN_Y      27

// Interrupt Declarations
void isr_encoder_x();
void isr_limit_switch_x();
void isr_encoder_y();
void isr_limit_switch_y();

AxisConfig axis_x_config = {DIR_PIN_X, STEP_PIN_X, ENCODER_PIN_A_X, ENCODER_PIN_B_X, LIMIT_SW_HOME_PIN_X, LIMIT_SW_FAR_PIN_X,
                            ACCEL_X, VEL_MAX_X, VEL_MIN_X, TC1, 0, 1, TC3_IRQn, TC4_IRQn, isr_encoder_x, isr_limit_switch_x};

AxisConfig axis_y_config = {DIR_PIN_Y, STEP_PIN_Y, ENCODER_PIN_A_Y, ENCODER_PIN_B_X, LIMIT_SW_HOME_PIN_Y, LIMIT_SW_FAR_PIN_Y,
                            ACCEL_Y, VEL_MAX_Y, VEL_MIN_Y, TC0, 0, 1, TC0_IRQn, TC1_IRQn, isr_encoder_y, isr_limit_switch_y};

Axis axis_x;
Axis axis_y;

static void setup_pins(AxisConfig *axis_config)
{
    pinMode(axis_config->encoder_pin_a, INPUT);
    pinMode(axis_config->encoder_pin_b, INPUT);
    pinMode(axis_config->ls_home, INPUT_PULLUP);
    pinMode(axis_config->ls_far, INPUT_PULLUP);
    pinMode(axis_config->dir_pin, OUTPUT);
    pinMode(axis_config->step_pin, OUTPUT);
}

static void setup_struct(AxisConfig *axis_config, Axis *axis)
{
    axis->encoder = {axis_config->encoder_pin_a, 0, 0};
    axis->ls_home = {axis_config->ls_home, (LimitSwitchStatus)digitalRead(axis_config->ls_home)};
    axis->ls_far_from_home = {axis_config->ls_far, (LimitSwitchStatus)digitalRead(axis_config->ls_far)};
    axis->dir_pin = axis_config->dir_pin;
    axis->step_pin = axis_config->step_pin;
    axis->accel = axis_config->acceleration;
    axis->vel_min = axis_config->vel_min;
    axis->vel_max = axis_config->vel_max;
    axis->vel = axis_config->vel_min;
    axis->tragectory_segment = VEL_SEG_ACCELERATE;
    axis->timer = axis_config->timer;
    axis->channel_velocity = axis_config->channel_velocity;
    axis->channel_accel = axis_config->channel_accel;
    axis->irq_velocity = axis_config->irq_velocity;
    axis->irq_accel = axis_config->irq_accel;

    axis->moving = false;
    axis->homing = false;
    axis->fault = false;
}

static void setup_interrupts(AxisConfig *axis_config, Axis *axis)
{
    attachInterrupt(digitalPinToInterrupt(axis_config->encoder_pin_a), axis_config->isr_encoder, RISING);
    delay(1000);
    attachInterrupt(digitalPinToInterrupt(axis_config->ls_home), axis_config->isr_limit_switch, CHANGE);
    delay(1000);
    attachInterrupt(digitalPinToInterrupt(axis_config->ls_far), axis_config->isr_limit_switch, CHANGE);
}

void reset_axis(AxisConfig *axis_config, Axis *axis)
{
    setup_struct(axis_config, axis);
}

void setup_axis(AxisConfig *axis_config, Axis *axis)
{  
    setup_pins(axis_config);
    setup_struct(axis_config, axis);
    setup_interrupts(axis_config, axis);
    
    // reseting axis in case encoder has be triggered or a limit switch due to noise on initilization
    DEBUG_PRINT_VAL("Reseting axis", 0);
    reset_axis(axis_config, axis);
}

void start_axis(Axis *axis)
{
    axis->moving = true;
    start_timer(axis->timer, axis->channel_velocity, axis->irq_velocity, axis->vel);
    start_timer_accel(axis->timer, axis->channel_accel, axis->irq_accel, axis->accel);
}

void stop_axis(Axis *axis)
{
    NVIC_DisableIRQ(axis->irq_accel);
    NVIC_DisableIRQ(axis->irq_velocity);
    axis->moving = false;
}

// -------------------------------- X AXIS INTERRUPTS -------------------------------- //

// isr to handle encoder of x axis
void isr_encoder_x()
{
    if (axis_x.dir == DIR_POSITIVE) axis_x.encoder.current++;
    else if (axis_x.dir == DIR_NEGATIVE) axis_x.encoder.current--;
    // DEBUG_PRINT_VAL("", axis_x.encoder.current);
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
        // DEBUG_PRINT_VAL("X Home Limit switch has been hit ", status);
        if (axis_x.homing) {
            if (status == RELEASED) {
                axis_x.encoder.current = 0;
                axis_x.homing = false;
                DEBUG_PRINT_VAL("X axis has been homed - SUCCESS", 1);
            }
            else if (status == PRESSED) {
                DEBUG_PRINT_VAL("HOMING IN POSITIVE DIR ", 0);
                home_axis(&axis_x);
            }
        }
        else {
            axis_x.fault = true;
        }
    }
    last_interrupt_time = interrupt_time;
}

// isr for operating x axis motor velocity
void TC3_Handler(void)
{   
    // DEBUG_PRINT_VAL("Desired count", axis_x.encoder.desired);
    // DEBUG_PRINT_VAL("Current count", axis_x.encoder.current);
    TC_GetStatus(TC1, 0); // Get current status on the selected channel
    PIOC->PIO_ODSR ^= PIO_ODSR_P25;
    reset_timer(TC1, 0, TC3_IRQn, axis_x.vel); 
}

// isr for operating x axis motor acceleration 
void TC4_Handler(void)
{   
    // DEBUG_PRINT_VAL("Desired count", axis_x.encoder.desired);
    // DEBUG_PRINT_VAL("Current count", axis_x.encoder.current);
    TC_GetStatus(TC1, 1); // Get current status on the selected channel
    // first accelerate
    if (axis_x.tragectory_segment == VEL_SEG_ACCELERATE) {
        if (((axis_x.encoder.current < axis_x.encoder.desired) != axis_x.dir) && axis_x.vel < axis_x.vel_max) {
            axis_x.vel++;
        }
        else {
            axis_x.tragectory_segment = VEL_SEG_HOLD;
            int delta = axis_x.encoder.desired - axis_x.encoder.current;
            axis_x.encoder.desired = axis_x.encoder.current + axis_x.vel_profile_cur_trap[1] + delta;
            // DEBUG_PRINT_VAL("accel d-c", delta);
        }
    }
    // hold velocity
    else if (axis_x.tragectory_segment == VEL_SEG_HOLD) {
        if ((axis_x.encoder.current < axis_x.encoder.desired) != axis_x.dir) {
            // DEBUG_PRINT_VAL("DC H", axis_x.encoder.desired);
            // DEBUG_PRINT_VAL("CC H", axis_x.encoder.current);
        }
        else {
            axis_x.tragectory_segment = VEL_SEG_DECELERATE;
            int delta = axis_x.encoder.desired - axis_x.encoder.current;
            axis_x.encoder.desired = axis_x.encoder.current + axis_x.vel_profile_cur_trap[2] + delta;
            // DEBUG_PRINT_VAL("hold d-c", delta);
        }
    }
    // decelerate
    else if (axis_x.tragectory_segment == VEL_SEG_DECELERATE) {
        if (((axis_x.encoder.current < axis_x.encoder.desired) != axis_x.dir)) {
            if (axis_x.vel > axis_x.vel_min) {
                axis_x.vel--;
            }
            // DEBUG_PRINT_VAL("DC H", axis_x.encoder.desired);
            // DEBUG_PRINT_VAL("CC H", axis_x.encoder.current);
        }
        else {
            NVIC_DisableIRQ(TC3_IRQn);
            NVIC_DisableIRQ(TC4_IRQn);
            DEBUG_PRINT_VAL("decel d-c", (axis_x.encoder.desired - axis_x.encoder.current));
        }
    }
}


// -------------------------------- Y AXIS INTERRUPTS -------------------------------- //

// isr to handle encoder of x axis
void isr_encoder_y()
{
    if (axis_y.dir == DIR_POSITIVE) axis_y.encoder.current++;
    else if (axis_y.dir == DIR_NEGATIVE) axis_y.encoder.current--;
    DEBUG_PRINT_VAL("Encoder ISR count", axis_y.encoder.current);
}

// isr to handle limit switch
void isr_limit_switch_y()
{   
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();
    // If interrupts come faster than 10ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 50) {
        stop_axis(&axis_x);
        DEBUG_PRINT_VAL("Y Home Limit switch has been hit", 1);

        LimitSwitchStatus status = (LimitSwitchStatus)digitalRead(LIMIT_SW_HOME_PIN_Y);
        if (axis_y.homing) {
            if (status == RELEASED) {
                axis_y.encoder.current = 0;
                axis_y.homing = false;
                DEBUG_PRINT_VAL("Y axis has been homed - SUCCESS", 1);
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

// isr for operating x axis motor velocity
void TC0_Handler(void)
{   
    // DEBUG_PRINT_VAL("Desired count", axis_x.encoder.desired);
    // DEBUG_PRINT_VAL("Current count", axis_x.encoder.current);
    TC_GetStatus(TC0, 0); // Get current status on the selected channel
    PIOB->PIO_ODSR ^= PIO_ODSR_P26;
    reset_timer(TC0, 0, TC0_IRQn, axis_y.vel);
}

// isr for operating x axis motor acceleration 
void TC1_Handler(void)
{   
    // DEBUG_PRINT_VAL("Desired count", axis_x.encoder.desired);
    // DEBUG_PRINT_VAL("Current count", axis_x.encoder.current);
    TC_GetStatus(TC0, 1); // Get current status on the selected channel
    // first accelerate
    if (axis_y.tragectory_segment == VEL_SEG_ACCELERATE) {
        if (((axis_y.encoder.current < axis_y.encoder.desired) != axis_y.dir) && axis_y.vel < axis_y.vel_max) {
            axis_y.vel++;
            return;
        }
        else {
            axis_y.tragectory_segment = VEL_SEG_HOLD;
            int delta = axis_y.encoder.desired - axis_y.encoder.current;
            axis_y.encoder.desired = axis_y.encoder.current + axis_y.vel_profile_cur_trap[1] + delta;
            // DEBUG_PRINT_VAL("accel d-c", delta);
            return;
        }
    }
    // hold velocity
    else if (axis_y.tragectory_segment == VEL_SEG_HOLD) {
        if ((axis_y.encoder.current < axis_y.encoder.desired) != axis_y.dir) {
            // DEBUG_PRINT_VAL("DC H", axis_x.encoder.desired);
            // DEBUG_PRINT_VAL("CC H", axis_x.encoder.current);
            return;
        }
        else {
            axis_y.tragectory_segment = VEL_SEG_DECELERATE;
            int delta = axis_y.encoder.desired - axis_y.encoder.current;
            axis_y.encoder.desired = axis_y.encoder.current + axis_y.vel_profile_cur_trap[2] + delta;
            // DEBUG_PRINT_VAL("hold d-c", delta);
            return;
        }
    }
    // decelerate
    else if (axis_y.tragectory_segment == VEL_SEG_DECELERATE) {
        if ((axis_y.encoder.current < axis_y.encoder.desired) != axis_y.dir) {
            if (axis_y.vel > axis_y.vel_min) {
                axis_y.vel--;
            }
            // DEBUG_PRINT_VAL("DC H", axis_x.encoder.desired);
            // DEBUG_PRINT_VAL("CC H", axis_x.encoder.current);
            return;
        }
        else {
            NVIC_DisableIRQ(TC0_IRQn);
            NVIC_DisableIRQ(TC1_IRQn);
            DEBUG_PRINT_VAL("decel d-c", (axis_y.encoder.desired - axis_y.encoder.current));
        }
    }
}