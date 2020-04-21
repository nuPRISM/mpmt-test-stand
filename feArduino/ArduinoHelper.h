#ifndef ARDUINO_HELPER_H
#define ARDUINO_HELPER_H

#include "TemperatureDAQ.h"
#include "Calibration.h"

#include "midas.h"

int32_t mm_to_cts(float val_mm);
float cts_to_mm(int32_t val_cts);
uint32_t mm_to_steps(float val_mm);
float steps_to_mm(uint32_t val_steps);

bool arduino_connect(char *device_file);
void arduino_disconnect();

bool arduino_move(float *dest_mm, float *vel_mm_s);
bool arduino_run_home();
bool arduino_stop();

bool arduino_get_status(DWORD *status_out);
bool arduino_get_position(float *gantry_x_mm_out, float *gantry_y_mm_out);
bool arduino_get_temp(TempData *temp_out);

bool arduino_calibrate(Calibration *calibration);

#endif // ARDUINO_HELPER_H
