#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "TemperatureDAQ.h"

#include <stdint.h>

typedef struct {
    uint32_t accel;
    uint32_t vel_start;
    uint32_t vel_home_a;
    uint32_t vel_home_b;
    uint32_t accel_home_a;
    uint32_t accel_home_b;
} GantryCalibration;

typedef struct {
    GantryCalibration cal_gantry;
    TemperatureDAQCalibration cal_temp;
} Calibration;

#endif // CALIBRATION_H