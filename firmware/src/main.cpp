#include <Arduino.h>
static const int thermistor1Pin = 14; // A0
void setup()
{
    // put your setup code here, to run once:
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    // put your main code here, to run repeatedly:
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
}