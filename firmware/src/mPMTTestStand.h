#ifndef MPMT_TEST_STAND_H
#define MPMT_TEST_STAND_H

#include <Arduino.h>

#include "ArduinoSerialDevice.h"
#include "TestStandCommController.h"
#include "Thermistor10k.h"

#include "shared_defs.h"

typedef struct {
    // Serial Devices
    UARTClass& serial_comm;
    uint32_t serial_comm_baud_rate;
    // Thermistor Pins
    uint8_t pin_therm_amb;
    uint8_t pin_therm_motor1;
    uint8_t pin_therm_mpmt;
    uint8_t pin_therm_motor2;
    uint8_t pin_therm_optical;
    // Gantry X-Axis Pins
    uint8_t pin_motor_x_step;
    uint8_t pin_motor_x_dir;
    uint8_t pin_motor_x_enc_a;
    uint8_t pin_motor_x_enc_b;
    uint8_t pin_motor_x_ls_home;
    uint8_t pin_motor_x_ls_far;
    // Gantry Y-Axis Pins
    uint8_t pin_motor_y_step;
    uint8_t pin_motor_y_dir;
    uint8_t pin_motor_y_enc_a;
    uint8_t pin_motor_y_enc_b;
    uint8_t pin_motor_y_ls_home;
    uint8_t pin_motor_y_ls_far;
} mPMTTestStandIO;

class mPMTTestStand
{
    private:
        const mPMTTestStandIO& io;
        ArduinoSerialDevice comm_dev;
        TestStandCommController comm;
        Thermistor10k thermistor_ambient;
        Thermistor10k thermistor_motor1;
        Thermistor10k thermistor_mpmt;
        Thermistor10k thermistor_motor2;
        Thermistor10k thermistor_optical;

        Status status;

        void handle_home();
        void handle_move();
        void handle_stop();
        void handle_get_status();
        void handle_get_data();

    public:
        mPMTTestStand(const mPMTTestStandIO& io);
        void setup();
        void execute();
};

#endif // CMD_HANDLER_H