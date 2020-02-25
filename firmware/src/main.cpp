#include <Arduino.h>
#include "thermistorUSP10976.h"
// migrate pin assignment to separate header file later
static const int thermistor1Pin = 14; // A0
static const int thermistor2Pin = 15; // A1
static const int thermistor3Pin = 16; // A2

float seriesResistor1 = 20000;
float seriesResistor2 = 20000;
float seriesResistor3 = 20000;

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);
}

void loop()
{
    // put your main code here, to run repeatedly:
    Thermistor_10k thermistor_short_1(thermistor1Pin,seriesResistor1);
    Thermistor_10k thermistor_short_2(thermistor2Pin,seriesResistor2);
    Thermistor_10k thermistor_long(thermistor3Pin,seriesResistor3);
    
    float temp1 = thermistor_short_1.readTemperature();
    float temp2 = thermistor_short_2.readTemperature();
    float temp3 = thermistor_long.readTemperature();
    
    Serial.print(temp1);
    Serial.print(",");
    Serial.print(temp2);
    Serial.print(",");
    Serial.println(temp3);

    delay(500);
}