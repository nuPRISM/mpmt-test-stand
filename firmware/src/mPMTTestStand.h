#ifndef MPMT_TEST_STAND_H
#define MPMT_TEST_STAND_H

/* **************************** Local Includes ***************************** */
#include "ArduinoSerialDevice.h"
#include "TestStandCommController.h"
#include "Thermistor10k.h"
#include "Gantry.h"
#include "Axis.h"

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
    uint8_t pin_therm_motor_x;
    uint8_t pin_therm_motor_y;
    uint8_t pin_therm_mpmt;
    uint8_t pin_therm_optical;
    // Gantry Axis Pins
    AxisIO io_axis_x;
    AxisIO io_axis_y;
} mPMTTestStandIOConfig;

typedef struct {
    AxisMech axis_mech;
    uint32_t accel;
    uint32_t vel_start;
    uint32_t vel_home_a;
    uint32_t vel_home_b;
    uint32_t accel_home_a;
    uint32_t accel_home_b;
} mPMTTestStandGantryConfig;

typedef struct {
    mPMTTestStandIOConfig io;
    mPMTTestStandGantryConfig gantry;
} mPMTTestStandConfig;

class mPMTTestStand
{
    private:
        const mPMTTestStandConfig& conf;

        ArduinoSerialDevice comm_dev;
        TestStandCommController comm;
        Thermistor10k thermistor_ambient;
        Thermistor10k thermistor_motor_x;
        Thermistor10k thermistor_motor_y;
        Thermistor10k thermistor_mpmt;
        Thermistor10k thermistor_optical;

        Status status;
        bool home_a_done;

        const AxisState *x_state;
        const AxisState *y_state;

        void handle_home_a();
        void handle_home_b();
        void handle_move();
        void handle_stop();
        void handle_get_status();
        void handle_get_position();
        void handle_get_axis_state();
        void handle_get_temp();

        void debug_dump_axis(AxisId axis_id);
        void debug_dump();

    public:
        mPMTTestStand(const mPMTTestStandConfig& conf);
        void setup();
        void execute();
};

#endif // CMD_HANDLER_H