#include "Debug.h"
// ++++++++++++++++++++++++ debug helper functions start
void debug() {
    if (DEBUG) {Serial.begin(BAUDRATE);}
}

void print(String text, uint32_t val)
{   
    if (DEBUG) {
        // Serial.print("DEBUG     ");
        Serial.print(text);
        Serial.println(val);
    }
}
// ------------------------ debug helper functions end