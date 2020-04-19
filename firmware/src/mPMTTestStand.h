#ifndef MPMT_TEST_STAND_H
#define MPMT_TEST_STAND_H

/* **************************** Local Includes ***************************** */
// Serial Communication
#include "ArduinoSerialDevice.h"
#include "TestStandCommController.h"
// Gantry
#include "Gantry.h"
#include "Axis.h"
// Temperature DAQ
#include "ThermistorArray.h"
// Other
#include "Calibration.h"

/* ************************ Shared Project Includes ************************ */
#include "shared_defs.h"

/* **************************** System Includes **************************** */
#include <Arduino.h>

typedef struct {
    // Serial Devices
    UARTClass &serial_comm;
    SerialBaudRate serial_comm_baud_rate;
    // Gantry Axes
    AxisIO io_axis_x;
    AxisIO io_axis_y;
    AxisMech axis_mech;
    // Temperature Measurement
    ThermistorArrayIO io_temp;
} mPMTTestStandConfig;

class mPMTTestStand
{
    private:
        const mPMTTestStandConfig &conf;
        Calibration cal;

        ArduinoSerialDevice comm_dev;
        TestStandCommController comm;

        ThermistorArray thermistors;

        Status status;
        bool home_a_done;

        const AxisState *x_state;
        const AxisState *y_state;

        void handle_echo();
        void handle_home_a();
        void handle_home_b();
        void handle_move();
        void handle_stop();
        void handle_get_status();
        void handle_get_position();
        void handle_get_axis_state();
        void handle_get_temp();

#ifdef DEBUG
        void debug_dump_axis(AxisId axis_id);
        void debug_dump();
#endif // DEBUG

    public:
        mPMTTestStand(const mPMTTestStandConfig &conf, Calibration cal);
        void setup();
        void execute();
};

#endif // CMD_HANDLER_H