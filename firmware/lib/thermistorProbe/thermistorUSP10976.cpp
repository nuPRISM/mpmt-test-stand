/**
 * @file thermistorUSP10976.cpp
 * Thermistor 10k temperature measurements.
 */
#include <Arduino.h>
#include "thermistorUSP10976.h"

/**
 * 
 * @param pin - Arduino Due pin
 */
Thermistor_10k::Thermistor_10k(int pin, double seriesResistor){
    this->thermistorPin = pin;
    this->seriesResistorVal = seriesResistor; // measure actual resistance of the 20 kOhm series resistor
}
/**
 * Obtains analog thermistor reading and converts it to temperature in Celsius
 * @return temperature in Celsius
 */
double Thermistor_10k::readTemperature(){
    vLevel = analogRead(thermistorPin);
    Tc = convertToTemp(vLevel);
    return Tc;
}

/**
 * Obtains analog thermistor reading and converts it to temperature in Celsius
 * @return temperature in Celsius
 */
double Thermistor_10k::readAveragedTemperature(){
    double averageVLevel = 0.0;
    for(int i = 0; i < numSamples; i++){
        vLevelSamples[i] = analogRead(thermistorPin);
        delay(10);
    }

    for(int i = 0; i < numSamples; i++){
        averageVLevel += vLevelSamples[i];
    }
    averageVLevel /= numSamples;
    Tc = convertToTemp(averageVLevel);

    return Tc;
}

double Thermistor_10k::convertToTemp(int vLevel){
    vTherm = vLevel * (vRef/resolutionLim);
    RTherm = seriesResistorVal / (vRef/vTherm - 1.0);
    logRTherm = log(RTherm);
    Tk = (1.0 / (c1 + c2*logRTherm + c3*pow(logRTherm,3.0)));
    
    Tc = Tk - kelvin_to_celsius;
    return Tc;
}

