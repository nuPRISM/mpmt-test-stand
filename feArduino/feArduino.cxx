/********************************************************************\

Arduino frontend for mPMT test stand.

  $Id$
\********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>
#include <iostream>
#include <sstream>

#include "midas.h"
#include "mfe.h"
#include "time.h"
#include "sys/time.h"

#include "feArduino.h"
#include "ArduinoHelper.h"
#include "DefaultCalibration.h"

#define  EQ_NAME   EQ_ARDUINO
#define  EQ_EVID   1
#define  EQ_TRGMSK 0x1111


/* Hardware */
extern HNDLE hDB;

/* make frontend functions callable from the C framework */

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
const char *frontend_name = "feArduino";
/* The frontend file name, don't change it */
const char *frontend_file_name = (char*)__FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = FALSE;

/* a frontend status page is displayed with this frequency in ms */
INT display_period = 000;

/* maximum event size produced by this frontend */
INT max_event_size = 32 * 34000;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 5 * 1024 * 1024;

/* buffer size to hold events */
INT event_buffer_size = 2 * max_event_size + 10000;

/*-- Function declarations -----------------------------------------*/
INT frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();
extern void interrupt_routine(void);
INT read_arduino_state(char *pevent, INT off);

/*-- Equipment list ------------------------------------------------*/
#undef USE_INT
EQUIPMENT equipment[] = {

  { EQ_NAME,                 /* equipment name */
    {
      EQ_EVID, EQ_TRGMSK,     /* event ID, trigger mask */
      "SYSTEM",              /* event buffer */
      EQ_PERIODIC ,      /* equipment type */
      LAM_SOURCE(0, 0x8111),     /* event source crate 0, all stations */
      "MIDAS",                /* format */
      TRUE,                   /* enabled */
      RO_ALWAYS | RO_ODB,             /* read always */
      500,                    /* poll for 500ms */
      0,                      /* stop run after this event limit */
      0,                      /* number of sub events */
      0,                      /* don't log history */
      "", "", "",
    },
    read_arduino_state,       /* readout routine */
  },
  {""}
};

/********************************************************************\
              Callback routines for system transitions

  These routines are called whenever a system transition like start/
  stop of a run occurs. The routines are called on the following
  occations:

  frontend_init:  When the frontend program is started. This routine
                  should initialize the hardware.

  frontend_exit:  When the frontend program is shut down. Can be used
                  to releas any locked resources like memory, commu-
                  nications ports etc.

  begin_of_run:   When a new run is started. Clear scalers, open
                  rungates, etc.

  end_of_run:     Called on a request to stop a run. Can send
                  end-of-run event and close run gates.

  pause_run:      When a run is paused. Should disable trigger events.

  resume_run:     When a run is resumed. Should enable trigger events.
\********************************************************************/

/********************************************************************/

/*-- Sequencer callback info  --------------------------------------*/
void seq_callback(INT hDB, INT hseq, void *info)
{
//   KEY key;

  printf("odb ... Settings %x touched\n", hseq);
}

BOOL gUpdateCalibration;
HNDLE handleUpdateCal;

BOOL gStartHome;
HNDLE handleHome;

BOOL gMoveRequest;
HNDLE handleMoveRequest;

// Host Calibration
float gCalGantryPulleyDia = default_pulley_diameter;

// Arduino Calibration
float gCalGantryAccel     = steps_to_mm(default_calibration.cal_gantry.accel);
float gCalGantryVelStart  = steps_to_mm(default_calibration.cal_gantry.vel_start);
float gCalGantryVelHome   = steps_to_mm(default_calibration.cal_gantry.vel_home);
double gCalTempC1         = default_calibration.cal_temp.all.c1;
double gCalTempC2         = default_calibration.cal_temp.all.c2;
double gCalTempC3         = default_calibration.cal_temp.all.c3;
double gCalTempResistor   = default_calibration.cal_temp.all.resistor;

void update_calibration()
{
    Calibration calibration = {
        .cal_gantry = {
            .accel = mm_to_steps(gCalGantryAccel),
            .vel_start = mm_to_steps(gCalGantryVelStart),
            .vel_home = mm_to_steps(gCalGantryVelHome)
        },
        .cal_temp = {
            .all = {
                .c1 = gCalTempC1,
                .c2 = gCalTempC2,
                .c3 = gCalTempC3,
                .resistor = gCalTempResistor
            }
        }
    };

    arduino_calibrate(&calibration);
}

