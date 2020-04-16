#ifndef SHARED_DEFS_H
#define SHARED_DEFS_H

#define SERIAL_BAUD_RATE       BAUD_115200

#define MSG_RECEIVE_TIMEOUT_MS 250

#define ENCODER_COUNTS_PER_REV 500
#define MOTOR_STEPS_PER_REV    800

typedef enum {
    STATUS_IDLE,
    STATUS_HOMING,
    STATUS_MOVING,
    STATUS_FAULT
} Status;

#endif // SHARED_DEFS_H