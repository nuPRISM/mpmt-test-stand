#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG

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
    DEBUG_PRINT(": ");              \
    DEBUG_PRINTLN(_val);             \
} while (0)

#endif // DEBUG_H