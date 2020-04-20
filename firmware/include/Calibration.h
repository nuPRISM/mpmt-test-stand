#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "TemperatureDAQ.h"

#include <stdint.h>

typedef struct {
    uint32_t accel;     //!< acceleration for all motion [steps / s^2]
    uint32_t vel_start; //!< starting velocity for all motion [steps / s]
    uint32_t vel_home;  //!< holding velocity for homing [steps / s]
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