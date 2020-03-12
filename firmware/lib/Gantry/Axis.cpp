#include "Axis.h"
#include "Debug.h"
#include "Timer.h"


// -------------------------------- X AXIS SETUP -------------------------------- //
#define ACCEL_X                 100
#define VEL_MIN_X               40
#define VEL_MAX_X               100
#define STEP_PIN_X              5 // need to know the register for fast operation pin 5 - PC25 
#define DIR_PIN_X               6 // need to know the register for fast operation pin 6 - PC24
#define ENCODER_PIN_A_X         7
#define ENCODER_PIN_B_X         8
#define LIMIT_SW_HOME_PIN_X     9
#define LIMIT_SW_FAR_PIN_X      10

AxisConfig axis_x_config = {DIR_PIN_X, STEP_PIN_X, ENCODER_PIN_A_X, ENCODER_PIN_B_X, LIMIT_SW_HOME_PIN_X, LIMIT_SW_FAR_PIN_X,
                            ACCEL_X, VEL_MAX_X, VEL_MIN_X, TC1, 0, 1, TC3_IRQn, TC4_IRQn, isr_encoder_x, isr_limit_switch_x};

Axis axis_x;

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
    axis->ls_home = {axis_config->ls_home, digitalRead(axis_config->ls_home)};
    axis->ls_far_from_home = {axis_config->ls_far, digitalRead(axis_config->ls_far)};
    axis->dir_pin = axis_config->dir_pin;
    axis->step_pin = axis_config->step_pin;
    axis->accel = axis_config->acceleration;
    axis->vel_min = axis_config->vel_min;
    axis->vel_max = axis_config->vel_max;
    axis->vel = axis_config->vel_min;
    axis->tragectory_segment = ACCELERATE;
    axis->timer = axis_config->timer;
    axis->channel_velocity = axis_config->channel_velocity;
    axis->channel_accel = axis_config->channel_accel;
    axis->isr_velocity = axis_config->isr_velocity;
    axis->isr_accel = axis_config->isr_accel;
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
    print("Reseting axis", 0);
    reset_axis(axis_config, axis);
}

// isr to handle encoder of x axis
void isr_encoder_x()
{
    // static uint32_t last_interrupt_time = 0;
    // uint32_t interrupt_time = millis();
    // If interrupts come faster than 200ms, assume it's a bounce and ignore
    // if (interrupt_time - last_interrupt_time > 10) {
    if (axis_x.dir == POSITIVE) axis_x.encoder.current++;
    else if (axis_x.dir == NEGATIVE) axis_x.encoder.current--;
    print("", axis_x.encoder.current);
    // }
    // last_interrupt_time = interrupt_time;
}

// isr to handle limit switch
void isr_limit_switch_x()
{   
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();
    // If interrupts come faster than 10ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 100) {
        NVIC_DisableIRQ(axis_x.isr_velocity);
        NVIC_DisableIRQ(axis_x.isr_accel);
        print("X Home Limit switch has been hit", 1);

        if (axis_x.homing && digitalRead(LIMIT_SW_HOME_PIN_X) == DEPRESSED) {
            axis_x.encoder.current = 0;
            axis_x.homing = 0;
            print("X axis has been homed - SUCCESS", 1);
        }
        else if (axis_x.homing && digitalRead(LIMIT_SW_HOME_PIN_X) == PRESSED) {
            home_axis(&axis_x);
        }
    }
    last_interrupt_time = interrupt_time;
}

// isr for operating x axis motor velocity
void TC3_Handler(void)
{   
    // print("Desired count: ", axis_x.encoder.desired);
    // print("Current count: ", axis_x.encoder.current);
    TC_GetStatus(TC1, 0); // Get current status on the selected channel
    PIOC->PIO_ODSR ^= PIO_ODSR_P25;
    reset_timer(TC1, 0, TC3_IRQn, axis_x.vel); 
}

