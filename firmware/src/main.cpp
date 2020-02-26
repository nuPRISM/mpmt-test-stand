#include <Arduino.h>
#include "thermistorUSP10976.h"
// migrate pin assignment to separate header file later
static const int thermistor1Pin = A0; 
static const int thermistor2Pin = A1; 
static const int thermistor3Pin = A2; 

// data collection variables
unsigned long currentTime = 0.0;
unsigned long deltaTime = 250.0; 

double seriesResistor1 = 9960.0;
double seriesResistor2 = 9960.0;
double seriesResistor3 = 9950.0;

void setup()
{
    //Ground unused analog pins to minimize interference
    for (int i = A3; i <= A6; i++){
        pinMode(i, OUTPUT); 
        digitalWrite(i, LOW);
    }
    
    Serial.begin(9600);
}

void loop()
{
    File dataFile = SD.open("temp_datalog.txt", FILE_WRITE);
    
    Thermistor_10k thermistor_25cm(thermistor1Pin,seriesResistor1);
    Thermistor_10k thermistor_50cm(thermistor2Pin,seriesResistor2);
    Thermistor_10k thermistor_200cm(thermistor3Pin,seriesResistor3);
    
    if(millis() - currentTime >= deltaTime){
        currentTime = millis();
        double temp1 = thermistor_25cm.readTemperature();
        double temp2 = thermistor_50cm.readTemperature();
        double temp3 = thermistor_200cm.readTemperature();

        // double temp1 = thermistor_25cm.readAveragedTemperature();
        // double temp2 = thermistor_50cm.readAveragedTemperature();
        // double temp3 = thermistor_200cm.readAveragedTemperature();
        Serial.print(currentTime/1000.0);
        Serial.print(",");
        Serial.print(temp1);
        Serial.print(",");
        Serial.print(temp2);
        Serial.print(",");
        Serial.println(temp3);
    }
}
