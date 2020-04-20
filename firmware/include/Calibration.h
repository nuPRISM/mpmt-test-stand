#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "TemperatureDAQ.h"

#include <stdint.h>

typedef struct {
    uint32_t accel;
    uint32_t vel_start;
    uint32_t vel_home;
} GantryCalibration;

typedef struct {
    GantryCalibration cal_gantry;
    TemperatureDAQCalibration cal_temp;
} Calibration;

typedef enum {
    CAL_GANTRY_ACCEL,
    CAL_GANTRY_VEL_START,
    CAL_GANTRY_VEL_HOME,
    CAL_TEMP_ALL_C1,
    CAL_TEMP_ALL_C2,
    CAL_TEMP_ALL_C3,
    CAL_TEMP_ALL_RESISTOR
} CalibrationKey;

#endif // CALIBRATION_H