#include "Arduino.h"
/*
TODO:
    - Add limit switch interrupt handling routines
    - Add homing routine
    - Add ansolute movement routines (might be done at PC level)
    - Quadrature encoder ISR needs to be rewritten and direction needs to be incorporated
        -- check direction values (positive HIGH or LOW)
    - Add error handling and error raising for impossible encoder counts - negative values etc
      might not be necessary since we have limit switched - needs more thought
    - Add mode where only motor counts are used, in case we dont need encoders
*/

// ++++++++++++++++++++++++ debug helper functions start
#define DEBUG   1

void debug() {
    if (DEBUG) {Serial.begin(115200);}
}

void print(String text, uint32_t val)
{   
    // Serial.print("DEBUG     ");
    Serial.print(text);
    Serial.println(val);
}
// ------------------------ debug helper functions end

enum direction{positive, negative};
enum status{pressed, depressed};

typedef struct LimitSwitch
{
    int pin;
    int status;
} LimitSwitch;

typedef struct Encoder
{
    uint32_t pin;
    volatile uint32_t current;
    uint32_t desired;
} Encoder;

typedef struct Axis
{
    int dir_pin;
    int step_pin;
    uint32_t accel;
    uint32_t vel_max;
    uint32_t vel;
    int32_t vel_profile_cur[3]; // defined in counts accelerate, hold, decelerate
    int tragectory_segment;
    Encoder encoder;
    LimitSwitch ls_home;
    LimitSwitch ls_far_from_home;
    // necessary timer info
    Tc *timer;
    uint32_t channel_velocity;
    uint32_t channel_accel;
    IRQn_Type isr_velocity;
    IRQn_Type isr_accel;
    direction dir;
} Axis;

void start_timer(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t velocity)
{
    pmc_set_writeprotect(false);
    pmc_enable_periph_clk((uint32_t)irq);
    TC_Configure(tc, channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC
                                          |TC_CMR_TCCLKS_TIMER_CLOCK4);
    uint32_t rc = VARIANT_MCK/128/velocity/2; // over 2 because need to toggle down
    TC_SetRA(tc, channel, rc/2); //50% high, 50% low
    TC_SetRC(tc, channel, rc);
    TC_Start(tc, channel);
    tc->TC_CHANNEL[channel].TC_IER=TC_IER_CPCS;
    tc->TC_CHANNEL[channel].TC_IDR=~TC_IER_CPCS;
    NVIC_EnableIRQ(irq);
}

void reset_timer(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t velocity)
{   
    NVIC_DisableIRQ(irq);
    uint32_t rc = VARIANT_MCK/128/velocity/2; // over 2 because need to toggle down
    TC_SetRA(tc, channel, rc/2); //50% high, 50% low
    TC_SetRC(tc, channel, rc);
    TC_Start(tc, channel);
    tc->TC_CHANNEL[channel].TC_IER=TC_IER_CPCS;
    tc->TC_CHANNEL[channel].TC_IDR=~TC_IER_CPCS;
    NVIC_EnableIRQ(irq);
}

void start_timer_accel(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t accel)
{
    pmc_set_writeprotect(false);
    pmc_enable_periph_clk((uint32_t)irq);
    TC_Configure(tc, channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC
                                          |TC_CMR_TCCLKS_TIMER_CLOCK4);
    uint32_t rc = VARIANT_MCK/128/accel; // over 2 because need to toggle down
    TC_SetRA(tc, channel, rc/2); //50% high, 50% low
    TC_SetRC(tc, channel, rc);
    TC_Start(tc, channel);
    tc->TC_CHANNEL[channel].TC_IER=TC_IER_CPCS;
    tc->TC_CHANNEL[channel].TC_IDR=~TC_IER_CPCS;
    NVIC_EnableIRQ(irq);
}

void reset_timer_accel(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t accel)
{   
    NVIC_DisableIRQ(irq);
    uint32_t rc = VARIANT_MCK/128/accel; // over 2 because need to toggle down
    TC_SetRA(tc, channel, rc/2); //50% high, 50% low
    TC_SetRC(tc, channel, rc);
    TC_Start(tc, channel);
    tc->TC_CHANNEL[channel].TC_IER=TC_IER_CPCS;
    tc->TC_CHANNEL[channel].TC_IDR=~TC_IER_CPCS;
    NVIC_EnableIRQ(irq);
}

