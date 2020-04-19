#ifndef DEFAULT_CALIBRATION_H
#define DEFAULT_CALIBRATION_H

#include "Calibration.h"

const ThermistorCalibration default_thermistor_calibration = {
    .c1              = 0.001127354682,
    .c2              = 0.0002343978227,
    .c3              = 0.00000008674847738,
    .series_resistor = 10000.0
};

const Calibration default_calibration = {
    .cal_gantry = {
        .accel                  = 10,  // acceleration for all motion [steps / s^2]
        .vel_start              = 1,   // starting velocity for all motion [steps / s]
        .vel_home_a             = 75,  // holding velocity for homing A [steps / s]
        .vel_home_b             = 75,  // holding velocity for homing B [steps / s]
        .accel_home_a           = 10,  // acceleration for homing A [steps / s^2]
        .accel_home_b           = 10   // acceleration for homing B [steps / s^2]
    },
    .cal_temp = {
        .cal_ambient = default_thermistor_calibration,
        .cal_motor_x = default_thermistor_calibration,
        .cal_motor_y = default_thermistor_calibration,
        .cal_mpmt    = default_thermistor_calibration,
        .cal_optical = default_thermistor_calibration
    }
};

#endif // CAL_H