// isr for operating x axis motor acceleration 
void TC4_Handler(void)
{   
    // print("Desired count: ", axis_x.encoder.desired);
    // print("Current count: ", axis_x.encoder.current);
    TC_GetStatus(TC1, 1); // Get current status on the selected channel
    // first accelerate
    if (axis_x.tragectory_segment == ACCELERATE) {
        if (((axis_x.encoder.current < axis_x.encoder.desired) != axis_x.dir) && axis_x.vel < axis_x.vel_max) {
            axis_x.vel++;
        }
        else {
            axis_x.tragectory_segment = HOLD;
            int delta = axis_x.encoder.desired - axis_x.encoder.current;
            axis_x.encoder.desired = axis_x.encoder.current + axis_x.vel_profile_cur_trap[1] + delta;
            // print("accel d-c: ", delta);
        }
    }
    // hold velocity
    else if (axis_x.tragectory_segment == HOLD) {
        if ((axis_x.encoder.current < axis_x.encoder.desired) != axis_x.dir) {
            // print("DC H: ", axis_x.encoder.desired);
            // print("CC H: ", axis_x.encoder.current);
        }
        else {
            axis_x.tragectory_segment = DECELERATE;
            int delta = axis_x.encoder.desired - axis_x.encoder.current;
            axis_x.encoder.desired = axis_x.encoder.current + axis_x.vel_profile_cur_trap[2] + delta;
            // print("hold d-c: ", delta);
        }
    }
    // decelerate
    else if (axis_x.tragectory_segment == DECELERATE) {
        if (((axis_x.encoder.current < axis_x.encoder.desired) != axis_x.dir)) {
            if (axis_x.vel > axis_x.vel_min) {
                axis_x.vel--;
            }
            // print("DC H: ", axis_x.encoder.desired);
            // print("CC H: ", axis_x.encoder.current);
        }
        else {
            NVIC_DisableIRQ(TC3_IRQn);
            NVIC_DisableIRQ(TC4_IRQn);
            int delta = axis_x.encoder.desired - axis_x.encoder.current;
            print("decel d-c: ", delta);
        }
    }
}


// -------------------------------- Y AXIS SETUP -------------------------------- //
#define ACCEL_Y                 100
#define VEL_MIN_Y               40
#define VEL_MAX_Y               100
#define STEP_PIN_Y              22 // need to know the register for fast operation pin 22 - PB26 
#define DIR_PIN_Y               23 // need to know the register for fast operation pin 23 - PA14
#define ENCODER_PIN_A_Y         24
#define ENCODER_PIN_B_Y         25
#define LIMIT_SW_HOME_PIN_Y     26
#define LIMIT_SW_FAR_PIN_Y      27

AxisConfig axis_y_config = {DIR_PIN_Y, STEP_PIN_Y, ENCODER_PIN_A_Y, ENCODER_PIN_B_X, LIMIT_SW_HOME_PIN_Y, LIMIT_SW_FAR_PIN_Y,
                            ACCEL_Y, VEL_MAX_Y, VEL_MIN_Y, TC0, 0, 1, TC0_IRQn, TC1_IRQn, isr_encoder_y, isr_limit_switch_y};

Axis axis_y;
// isr to handle encoder of x axis
void isr_encoder_y()
{
    if (axis_y.dir == POSITIVE) axis_y.encoder.current++;
    else if (axis_y.dir == NEGATIVE) axis_y.encoder.current--;
    print("Encoder ISR count: ", axis_y.encoder.current);
}

// isr to handle limit switch
void isr_limit_switch_y()
{   
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();
    // If interrupts come faster than 10ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 50) {
        NVIC_DisableIRQ(axis_y.isr_velocity);
        NVIC_DisableIRQ(axis_y.isr_accel);
        print("Y Home Limit switch has been hit", 1);

        if (axis_y.homing && digitalRead(LIMIT_SW_HOME_PIN_Y) == DEPRESSED) {
            axis_y.encoder.current = 0;
            axis_y.homing = 0;
            print("Y axis has been homed - SUCCESS", 1);
        }
        else if (axis_y.homing && digitalRead(LIMIT_SW_HOME_PIN_Y) == PRESSED) {
            home_axis(&axis_y);
        }
    }
    last_interrupt_time = interrupt_time;
}

// isr for operating x axis motor velocity
void TC0_Handler(void)
{   
    // print("Desired count: ", axis_x.encoder.desired);
    // print("Current count: ", axis_x.encoder.current);
    TC_GetStatus(TC0, 0); // Get current status on the selected channel
    PIOB->PIO_ODSR ^= PIO_ODSR_P26;
    reset_timer(TC0, 0, TC0_IRQn, axis_y.vel);
}