void axis_trapezoidal_move_rel(Axis *axis, uint32_t vel_max, uint32_t counts_accel, uint32_t counts_const, uint32_t counts_decel, direction dir)
{   
    axis->dir = dir;
    digitalWrite(axis->dir_pin, dir);

    if (vel_max < axis->vel_max && vel_max != 0) {
        axis->vel_max = vel_max;
    }
    
    if (dir == positive) {
        axis->vel_profile_cur[0] = counts_accel;
        axis->vel_profile_cur[1] = counts_const;
        axis->vel_profile_cur[2] = counts_decel;
    }
    else if (dir == negative) {
        axis->vel_profile_cur[0] = - counts_accel;
        axis->vel_profile_cur[1] = - counts_const;
        axis->vel_profile_cur[2] = - counts_decel;
    }
    
    axis->encoder.desired = axis->encoder.current + axis->vel_profile_cur[0];
    
    // print("Desired count: ", axis->encoder.desired);
    // print("Current count: ", axis->encoder.current);
    start_timer(axis->timer, axis->channel_velocity, axis->isr_velocity, axis->vel);
    start_timer_accel(axis->timer, axis->channel_accel, axis->isr_accel, axis->accel);
}


void axis_move_abs(Axis *axis, uint32_t counts_accel, uint32_t counts_const, uint32_t counts_decel)
{

}

void axis_move(Axis *axis, direction dir) 
{

}

void home_axis(Axis *axis)
{   
    // check if the limit switched is pressed at home
    if (digitalRead(axis->ls_home.pin) == pressed) {
        axis_move(axis, positive); // move until limit switch is depressed
        // set this as zero position
        axis->encoder.current = 0;
        return;
    }
    else {
        axis_move(axis, negative); // move until limit switch is pressed
        axis_move(axis, positive); // move until limit switch is depressed
        // set this as zero position
        axis->encoder.current = 0;
        return;
    }
}


// -------------------------------- X AXIS SETUP -------------------------------- //
#define ACCEL_X                 100
#define VEL_MIN_X               100
#define VEL_MAX_X               1000
#define STEP_PIN_X              5 // need to know the register for fast operation pin 5 - PC25 
#define DIR_PIN_X               6 // need to know the register for fast operation pin 6 - PC24
#define ENCODER_PIN_A_X         7
#define ENCODER_PIN_B_X         8
#define LIMIT_SW_HOME_PIN_X     9
#define LIMIT_SW_FAR_PIN_X      10

Axis setup_axis_x()
{  
    // Set up the timer interrupt
    // TC1 channel 0, the IRQ for that channel and the desired frequency
    pinMode(ENCODER_PIN_A_X, INPUT);
    pinMode(ENCODER_PIN_B_X, INPUT);
    Encoder encoder_x = {ENCODER_PIN_A_X, 10, 0};

    pinMode(LIMIT_SW_HOME_PIN_X, INPUT_PULLUP);
    LimitSwitch ls_home = {LIMIT_SW_HOME_PIN_X, digitalRead(LIMIT_SW_HOME_PIN_X)};

    pinMode(LIMIT_SW_FAR_PIN_X, INPUT_PULLUP);
    LimitSwitch ls_far_from_home = {LIMIT_SW_FAR_PIN_X, digitalRead(LIMIT_SW_FAR_PIN_X)};
    
    pinMode(DIR_PIN_X, OUTPUT);
    pinMode(STEP_PIN_X, OUTPUT);
    Axis axis_x_temp = {DIR_PIN_X, STEP_PIN_X, ACCEL_X, VEL_MAX_X, VEL_MIN_X, 
                        {0, 0, 0}, 0, encoder_x, ls_home, ls_far_from_home,
                        TC1, 0, 1, TC3_IRQn, TC4_IRQn};
    return axis_x_temp;
}

Axis axis_x = setup_axis_x();

