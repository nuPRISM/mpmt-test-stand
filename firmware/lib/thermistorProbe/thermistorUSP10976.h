#pragma once
#ifndef THERMISTOR_USP10976_H
#define THERMISTOR_USP10976_H

class Thermistor_10k
{
    public:
        // const double resolutionLim = 1023.0; // 10-bit Arduino Uno
        const double resolutionLim = 4094.0; // 12-bit Arduino Due
        // const float seriesResistor = 10000.0; 
        const double vRef = 3.3;
        const double c1 = 0.001127354682, c2 = 0.0002343978227, c3 = 0.00000008674847738; //Steinhart-hart equation constants
        const double kelvin_to_celsius = 273.15;
        static const int numSamples = 5;

        int vLevel;
        double seriesResistorVal;
        double vTherm;
        double logRTherm,RTherm, Tk, Tc;
        
        int vLevelSamples[numSamples];
        /**
         * Constructor to create a new thermistor object.
         * @param pin - Arduino Due pin
         */
        Thermistor_10k(int pin, double seriesResistor);

        /**
         * Obtains analog thermistor reading and converts it to temperature in Celsius
         * @return temperature in Celsius
         */
        double readTemperature();
        /**
         * Obtains five analog thermistor reading averages them and converts it to temperature in Celsius
         * @return temperature in Celsius
         */
        double readAveragedTemperature();
        
    private:
        int thermistorPin;
        double convertToTemp(int vLevel);
};

#endif