#include "ThermistorArray.h"

#include <Arduino.h>

ThermistorArray::ThermistorArray(const ThermistorArrayIO &io, TemperatureDAQCalibration &calibration) :
    io(io),
    thermistor_ambient(io.pin_therm_amb    , calibration.cal_ambient),
    thermistor_motor_x(io.pin_therm_motor_x, calibration.cal_motor_x),
    thermistor_motor_y(io.pin_therm_motor_y, calibration.cal_motor_y),
    thermistor_mpmt   (io.pin_therm_mpmt   , calibration.cal_mpmt   ),
    thermistor_optical(io.pin_therm_optical, calibration.cal_optical)
{
}

void ThermistorArray::setup()
{
    analogReadResolution(12); // enable 12 bit resolution mode in Arduino Due. Default is 10 bit.
    pinMode(this->io.pin_therm_amb,     INPUT);
    pinMode(this->io.pin_therm_motor_x, INPUT);
    pinMode(this->io.pin_therm_motor_y, INPUT);
    pinMode(this->io.pin_therm_mpmt,    INPUT);
    pinMode(this->io.pin_therm_optical, INPUT);
}

void ThermistorArray::read_temp_data(TempData &temp_data)
{
    temp_data.temp_ambient = this->thermistor_ambient.readTemperature();
    temp_data.temp_motor_x = this->thermistor_motor_x.readTemperature();
    temp_data.temp_motor_y = this->thermistor_motor_y.readTemperature();
    temp_data.temp_mpmt    = this->thermistor_mpmt.readTemperature();
    temp_data.temp_optical = this->thermistor_optical.readTemperature();
}