// isr to handle encoder of x axis
void isr_encoder_x()
{
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 200ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 10) {
        axis_x.encoder.current++;
        print("Encoder ISR count: ", axis_x.encoder.current);
    }
    last_interrupt_time = interrupt_time;
}

// isr to handle limit switch
void isr_limit_switch_x()
{
    NVIC_DisableIRQ(axis_x.isr_velocity);
    NVIC_DisableIRQ(axis_x.isr_accel);
    print("X Limit switch hit", 0);
}

// isr for operating x axis motor velocity
void TC3_Handler(void)
{   
    // print("Desired count: ", axis_x.encoder.desired);
    // print("Current count: ", axis_x.encoder.current);
    TC_GetStatus(TC1, 0);
    PIOC->PIO_ODSR ^= PIO_ODSR_P25;
    reset_timer(TC1, 0, TC3_IRQn, axis_x.vel); 
}

// isr for operating x axis motor acceleration 
void TC4_Handler(void)
{   
    // print("Desired count: ", axis_x.encoder.desired);
    // print("Current count: ", axis_x.encoder.current);
    TC_GetStatus(TC1, 1);
    // first accelerate
    if (axis_x.tragectory_segment == 0) {
        if (((axis_x.encoder.current < axis_x.encoder.desired) != axis_x.dir) && axis_x.vel < axis_x.vel_max) {
            axis_x.vel++;
            return;
        }
        else {
            axis_x.tragectory_segment++;
            int delta = axis_x.encoder.desired - axis_x.encoder.current;
            axis_x.encoder.desired = axis_x.encoder.current + axis_x.vel_profile_cur[1] + delta;
            // print("accel d-c: ", delta);
            return;
        }
    }
    // hold velocity
    else if (axis_x.tragectory_segment == 1) {
        if ((axis_x.encoder.current < axis_x.encoder.desired) != axis_x.dir) {
            // print("DC H: ", axis_x.encoder.desired);
            // print("CC H: ", axis_x.encoder.current);
            return;
        }
        else {
            axis_x.tragectory_segment++;
            int delta = axis_x.encoder.desired - axis_x.encoder.current;
            axis_x.encoder.desired = axis_x.encoder.current + axis_x.vel_profile_cur[2] + delta;
            // print("hold d-c: ", delta);
            return;
        }
    }
    // decelerate
    else if (axis_x.tragectory_segment == 2) {
        if (((axis_x.encoder.current < axis_x.encoder.desired) != axis_x.dir) && axis_x.vel > 1) {
            axis_x.vel--;
            // print("DC H: ", axis_x.encoder.desired);
            // print("CC H: ", axis_x.encoder.current);
            return;
        }
        else {
            NVIC_DisableIRQ(TC3_IRQn);
            NVIC_DisableIRQ(TC4_IRQn);
            int delta = axis_x.encoder.desired - axis_x.encoder.current;
            // print("decel d-c: ", delta);
        }
    }
}


// -------------------------------- Y AXIS SETUP -------------------------------- //
#define ACCEL_Y                 100
#define VEL_MIN_Y               100
#define VEL_MAX_Y               1000
#define STEP_PIN_Y              22 // need to know the register for fast operation pin 5 - PB26 
#define DIR_PIN_Y               23 // need to know the register for fast operation pin 6 - PA14
#define ENCODER_PIN_A_Y         24
#define ENCODER_PIN_B_Y         25
#define LIMIT_SW_HOME_PIN_Y     26
#define LIMIT_SW_FAR_PIN_Y      27

