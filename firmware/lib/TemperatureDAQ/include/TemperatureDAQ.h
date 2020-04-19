#ifndef TEMPERATURE_DAQ_H
#define TEMPERATURE_DAQ_H

#include <stdint.h>

const int32_t temp_data_scaler = 1E6;

typedef struct {
    double temp_ambient;
    double temp_motor_x;
    double temp_motor_y;
    double temp_mpmt;
    double temp_optical;
} TempData;

typedef struct {
    double c1;              //!< Steinhart-hart equation constant
    double c2;              //!< Steinhart-hart equation constant
    double c3;              //!< Steinhart-hart equation constant
    double series_resistor; //!< Resistance of series resistor
} ThermistorCalibration;

typedef struct {
    ThermistorCalibration cal_ambient;
    ThermistorCalibration cal_motor_x;
    ThermistorCalibration cal_motor_y;
    ThermistorCalibration cal_mpmt;
    ThermistorCalibration cal_optical;
} TemperatureDAQCalibration;

#endif // TEMPERATURE_DAQ_H