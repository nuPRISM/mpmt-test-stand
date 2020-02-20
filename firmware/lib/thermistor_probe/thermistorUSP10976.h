#pragma once
#ifndef THERMISTOR_USP10976_H
#define THERMISTOR_USP10976_H

class Thermistor_10k
{
    public:
        const float resolutionLim = 1023.0; // 12-bit, (4095-1) for Arduino Due
        const float seriesResistor = 20000.0; // 20kOhm
        const float vRef = 5.0;
        const float c1 = 0.001127354682, c2 = 0.0002343978227, c3 = 0.00000008674847738; //Steinhart-hart equation constants
        const float kelvin_to_celsius = 273.15;

        int vLevel;
        float vTherm;
        float logRTherm,RTherm, Tk, Tc;

        /**
         * Constructor to create a new thermistor object.
         * @param pin - Arduino Due pin
         */
        Thermistor_10k(int pin);

        /**
         * Obtains analog thermistor reading and converts it to temperature in Celsius
         * @return temperature in Celsius
         */
        float readTemperature();
    
    private:
        int thermistorPin;
};

#endif