#ifndef MPMT_TEST_STAND_H
#define MPMT_TEST_STAND_H

#include <Arduino.h>

#include "ArduinoSerialDevice.h"
#include "TestStandCommController.h"
#include "Thermistor10k.h"
#include "Axis.h"

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
    // Gantry Axis Pins
    AxisPins pins_axis_x;
    AxisPins pins_axis_y;
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