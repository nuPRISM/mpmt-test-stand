#ifndef THERMISTOR_10K_H
#define THERMISTOR_10K_H

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
        Thermistor10k(uint32_t pin);

        /**
         * Obtains an analog thermistor reading and converts it to temperature in Celsius
         * @return temperature in Celsius
         */
        double readTemperature();
        /**
         * @param numSamples - number of analog readings to average over
         * Averagesa given number of analog thermistor readings and converts value to 
         * temperature in Celsius
         * @return temperature in Celsius
         */
        double readAveragedTemperature(int numSamples);

    private:
        static constexpr double resolutionLim = 4095.0; // 12-bit Arduino Due
        static constexpr double vRef = 3.3;
        static constexpr double c1 = 0.001127354682, c2 = 0.0002343978227, c3 = 0.00000008674847738; //Steinhart-hart equation constants
        static constexpr double kelvin_to_celsius = 273.15;
        static constexpr double seriesResistorVal = 10000.0; //10k precision resistor

        uint32_t thermistorPin;

        double convertToTemp(int vLevel);
};

#endif // THERMISTOR_10K_H
