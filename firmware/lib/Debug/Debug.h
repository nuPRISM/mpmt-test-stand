#ifndef DEBUG_H
#define DEBUG_H

// TODO: move to build args in platformio
#define ENABLE_DEBUG_PRINT

#ifdef ENABLE_DEBUG_PRINT

#define DEBUG_SERIAL Serial2
#define DEBUG_BAUD 115200

#define DEBUG_INIT DEBUG_SERIAL.begin(DEBUG_BAUD)

#define DEBUG_PRINT(_text, _val) \
do {                             \
    DEBUG_SERIAL.print(_text);   \
    DEBUG_SERIAL.println(_val);  \
} while (0)

#else // !ENABLE_DEBUG_PRINT

#define DEBUG_INIT
#define DEBUG_PRINT

#endif // !ENABLE_DEBUG_PRINT

#endif // DEBUG_H