#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

// #define DEBUG

#ifdef DEBUG

#define DEBUG_SERIAL Serial2
#define DEBUG_BAUD 115200

#define DEBUG_INIT        DEBUG_SERIAL.begin(DEBUG_BAUD)
#define DEBUG_PRINT(_s)   DEBUG_SERIAL.print(_s)
#define DEBUG_PRINTLN(_s) DEBUG_SERIAL.println(_s)

#else // !DEBUG

#define DEBUG_INIT
#define DEBUG_PRINT(_s)
#define DEBUG_PRINTLN(_s)

#endif // !DEBUG

#define DEBUG_PRINT_VAL(_name, _val) \
do {                                 \
    DEBUG_PRINT(_name);              \
    DEBUG_PRINT(": ");               \
    DEBUG_PRINTLN(_val);             \
} while (0)

#define PROFILE(_name, _thresh_us, _x)         \
do {                                           \
    uint32_t start = micros();                 \
    _x;                                        \
    uint32_t delta = micros() - start;         \
    if (delta < (_thresh_us)) break;           \
    DEBUG_PRINT(_name); DEBUG_PRINT(": ");     \
    DEBUG_PRINT(delta); DEBUG_PRINTLN(" us");  \
} while(0)

#define PERIODIC(_x, _t)               \
do {                                   \
    static uint32_t last_time = 0;     \
    if (millis() - last_time > (_t)) { \
        _x;                            \
        last_time = millis();          \
    }                                  \
} while(0);

#endif // DEBUG_H