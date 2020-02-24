#ifndef DEBUG_H
#define DEBUG_H

#include "Arduino.h"

#define DEBUG   1
#define BAUDRATE 250000

void debug();

void print(String text, uint32_t val);

#endif