Axis setup_axis_y()
{  
    // Set up the timer interrupt
    // TC1 channel 0, the IRQ for that channel and the desired frequency
    pinMode(ENCODER_PIN_A_Y, INPUT);
    pinMode(ENCODER_PIN_B_Y, INPUT);
    Encoder encoder_y = {ENCODER_PIN_A_Y, 0, 0};

    pinMode(LIMIT_SW_HOME_PIN_Y, INPUT_PULLUP);
    LimitSwitch ls_home = {LIMIT_SW_HOME_PIN_Y, digitalRead(LIMIT_SW_HOME_PIN_Y)};

    pinMode(LIMIT_SW_FAR_PIN_Y, INPUT_PULLUP);
    LimitSwitch ls_far_from_home = {LIMIT_SW_FAR_PIN_Y, digitalRead(LIMIT_SW_FAR_PIN_Y)};
    
    pinMode(DIR_PIN_Y, OUTPUT);
    pinMode(STEP_PIN_Y, OUTPUT);
    Axis axis_y_temp = {DIR_PIN_Y, STEP_PIN_Y, ACCEL_Y, VEL_MAX_Y, VEL_MIN_Y, 
                        {0, 0, 0}, 0, encoder_y, ls_home, ls_far_from_home,
                        TC0, 0, 1, TC0_IRQn, TC1_IRQn};
    return axis_y_temp;
}

Axis axis_y = setup_axis_y();

// isr to handle encoder of x axis
void isr_encoder_y()
{
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 10ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 10) {
        axis_y.encoder.current++;
        print("Encoder y ISR count: ", axis_y.encoder.current);
    }
    last_interrupt_time = interrupt_time;
}

// isr to handle limit switch
void isr_limit_switch_y()
{   
    NVIC_DisableIRQ(axis_y.isr_velocity);
    NVIC_DisableIRQ(axis_y.isr_accel);
    print("Y Limit switch hit", 0);
}

// isr for operating x axis motor velocity
void TC0_Handler(void)
{   
    // print("Desired count: ", axis_x.encoder.desired);
    // print("Current count: ", axis_x.encoder.current);
    TC_GetStatus(TC0, 0);
    PIOB->PIO_ODSR ^= PIO_ODSR_P26;
    reset_timer(TC0, 0, TC0_IRQn, axis_y.vel);
}

// isr for operating x axis motor acceleration 
void TC1_Handler(void)
{   
    // print("Desired count: ", axis_x.encoder.desired);
    // print("Current count: ", axis_x.encoder.current);
    TC_GetStatus(TC0, 1);
    // first accelerate
    if (axis_y.tragectory_segment == 0) {
        if (((axis_y.encoder.current < axis_y.encoder.desired) != axis_y.dir) && axis_y.vel < axis_y.vel_max) {
            axis_y.vel++;
            return;
        }
        else {
            axis_y.tragectory_segment++;
            int delta = axis_y.encoder.desired - axis_y.encoder.current;
            axis_y.encoder.desired = axis_y.encoder.current + axis_y.vel_profile_cur[1] + delta;
            // print("accel d-c: ", delta);
            return;
        }
    }
    // hold velocity
    else if (axis_y.tragectory_segment == 1) {
        if ((axis_y.encoder.current < axis_y.encoder.desired) != axis_y.dir) {
            // print("DC H: ", axis_x.encoder.desired);
            // print("CC H: ", axis_x.encoder.current);
            return;
        }
        else {
            axis_y.tragectory_segment++;
            int delta = axis_y.encoder.desired - axis_y.encoder.current;
            axis_y.encoder.desired = axis_y.encoder.current + axis_y.vel_profile_cur[2] + delta;
            // print("hold d-c: ", delta);
            return;
        }
    }
    // decelerate
    else if (axis_y.tragectory_segment == 2) {
        if ((axis_y.encoder.current < axis_y.encoder.desired) != axis_y.dir) {
            axis_y.vel--;
            // print("DC H: ", axis_x.encoder.desired);
            // print("CC H: ", axis_x.encoder.current);
            return;
        }
        else {
            NVIC_DisableIRQ(TC0_IRQn);
            NVIC_DisableIRQ(TC1_IRQn);
            int delta = axis_y.encoder.desired - axis_y.encoder.current;
            // print("decel d-c: ", delta);
        }
    }
}

void setup_encoder_interrupts()
{
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A_X), isr_encoder_x, RISING);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A_Y), isr_encoder_y, RISING);
}

void setup()
{   
    debug();
    // for testing only
    setup_encoder_interrupts();
    axis_trapezoidal_move_rel(&axis_x, 5, 5, 5, negative);
    // axis_trapezoidal_move_rel(&axis_y, 5, 5, 5, positive);
    // constant_max_vel(&axis_x, 5);
}


void loop()
{

}


