#ifndef DEFAULT_CALIBRATION_H
#define DEFAULT_CALIBRATION_H

#include "Calibration.h"

// Host Calibration
const float default_pulley_diameter = 17.0; // mm

// Arduino Calibration
const Calibration default_calibration = {
    .cal_gantry = {
        .accel     = 10,  // acceleration for all motion [steps / s^2]
        .vel_start = 1,   // starting velocity for all motion [steps / s]
        .vel_home  = 75,  // holding velocity for homing A [steps / s]
    },
    .cal_temp = {
        .all = {
            .c1       = 0.001127354682,
            .c2       = 0.0002343978227,
            .c3       = 0.00000008674847738,
            .resistor = 10000.0
        }
    }
};

#endif // CAL_H