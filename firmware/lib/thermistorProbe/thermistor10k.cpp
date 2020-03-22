/**
 * @file thermistor10k.cpp
 * Thermistor 10k temperature measurements.
 */
#include <Arduino.h>
#include "thermistor10k.h"

/**
 * @param pin - Arduino Due pin
 */
Thermistor10k::Thermistor10k(uint32_t pin, double seriesResistor)
{
    this->thermistorPin = pin;
    this->seriesResistorVal = seriesResistor; // measure actual resistance of the 20 kOhm series resistor
}
/**
 * Obtains analog thermistor reading and converts it to temperature in Celsius
 * @return temperature in Celsius
 */
double Thermistor10k::readTemperature()
{
    int vLevel = analogRead(thermistorPin); //defaults to 10 bit if analogReadResolution(12) is not set
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
    int vLevelSamples[numSamples];

    for(int i = 0; i < numSamples; i++){
        vLevelSamples[i] = analogRead(thermistorPin);
        delay(10);
    }

    for(int i = 0; i < numSamples; i++){
        averageVLevel += vLevelSamples[i];
    }
    averageVLevel /= numSamples;
    double Tc = convertToTemp(averageVLevel);

    return Tc;
}

double Thermistor10k::convertToTemp(int vLevel)
{
    double vTherm = vLevel * (vRef/resolutionLim);
    double RTherm = seriesResistorVal / (vRef/vTherm - 1.0);
    double logRTherm = log(RTherm);
    double Tk = (1.0 / (c1 + c2*logRTherm + c3*pow(logRTherm,3.0)));
    
    double Tc = Tk - kelvin_to_celsius;
    return Tc;
}

