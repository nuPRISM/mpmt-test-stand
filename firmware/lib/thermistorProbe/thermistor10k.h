#ifndef THERMISTOR_USP10976_H
#define THERMISTOR_USP10976_H
/*
Thermistor library for USP10976 10k sensors.
https://www.digikey.ca/product-detail/en/littelfuse-inc/USP10976/615-1086-ND/2651604
*/

class Thermistor10k
{
    public:
        /**
         * Constructor to create a new thermistor object.
         * @param pin - Arduino Due pin
         */
        Thermistor10k(uint32_t pin, double seriesResistor);

        /**
         * Obtains analog thermistor reading and converts it to temperature in Celsius
         * @return temperature in Celsius
         */
        double readTemperature();
        /**
         * Obtains five analog thermistor reading averages them and converts it to temperature in Celsius
         * @return temperature in Celsius
         */
        double readAveragedTemperature(int numSamples);
        
    private:
        uint32_t thermistorPin; 
        double seriesResistorVal;       
        // const double resolutionLim = 1023.0; // 10-bit Arduino Uno
        const double resolutionLim = 4094.0; // 12-bit Arduino Due
        const double vRef = 3.3;
        const double c1 = 0.001127354682, c2 = 0.0002343978227, c3 = 0.00000008674847738; //Steinhart-hart equation constants
        const double kelvin_to_celsius = 273.15;
        double convertToTemp(int vLevel);
};

#endif