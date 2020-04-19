#ifndef THERMISTOR_ARRAY_H
#define THERMISTOR_ARRAY_H

#include <TemperatureDAQ.h>
#include <Thermistor10k.h>

typedef struct {
    uint8_t pin_therm_amb;
    uint8_t pin_therm_motor_x;
    uint8_t pin_therm_motor_y;
    uint8_t pin_therm_mpmt;
    uint8_t pin_therm_optical;
} ThermistorArrayIO;

class ThermistorArray
{
    private:
        const ThermistorArrayIO &io;
        Thermistor10k thermistor_ambient;
        Thermistor10k thermistor_motor_x;
        Thermistor10k thermistor_motor_y;
        Thermistor10k thermistor_mpmt;
        Thermistor10k thermistor_optical;

    public:
        ThermistorArray(const ThermistorArrayIO &io, TemperatureDAQCalibration &calibration);
        void setup();
        void read_temp_data(TempData &temp_data);
};

#endif // THERMISTOR_ARRAY_H