#include "Axis.h"

// -------------------------------- X AXIS SETUP -------------------------------- //
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
        if (axis_x.dir == positive) axis_x.encoder.current++;
        else if (axis_x.dir == negative) axis_x.encoder.current--;
        print("Encoder ISR count: ", axis_x.encoder.current);
    }
    last_interrupt_time = interrupt_time;
}

// isr to handle limit switch
void isr_limit_switch_x()
{   
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 10ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 10) {
        NVIC_DisableIRQ(axis_x.isr_velocity);
        NVIC_DisableIRQ(axis_x.isr_accel);
        print("X Home Limit switch has been hit", 1);

        if (axis_x.homing && digitalRead(LIMIT_SW_HOME_PIN_X) == depressed) {
            axis_x.encoder.current = 0;
            axis_x.homing = 0;
            print("X axis has been homed - SUCCESS", 1);
        }
        else if (axis_x.homing && digitalRead(LIMIT_SW_HOME_PIN_X) == pressed) {
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
    if (axis_x.tragectory_segment == accelerate) {
        if (((axis_x.encoder.current < axis_x.encoder.desired) != axis_x.dir) && axis_x.vel < axis_x.vel_max) {
            axis_x.vel++;
            return;
        }
        else {
            axis_x.tragectory_segment++;
            int delta = axis_x.encoder.desired - axis_x.encoder.current;
            axis_x.encoder.desired = axis_x.encoder.current + axis_x.vel_profile_cur_trap[1] + delta;
            // print("accel d-c: ", delta);
            return;
        }
    }
    // hold velocity
    else if (axis_x.tragectory_segment == hold) {
        if ((axis_x.encoder.current < axis_x.encoder.desired) != axis_x.dir) {
            // print("DC H: ", axis_x.encoder.desired);
            // print("CC H: ", axis_x.encoder.current);
            return;
        }
        else {
            axis_x.tragectory_segment++;
            int delta = axis_x.encoder.desired - axis_x.encoder.current;
            axis_x.encoder.desired = axis_x.encoder.current + axis_x.vel_profile_cur_trap[2] + delta;
            // print("hold d-c: ", delta);
            return;
        }
    }
    // decelerate
    else if (axis_x.tragectory_segment == decelerate) {
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
    // If interrupts come faster than 200ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 10) {
        if (axis_y.dir == positive) axis_y.encoder.current++;
        else if (axis_y.dir == negative) axis_y.encoder.current--;
        print("Encoder ISR count: ", axis_y.encoder.current);
    }
    last_interrupt_time = interrupt_time;
}

// isr to handle limit switch
void isr_limit_switch_y()
{   
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 10ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 10) {
        NVIC_DisableIRQ(axis_y.isr_velocity);
        NVIC_DisableIRQ(axis_y.isr_accel);
        print("Y Home Limit switch has been hit", 1);

        if (axis_y.homing && digitalRead(LIMIT_SW_HOME_PIN_Y) == depressed) {
            axis_y.encoder.current = 0;
            axis_y.homing = 0;
            print("Y axis has been homed - SUCCESS", 1);
        }
        else if (axis_y.homing && digitalRead(LIMIT_SW_HOME_PIN_Y) == pressed) {
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
    if (axis_y.tragectory_segment == accelerate) {
        if (((axis_y.encoder.current < axis_y.encoder.desired) != axis_y.dir) && axis_y.vel < axis_y.vel_max) {
            axis_y.vel++;
            return;
        }
        else {
            axis_y.tragectory_segment++;
            int delta = axis_y.encoder.desired - axis_y.encoder.current;
            axis_y.encoder.desired = axis_y.encoder.current + axis_y.vel_profile_cur_trap[1] + delta;
            // print("accel d-c: ", delta);
            return;
        }
    }
    // hold velocity
    else if (axis_y.tragectory_segment == hold) {
        if ((axis_y.encoder.current < axis_y.encoder.desired) != axis_y.dir) {
            // print("DC H: ", axis_x.encoder.desired);
            // print("CC H: ", axis_x.encoder.current);
            return;
        }
        else {
            axis_y.tragectory_segment++;
            int delta = axis_y.encoder.desired - axis_y.encoder.current;
            axis_y.encoder.desired = axis_y.encoder.current + axis_y.vel_profile_cur_trap[2] + delta;
            // print("hold d-c: ", delta);
            return;
        }
    }
    // decelerate
    else if (axis_y.tragectory_segment == decelerate) {
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

void setup_ls_interrupts()
{
    attachInterrupt(digitalPinToInterrupt(LIMIT_SW_HOME_PIN_X), isr_limit_switch_x, CHANGE);
    attachInterrupt(digitalPinToInterrupt(LIMIT_SW_HOME_PIN_Y), isr_limit_switch_y, CHANGE);
    attachInterrupt(digitalPinToInterrupt(LIMIT_SW_FAR_PIN_X), isr_limit_switch_x, CHANGE);
    attachInterrupt(digitalPinToInterrupt(LIMIT_SW_FAR_PIN_Y), isr_limit_switch_y, CHANGE);
}