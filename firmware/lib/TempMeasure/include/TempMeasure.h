#ifndef TEMP_MEASURE_H
#define TEMP_MEASURE_H

#include <stdint.h>

const int32_t temp_data_scaler = 1E6;

typedef struct {
    double temp_ambient;
    double temp_motor_x;
    double temp_motor_y;
    double temp_mpmt;
    double temp_optical;
} TempData;

#endif // TEMP_MEASURE_H