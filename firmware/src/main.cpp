#include "Arduino.h"
#define DEBUG   1
void debug() {
    if (DEBUG) {Serial.begin(115200);}
}

void print(String text, uint32_t val)
{   
    Serial.print("DEBUG     ");
    Serial.print(text);
    Serial.println(val);
}

enum direction{positive, negative};
enum status{pressed, depressed};

struct LimitSwitch
{
    int pin;
    int status;
};

struct Encoder
{
    uint32_t pin;
    volatile uint32_t current;
    uint32_t desired;
};

struct Axis
{
    int dir_pin;
    int step_pin;
    int accel;
    int vel_max;
    struct Encoder encoder;
    struct LimitSwitch ls_home;
    struct LimitSwitch ls_far_from_home;
};

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

void accelerate(struct Axis *axis, uint32_t counts)
{

}

void decelerate(struct Axis *axis, uint32_t counts)
{

}

void constant_max_vel(struct Axis *axis, uint32_t counts)
{   
    axis->encoder.desired = axis->encoder.current + counts;
    // print("Desired count: ", axis->encoder.desired);
    // print("Current count: ", axis->encoder.current);
    start_timer(TC1, 0, TC3_IRQn, axis->vel_max);
}


void axis_move_rel(struct Axis *axis, uint32_t counts_accel, uint32_t counts_const, uint32_t counts_decel)
{

}

void axis_move_abs(struct Axis *axis, uint32_t counts_accel, uint32_t counts_const, uint32_t counts_decel)
{

}

void axis_move(struct Axis *axis, direction direction) 
{

}

void home_axis(struct Axis *axis)
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

#define ACCEL_X             20000
#define VEL_MAX_X           100
#define STEP_PIN_X          5 // need to know the register for fast operation pin 5 - PC25 
#define DIR_PIN_X           6 // need to know the register for fast operation pin 6 - PC24
#define ENCODER_PIN_X       7
#define LIMIT_SW_HOME_PIN   8
#define LIMIT_SW_FAR_PIN    9

struct Axis setup_axis_x()
{  
    // Set up the timer interrupt
    // TC1 channel 0, the IRQ for that channel and the desired frequency
    pinMode(ENCODER_PIN_X, INPUT);
    struct Encoder encoder_x = {ENCODER_PIN_X, 0, 0};

    pinMode(LIMIT_SW_HOME_PIN, INPUT_PULLUP);
    struct LimitSwitch ls_home = {LIMIT_SW_HOME_PIN, digitalRead(LIMIT_SW_HOME_PIN)};

    pinMode(LIMIT_SW_FAR_PIN, INPUT_PULLUP);
    struct LimitSwitch ls_far_from_home = {LIMIT_SW_FAR_PIN, digitalRead(LIMIT_SW_FAR_PIN)};
    
    pinMode(DIR_PIN_X, OUTPUT);
    pinMode(STEP_PIN_X, OUTPUT);
    struct Axis axis_x_temp = {DIR_PIN_X, STEP_PIN_X, ACCEL_X, VEL_MAX_X, encoder_x, ls_home, ls_far_from_home};
    return axis_x_temp;
}

struct Axis axis_x = setup_axis_x();

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

void setup_encoder_interrupts()
{
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_X), isr_encoder_x, RISING);
}
// isr for operating x axis motor
void TC3_Handler(void)
{   
    // print("Desired count: ", axis_x.encoder.desired);
    // print("Current count: ", axis_x.encoder.current);
    TC_GetStatus(TC1, 0);
    if (axis_x.encoder.current < axis_x.encoder.desired) {
        PIOC->PIO_ODSR ^= PIO_ODSR_P25;
        reset_timer(TC1, 0, TC3_IRQn, axis_x.vel_max);
    }
    else
    {   
        NVIC_DisableIRQ(TC3_IRQn);
    }
    
}

void setup()
{   
    debug();
    // for testing only
    setup_encoder_interrupts();
    constant_max_vel(&axis_x, 5);
}


void loop()
{

}