void update_calibration(INT hDB, INT hkey, void *info)
{
    if (!gUpdateCalibration) return;
    update_calibration();

    // Reset UpdateCalibration
    BOOL update_cal = false;
    db_set_data_index1(hDB, handleUpdateCal, &update_cal, sizeof(update_cal), 0, TID_BOOL, FALSE);
}

void move_request(INT hDB, INT hkey, void *info)
{
  if(!gMoveRequest) return; // Just return if move not requested...

  INT status;
  int size;

  // Clear MoveResponse
  BOOL response[2] = {false, false};
  size = sizeof(response);
  status = db_set_value(hDB, 0, ODB_KEY_ARDUINO_MOVE_RESPONSE, &response, size, 2, TID_BOOL);
  if (status != DB_SUCCESS) {
      cm_msg(MERROR, "start_move", "Failed to clear MoveResponse in ODB. Error: %d", status);
      return;
  }

  // Get absolute destination position
  float destination[2] = {0,0};
  int size_dest = sizeof(destination);
  status = db_get_value(hDB, 0, ODB_KEY_ARDUINO_DESTINATION, &destination, &size_dest, TID_FLOAT, TRUE);
  if (status != DB_SUCCESS) {
      cm_msg(MERROR, "start_move", "Failed to retrieve Destination from ODB. Error: %d", status);
      return;
  }

  // Get velocity
  float velocity[2] = {0,0};
  int size_vel = sizeof(velocity);
  status = db_get_value(hDB, 0, ODB_KEY_ARDUINO_VELOCITY, &velocity, &size_vel, TID_FLOAT, TRUE);
  if (status != DB_SUCCESS) {
    cm_msg(MERROR, "start_move", "Failed to retrieve Velocity from ODB. Error: %d", status);
    return;
  }

  // Send MOVE to Arduino
  bool move_success = arduino_move(destination, velocity);

  // Set MoveResponse
  response[0] = true;         // Index 0 just indicates we have a response
  response[1] = move_success; // Index 0 indicates success or failure
  size = sizeof(response);
  status = db_set_value(hDB, 0, ODB_KEY_ARDUINO_MOVE_RESPONSE, &response, size, 2, TID_BOOL);

  // Reset MoveRequest
  BOOL move = false;
  db_set_data_index1(hDB, handleMoveRequest, &move, sizeof(move), 0, TID_BOOL, FALSE);
}

void start_home(INT hDB, INT hkey, void *info)
{
  // TOFIX: add some checks that we aren't already moving

  if(!gStartHome) return; // Just return if home not requested...

  printf("================================================================================\n");
  printf("START HOME\n");
  printf("--------------------------------------------------------------------------------\n");

  arduino_run_home();

  // Reset StartHome
  BOOL home = false;
  db_set_data_index1(hDB, handleHome, &home, sizeof(home), 0, TID_BOOL, FALSE);

  printf("================================================================================\n");
}

static INT setup_odb_var(const char *key, void *data, INT size, DWORD type, bool hotlink, HNDLE *handle, void (*dispatcher)(INT, INT, void *), void *info)
{
  INT status = db_get_value(hDB, 0, key, data, &size, type, TRUE);
  if (status != DB_SUCCESS) {
      cm_msg(MERROR, "setup_odb_var", "db_get_value failed for key: %s. Error: %d", key, status);
      return status;
  }

  if (hotlink) {
    HNDLE tmp_handle;
    if (handle == NULL) handle = &tmp_handle;
    status = db_find_key(hDB, 0, key, handle);
    if (status != DB_SUCCESS) {
        cm_msg(MERROR, "setup_odb_var", "db_find_key failed for key: %s. Error: %d", key, status);
        return status;
    }
    if ((status = db_open_record(hDB, *handle, data, size, MODE_READ, dispatcher, info)) != DB_SUCCESS) {
      cm_msg(MERROR, "setup_odb_var", "db_open_record failed for key: %s. Error: %d", key, status);
      return status;
    }
  }

  return DB_SUCCESS;
}

