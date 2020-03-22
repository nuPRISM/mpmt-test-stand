#ifndef SHARED_DEFS_H
#define SHARED_DEFS_H

typedef enum {
    DIR_POSITIVE,
    DIR_NEGATIVE
} Direction;

typedef enum {
    AXIS_X,
    AXIS_Y
} AxisId;

typedef enum {
    DATA_TEMP,
    DATA_MOTOR
} DataId;

typedef enum {
    LL_DEBUG,
    LL_INFO,
    LL_WARNING,
    LL_ERROR,
    LL_CRITICAL
} LogLevel;

typedef enum {
    STATUS_IDLE,
    STATUS_HOMING,
    STATUS_MOVING,
    STATUS_MEASURING,
    STATUS_LIMIT_REACHED
} Status;

#endif // SHARED_DEFS_H