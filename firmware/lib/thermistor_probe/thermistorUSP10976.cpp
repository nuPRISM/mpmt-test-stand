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
Thermistor_10k::Thermistor_10k(int pin){
    this->thermistorPin = pin;
}
/**
 * Obtains analog thermistor reading and converts it to temperature in Celsius
 * @return temperature in Celsius
 */
float Thermistor_10k::readTemperature(){
    vLevel = analogRead(thermistorPin);
    vTherm = vLevel * (vRef/resolutionLim);
    RTherm = seriesResistor / (vRef/vTherm - 1.0);
    logRTherm = log(RTherm);
    Tk = (1.0 / (c1 + c2*logRTherm + c3*pow(logRTherm,3.0)));
    
    Tc = Tk - kelvin_to_celsius;
    return Tc;
}