static INT setup_odb_var(const char *key, void *data, INT size, DWORD type, bool hotlink)
{
    return setup_odb_var(key, data, size, type, hotlink, NULL, NULL, NULL);
}

static INT setup_odb_var(const char *key, void *data, INT size, DWORD type)
{
    return setup_odb_var(key, data, size, type, false);
}

/*-- Frontend Init -------------------------------------------------*/
INT frontend_init()
{
  /* *************************** CONNECT TO ARDUINO *************************** */

  // Read the name of the serial device from the command line arguments
  int argc;
  char **argv; 

  mfe_get_args(&argc, &argv);
  for (int i=0 ; i < argc; i++) {
    puts(argv[i]);
  }

  if (argc != 2) {
    printf("\nusage: %s <serial device file>\n\nexample:\n    %s /dev/ttyACM0\n\n", argv[0], argv[0]);
    return FE_ERR_HW;
  }

  if (!arduino_connect(argv[1])) return FE_ERR_HW;

  /* ***************************** CONNECT TO ODB ***************************** */

  int status = cm_get_experiment_database(&hDB, NULL);
  if (status != CM_SUCCESS) {
    cm_msg(MERROR, "frontend_init", "Cannot connect to ODB, cm_get_experiment_database() returned %d", status);
    return FE_ERR_ODB;
  }

  /* ***************************** SETTINGS VARS ****************************** */
  // DESTINATION
  float destination[2] = {0,0};
  if (setup_odb_var(ODB_KEY_ARDUINO_DESTINATION, &destination, sizeof(destination), TID_FLOAT) != DB_SUCCESS) return FE_ERR_ODB;
  // VELOCITY
  float velocity[2] = {0,0};
  if (setup_odb_var(ODB_KEY_ARDUINO_VELOCITY, &velocity, sizeof(velocity), TID_FLOAT) != DB_SUCCESS) return FE_ERR_ODB;

  /* **************************** CALIBRATION VARS **************************** */
  if (setup_odb_var(ODB_KEY_ARDUINO_GANTRY_PULLEY_DIA, &gCalGantryPulleyDia, sizeof(gCalGantryPulleyDia), TID_FLOAT, true) != DB_SUCCESS) return FE_ERR_ODB;
  if (setup_odb_var(ODB_KEY_ARDUINO_GANTRY_ACCEL, &gCalGantryAccel, sizeof(gCalGantryAccel), TID_FLOAT, true) != DB_SUCCESS) return FE_ERR_ODB;
  if (setup_odb_var(ODB_KEY_ARDUINO_GANTRY_VEL_START, &gCalGantryVelStart, sizeof(gCalGantryVelStart), TID_FLOAT, true) != DB_SUCCESS) return FE_ERR_ODB;
  if (setup_odb_var(ODB_KEY_ARDUINO_GANTRY_VEL_HOME, &gCalGantryVelHome, sizeof(gCalGantryVelHome), TID_FLOAT, true) != DB_SUCCESS) return FE_ERR_ODB;
  if (setup_odb_var(ODB_KEY_ARDUINO_TEMP_C1, &gCalTempC1, sizeof(gCalTempC1), TID_DOUBLE, true) != DB_SUCCESS) return FE_ERR_ODB;
  if (setup_odb_var(ODB_KEY_ARDUINO_TEMP_C2, &gCalTempC2, sizeof(gCalTempC2), TID_DOUBLE, true) != DB_SUCCESS) return FE_ERR_ODB;
  if (setup_odb_var(ODB_KEY_ARDUINO_TEMP_C3, &gCalTempC3, sizeof(gCalTempC3), TID_DOUBLE, true) != DB_SUCCESS) return FE_ERR_ODB;
  if (setup_odb_var(ODB_KEY_ARDUINO_TEMP_RESISTOR, &gCalTempResistor, sizeof(gCalTempResistor), TID_DOUBLE, true) != DB_SUCCESS) return FE_ERR_ODB;

  update_calibration();

  /* ******************************* HOT-LINKS ******************************** */
  void (*update_cal_handler)(INT, INT, void *) = update_calibration;
  if (setup_odb_var(ODB_KEY_ARDUINO_UPDATE_CAL, &gUpdateCalibration, sizeof(gUpdateCalibration), TID_BOOL, true, &handleUpdateCal, update_cal_handler, NULL) != DB_SUCCESS) return FE_ERR_ODB;

  if (setup_odb_var(ODB_KEY_ARDUINO_START_HOME, &gStartHome, sizeof(gStartHome), TID_BOOL, true, &handleHome, start_home, NULL) != DB_SUCCESS) return FE_ERR_ODB;
  if (setup_odb_var(ODB_KEY_ARDUINO_MOVE_REQUEST, &gMoveRequest, sizeof(gMoveRequest), TID_BOOL, true, &handleMoveRequest, move_request, NULL) != DB_SUCCESS) return FE_ERR_ODB;

  return SUCCESS;
}

