#ifndef DEFAULT_CALIBRATION_H
#define DEFAULT_CALIBRATION_H

#include "Calibration.h"

// Host Calibration
const float default_pulley_diameter = 17.0; // mm

// Arduino Calibration
const Calibration default_calibration = {
    .cal_gantry = {
        .accel     = 10, // steps/s^2
        .vel_start = 1,  // steps/s
        .vel_home  = 75, // steps/s
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