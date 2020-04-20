#ifndef FE_ARDUINO_H
#define FE_ARDUINO_H

#define ODB_PATH_EQS                       "/Equipment"

// Equipment Name
#define EQ_ARDUINO                         "ARDUINO"

// Paths
#define ODB_PATH_ARDUINO                   ODB_PATH_EQS "/" EQ_ARDUINO
#define ODB_PATH_ARDUINO_SETTINGS          ODB_PATH_ARDUINO "/Settings"
#define ODB_PATH_ARDUINO_VARIABLES         ODB_PATH_ARDUINO "/Variables"

// Variable Banks (must be 4 letters)
#define ODB_BANK_ARDUINO_STATUS            "STAT"
#define ODB_BANK_ARDUINO_GANTRY            "GANT"
#define ODB_BANK_ARDUINO_TEMP              "TEMP"

// Keys
#define ODB_KEY_ARDUINO_UPDATE_CAL         ODB_PATH_ARDUINO_SETTINGS "/UpdateCalibration"
#define ODB_KEY_ARDUINO_START_HOME         ODB_PATH_ARDUINO_SETTINGS "/StartHome"
#define ODB_KEY_ARDUINO_MOVE_REQUEST       ODB_PATH_ARDUINO_SETTINGS "/MoveRequest"
#define ODB_KEY_ARDUINO_MOVE_RESPONSE      ODB_PATH_ARDUINO_SETTINGS "/MoveResponse"
#define ODB_KEY_ARDUINO_DESTINATION        ODB_PATH_ARDUINO_SETTINGS "/Destination"
#define ODB_KEY_ARDUINO_VELOCITY           ODB_PATH_ARDUINO_SETTINGS "/Velocity"

#define ODB_KEY_ARDUINO_GANTRY_PULLEY_DIA  ODB_PATH_ARDUINO_SETTINGS "/Calibration/Gantry_PulleyDiameter"
#define ODB_KEY_ARDUINO_GANTRY_ACCEL       ODB_PATH_ARDUINO_SETTINGS "/Calibration/Gantry_Accel"
#define ODB_KEY_ARDUINO_GANTRY_VEL_START   ODB_PATH_ARDUINO_SETTINGS "/Calibration/Gantry_VelStart"
#define ODB_KEY_ARDUINO_GANTRY_VEL_HOME    ODB_PATH_ARDUINO_SETTINGS "/Calibration/Gantry_VelHome"
#define ODB_KEY_ARDUINO_TEMP_C1            ODB_PATH_ARDUINO_SETTINGS "/Calibration/Temp_C1"
#define ODB_KEY_ARDUINO_TEMP_C2            ODB_PATH_ARDUINO_SETTINGS "/Calibration/Temp_C2"
#define ODB_KEY_ARDUINO_TEMP_C3            ODB_PATH_ARDUINO_SETTINGS "/Calibration/Temp_C3"
#define ODB_KEY_ARDUINO_TEMP_RESISTOR      ODB_PATH_ARDUINO_SETTINGS "/Calibration/Temp_Resistor"

#define ODB_KEY_ARDUINO_GANTRY_X           ODB_PATH_ARDUINO_VARIABLES "/" ODB_BANK_ARDUINO_GANTRY "[0]"
#define ODB_KEY_ARDUINO_GANTRY_Y           ODB_PATH_ARDUINO_VARIABLES "/" ODB_BANK_ARDUINO_GANTRY "[1]"
#define ODB_KEY_ARDUINO_STATUS             ODB_PATH_ARDUINO_VARIABLES "/" ODB_BANK_ARDUINO_STATUS "[0]"

#endif // FE_ARDUINO_H