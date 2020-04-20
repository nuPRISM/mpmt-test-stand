/**
 * @file Thermistor10k.cpp
 * Thermistor 10k temperature measurements.
 */
#include "Thermistor10k.h"
#include <Arduino.h>

/**
 * @param pin - Arduino Due pin
 */
Thermistor10k::Thermistor10k(uint32_t pin, ThermistorCalibration &calibration) :
    thermistorPin(pin),
    calibration(calibration)
{
}

/**
 * Obtains analog thermistor reading and converts it to temperature in Celsius
 * @return temperature in Celsius
 */
double Thermistor10k::readTemperature()
{
    int vLevel = analogRead(thermistorPin); //defaults to 10 bit if analogReadResolution(12) is not set in main.cpp
    double Tc = convertToTemp(vLevel);
    return Tc;
}

/**
 * Obtains analog thermistor reading and converts it to temperature in Celsius
 * @return temperature in Celsius
 */
double Thermistor10k::readAveragedTemperature(int numSamples)
{
    double averageVLevel = 0.0;

    for(int i = 0; i < numSamples; i++) {
        averageVLevel += analogRead(thermistorPin);
        delay(10);
    }
    averageVLevel /= numSamples;
    double Tc = convertToTemp(averageVLevel);

    return Tc;
}

double Thermistor10k::convertToTemp(int vLevel)
{
    double vTherm = vLevel * (vRef/resolutionLim);
    double RTherm = this->calibration.resistor / (vRef/vTherm - 1.0);
    double logRTherm = log(RTherm);
    double Tk = (1.0 / (this->calibration.c1 + this->calibration.c2*logRTherm + this->calibration.c3*pow(logRTherm,3.0)));

    double Tc = Tk - kelvin_to_celsius;
    return Tc;
}