// isr for operating x axis motor acceleration 
void TC1_Handler(void)
{   
    // print("Desired count: ", axis_x.encoder.desired);
    // print("Current count: ", axis_x.encoder.current);
    TC_GetStatus(TC0, 1); // Get current status on the selected channel
    // first accelerate
    if (axis_y.tragectory_segment == ACCELERATE) {
        if (((axis_y.encoder.current < axis_y.encoder.desired) != axis_y.dir) && axis_y.vel < axis_y.vel_max) {
            axis_y.vel++;
            return;
        }
        else {
            axis_y.tragectory_segment = HOLD;
            int delta = axis_y.encoder.desired - axis_y.encoder.current;
            axis_y.encoder.desired = axis_y.encoder.current + axis_y.vel_profile_cur_trap[1] + delta;
            // print("accel d-c: ", delta);
            return;
        }
    }
    // hold velocity
    else if (axis_y.tragectory_segment == HOLD) {
        if ((axis_y.encoder.current < axis_y.encoder.desired) != axis_y.dir) {
            // print("DC H: ", axis_x.encoder.desired);
            // print("CC H: ", axis_x.encoder.current);
            return;
        }
        else {
            axis_y.tragectory_segment = DECELERATE;
            int delta = axis_y.encoder.desired - axis_y.encoder.current;
            axis_y.encoder.desired = axis_y.encoder.current + axis_y.vel_profile_cur_trap[2] + delta;
            // print("hold d-c: ", delta);
            return;
        }
    }
    // decelerate
    else if (axis_y.tragectory_segment == DECELERATE) {
        if ((axis_y.encoder.current < axis_y.encoder.desired) != axis_y.dir) {
            if (axis_y.vel > axis_y.vel_min) {
                axis_y.vel--;
            }
            // print("DC H: ", axis_x.encoder.desired);
            // print("CC H: ", axis_x.encoder.current);
            return;
        }
        else {
            NVIC_DisableIRQ(TC0_IRQn);
            NVIC_DisableIRQ(TC1_IRQn);
            int delta = axis_y.encoder.desired - axis_y.encoder.current;
            print("decel d-c: ", delta);
        }
    }
}

// MOVEMENT
void axis_trapezoidal_move_rel(Axis *axis, uint32_t counts_accel, uint32_t counts_hold, uint32_t counts_decel, Direction dir)
{   
    if (counts_accel == 0 && counts_hold == 0 && counts_decel == 0) return;
    axis->dir = dir;
    digitalWrite(axis->dir_pin, dir);

    if (dir == POSITIVE) {
        axis->vel_profile_cur_trap[0] = counts_accel;
        axis->vel_profile_cur_trap[1] = counts_hold;
        axis->vel_profile_cur_trap[2] = counts_decel;
    }
    else if (dir == NEGATIVE) {
        axis->vel_profile_cur_trap[0] = - counts_accel;
        axis->vel_profile_cur_trap[1] = - counts_hold;
        axis->vel_profile_cur_trap[2] = - counts_decel;
    }
    
    axis->encoder.desired = axis->encoder.current + axis->vel_profile_cur_trap[0];
    
    print("Desired count: ", axis->encoder.desired);
    print("Current count: ", axis->encoder.current);
    start_timer(axis->timer, axis->channel_velocity, axis->isr_velocity, axis->vel);
    start_timer_accel(axis->timer, axis->channel_accel, axis->isr_accel, axis->accel);
}

void axis_trapezoidal_move_tri(Axis *axis, uint32_t counts_accel, uint32_t counts_decel, Direction dir)
{   
    axis_trapezoidal_move_rel(axis, counts_accel, 0, counts_decel, dir);
}

void home_axis(Axis *axis)
{   
    axis->homing = 1;
    // math to calculate number of accel counts
    uint32_t counts_accel;
    // check if the limit switched is pressed at home
    if (digitalRead(axis->ls_home.pin) == PRESSED) {
        axis_trapezoidal_move_rel(axis, counts_accel, UINT32_MAX, counts_accel, POSITIVE); // move until limit switch is depressed
        return;
    }
    else {
        axis_trapezoidal_move_rel(axis, counts_accel, UINT32_MAX, counts_accel, NEGATIVE);
        return;
    }
}

void stop_axis(Axis *axis)
{
    NVIC_DisableIRQ(TC0_IRQn);
    NVIC_DisableIRQ(TC1_IRQn);
    NVIC_DisableIRQ(TC3_IRQn);
    NVIC_DisableIRQ(TC4_IRQn);
}