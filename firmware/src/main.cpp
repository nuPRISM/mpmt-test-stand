#include "Arduino.h"

#include "Debug.h"
#include "mPMTTestStand.h"

const mPMTTestStandIO io = {
    // Serial Devices
    .serial_comm            = Serial,
    .serial_comm_baud_rate  = 115200,
    // Thermistor Pins
    .pin_therm_amb          = A0,
    .pin_therm_motor1       = A1,
    .pin_therm_mpmt         = A2,
    .pin_therm_motor2       = A3,
    .pin_therm_optical      = A4,
    // Gantry X-Axis Pins
    .pin_motor_x_step       = 5,
    .pin_motor_x_dir        = 6,
    .pin_motor_x_enc_a      = 7,
    .pin_motor_x_enc_b      = 8,
    .pin_motor_x_ls_home    = 9,
    .pin_motor_x_ls_far     = 10,
    // Gantry Y-Axis Pins
    .pin_motor_y_step       = 22,
    .pin_motor_y_dir        = 23,
    .pin_motor_y_enc_a      = 24,
    .pin_motor_y_enc_b      = 25,
    .pin_motor_y_ls_home    = 26,
    .pin_motor_y_ls_far     = 27
};

mPMTTestStand test_stand(io);

uint32_t blink_start;
bool blink_state;

void setup()
{
    DEBUG_INIT;
    pinMode(LED_BUILTIN, OUTPUT);

    test_stand.setup();

    blink_start = millis();
    blink_state = false;
}

void loop()
{
    test_stand.execute();

    // Blink an LED
    if ((millis() - blink_start) >= 500) {
        blink_state = !blink_state;
        digitalWrite(LED_BUILTIN, blink_state);
        blink_start = millis();
    }
}
