#ifndef ARDUINO_HELPER_H
#define ARDUINO_HELPER_H

#include "TemperatureDAQ.h"

#include "midas.h"

bool arduino_connect(char *device_file);
void arduino_disconnect();

bool arduino_move(float *dest_mm, float *vel_mm_s);
bool arduino_run_home();
bool arduino_stop();

bool arduino_get_status(DWORD *status_out);
bool arduino_get_position(float *gantry_x_mm_out, float *gantry_y_mm_out);
bool arduino_get_temp(TempData *temp_out);

#endif // ARDUINO_HELPER_H
