#ifndef GANTRY_H
#define GANTRY_H

typedef enum {
    AXIS_DIR_POSITIVE,
    AXIS_DIR_NEGATIVE
} AxisDirection;

typedef enum {
    AXIS_X,
    AXIS_Y
} AxisId;

/**
 * @enum AxisResult
 * 
 * @brief Possible return values for a call to axis_start
 */
typedef enum {
    AXIS_OK,                  //!< Axis movement started OK
    AXIS_ERR_ALREADY_MOVING,  //!< Axis is already moving
    AXIS_ERR_LS_HOME,         //!< Trying to move backward while HOME limit switch is pressed
    AXIS_ERR_LS_FAR,          //!< Trying to move forward while FAR limit switch is pressed
    AXIS_ERR_INVALID          //!< The parameters resulted in an invalid motion profile
} AxisResult;

#endif // GANTRY_H