/*-- Frontend Exit -------------------------------------------------*/
INT frontend_exit()
{
  arduino_stop();
  arduino_disconnect();
  return SUCCESS;
}

/*-- Begin of Run --------------------------------------------------*/
INT begin_of_run(INT run_number, char *error)
{

  //------ FINAL ACTIONS before BOR -----------
  printf("End of BOR\n");

  return SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/
INT end_of_run(INT run_number, char *error)
{
  arduino_stop();
  printf("EOR\n");
  
  return SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/
INT pause_run(INT run_number, char *error)
{
  return SUCCESS;
}

/*-- Resume Run ----------------------------------------------------*/
INT resume_run(INT run_number, char *error)
{
  return SUCCESS;
}

/*-- Frontend Loop -------------------------------------------------*/
INT frontend_loop()
{

  return SUCCESS;
}

/*------------------------------------------------------------------*/
/********************************************************************\
  Readout routines for different events
\********************************************************************/
int Nloop, Ncount;

/*-- Trigger event routines ----------------------------------------*/
 INT poll_event(INT source, INT count, BOOL test)
/* Polling routine for events. Returns TRUE if event
   is available. If test equals TRUE, don't return. The test
   flag is used to time the polling */
{
  register int i;  // , mod=-1;
  register int lam = 0;

  for (i = 0; i < count; i++) {
    
    if (lam) {
      if (!test){
        return lam;
      }
    }
  }
  return 0;
}

/*-- Interrupt configuration ---------------------------------------*/
 INT interrupt_configure(INT cmd, INT source, POINTER_T adr)
{
  switch (cmd) {
  case CMD_INTERRUPT_ENABLE:
    break;
  case CMD_INTERRUPT_DISABLE:
    break;
  case CMD_INTERRUPT_ATTACH:
    break;
  case CMD_INTERRUPT_DETACH:
    break;
  }
  return SUCCESS;
}

/*-- Event readout -------------------------------------------------*/
INT read_arduino_state(char *pevent, INT off)
{
  // Create event header
  bk_init32(pevent);

  // Status Bank
  DWORD status;
  if (arduino_get_status(&status)) {
    DWORD *pddata_status;
    bk_create(pevent, ODB_BANK_ARDUINO_STATUS, TID_DWORD, (void**)&pddata_status);
    *pddata_status++ = status;
    bk_close(pevent, pddata_status);
  }

  // Gantry Bank
  float gantry_x_mm, gantry_y_mm;
  if (arduino_get_position(&gantry_x_mm, &gantry_y_mm)) {
    float *pddata_gantry;
    bk_create(pevent, ODB_BANK_ARDUINO_GANTRY, TID_FLOAT, (void**)&pddata_gantry);
    *pddata_gantry++ = gantry_x_mm;
    *pddata_gantry++ = gantry_y_mm;
    bk_close(pevent, pddata_gantry);
  }

  // Temp Bank
  TempData temp_data;
  if (arduino_get_temp(&temp_data)) {
    double *pddata_temp;
    bk_create(pevent, ODB_BANK_ARDUINO_TEMP, TID_DOUBLE, (void**)&pddata_temp);
    *pddata_temp++ = temp_data.temp_ambient;
    *pddata_temp++ = temp_data.temp_motor_x;
    *pddata_temp++ = temp_data.temp_motor_y;
    *pddata_temp++ = temp_data.temp_mpmt;
    *pddata_temp++ = temp_data.temp_optical;
    bk_close(pevent, pddata_temp);
  }

  return bk_size(pevent);

}
