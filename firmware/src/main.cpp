#include <Arduino.h>
#include "thermistorUSP10976.h"
// migrate pin assignment to separate header file later
static const int thermistor1Pin = 14; // A0
static const int thermistor2Pin = 15; // A1
static const int thermistor3Pin = 16; // A2

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);
}

void loop()
{
    // put your main code here, to run repeatedly:
    Thermistor_10k thermistor_short_1(thermistor1Pin);
    Thermistor_10k thermistor_short_2(thermistor2Pin);
    Thermistor_10k thermistor_long(thermistor3Pin);
    
    float temp1 = thermistor_short_1.readTemperature();
    float temp2 = thermistor_short_2.readTemperature();
    float temp3 = thermistor_long.readTemperature();

    Serial.print("Temperature 1: "); 
    Serial.print(temp1);
    Serial.println(" C");  
    Serial.print("Temperature 2: "); 
    Serial.print(temp2);
    Serial.println(" C"); 
    Serial.print("Temperature 3: "); 
    Serial.print(temp3);
    Serial.println(" C"); 
    Serial.println("");

    delay(500);
}