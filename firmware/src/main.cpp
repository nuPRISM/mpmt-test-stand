#include <Arduino.h>
#include "thermistor10k.h"
// migrate pin assignment to separate header file later
static const int thermistor1Pin = A0; 
static const int thermistor2Pin = A1; 
static const int thermistor3Pin = A2; 
static const int thermistor4Pin = A3;
static const int thermistor5Pin = A4;

// data collection variables
unsigned long currentTime = 0.0;
unsigned long deltaTime = 250.0; 

// use of precision resistors removes need to specify resistor values for each thermistor
double seriesResistor = 10000.0;
void setup()
{
    Serial.begin(9600);
    analogReadResolution(12);
}

void loop()
{
    Thermistor10k thermistor_ambient(thermistor1Pin,seriesResistor);
    Thermistor10k thermistor_motor1(thermistor2Pin,seriesResistor);
    Thermistor10k thermistor_mpmt(thermistor3Pin,seriesResistor);
    Thermistor10k thermistor_motor2(thermistor4Pin,seriesResistor);
    Thermistor10k thermistor_optical_box(thermistor5Pin,seriesResistor);
    
    if(millis() - currentTime >= deltaTime){
        currentTime = millis();
        double temp1 = thermistor_ambient.readTemperature();
        double temp2 = thermistor_motor1.readTemperature();
        double temp3 = thermistor_mpmt.readTemperature();
        double temp4 = thermistor_motor2.readTemperature();
        double temp5 = thermistor_optical_box.readTemperature();

        // double temp1 = thermistor_25cm.readAveragedTemperature(5);
        // double temp2 = thermistor_50cm.readAveragedTemperature(5);
        // double temp3 = thermistor_200cm.readAveragedTemperature(5);
        
        Serial.print(currentTime/1000.0);
        Serial.print(",");
        Serial.print(temp1);
        Serial.print(",");
        Serial.print(temp2);
        Serial.print(",");
        Serial.print(temp3);
        Serial.print(",");
        Serial.print(temp4);
        Serial.print(",");
        Serial.println(temp5);
    }
}
