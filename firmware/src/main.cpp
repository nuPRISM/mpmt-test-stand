#include <Arduino.h>
#include "thermistorUSP10976.h"
// migrate pin assignment to separate header file later
static const int thermistor1Pin = 14; // A0

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);
}

void loop()
{
    // put your main code here, to run repeatedly:
    Thermistor_10k thermistor1;
    float temp1 = thermistor1.readTemperature(thermistor1Pin);

    Serial.print("Temperature 1: "); 
    Serial.print(temp1);
    Serial.println(" C");  

    delay(500);
}