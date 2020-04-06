#ifndef ARDUINO_HELPER_H
#define ARDUINO_HELPER_H

bool arduino_connect(char *device_file);
void arduino_disconnect();

void arduino_attempt_move(float *dest_mm, float *vel_mm_s);
void arduino_run_home();

#endif // ARDUINO_HELPER_H
