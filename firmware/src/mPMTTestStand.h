#ifndef MPMT_TEST_STAND_H
#define MPMT_TEST_STAND_H

/* **************************** Local Includes ***************************** */
#include "ArduinoSerialDevice.h"
#include "TestStandCommController.h"
#include "Thermistor10k.h"
#include "Axis.h"
#include "Movement.h"

/* ************************ Shared Project Includes ************************ */
#include "shared_defs.h"

/* **************************** System Includes **************************** */
#include <Arduino.h>

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
    // Gantry Axis Pins
    AxisIO io_axis_x;
    AxisIO io_axis_y;
} mPMTTestStandIO;

typedef struct {
    uint32_t vel_start;
    uint32_t vel_home_a;
    uint32_t vel_home_b;
    uint32_t accel_home_a;
    uint32_t accel_home_b;
    MotionUnits units;
} mPMTTestStandMotionConfig;

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

        mPMTTestStandMotionConfig motion_config;

        Status status;
        bool home_a_done;

        const AxisState *x_state;
        const AxisState *y_state;

        void handle_home();
        void handle_move();
        void handle_stop();
        void handle_get_status();
        void handle_get_data();

        void debug_dump_axis(AxisId axis_id);
        void debug_dump();

    public:
        mPMTTestStand(const mPMTTestStandIO& io, const mPMTTestStandMotionConfig& motion_config);
        void setup();
        void execute();
};

#endif // CMD_HANDLER_H