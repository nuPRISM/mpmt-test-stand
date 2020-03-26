/********************************************************************\

Arduino motor control for mPMT test stand.

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

#include "LinuxSerialDevice.h"
#include "TestStandCommHost.h"
#include "motorCalculation.h"
#include "macros.h"

#define  EQ_NAME   "ARDUINO"
#define  EQ_EVID   1
#define  EQ_TRGMSK 0x1111
#define  MSG_RECEIVE_TIMEOUT  5000

#define  BAUD_RATE 115200

// error checking user inputs
const float gantry_x_min_mm = 0.0;
const float gantry_x_max_mm = 1200.0; // max rail is 1219 mm
const float gantry_y_min_mm = 0.0;
const float gantry_y_max_mm = 1200.0;
const float vel_min_mm_s = 0.0; 
const float vel_max_mm_s = 10.0;
const float accel_default_mm_s_2 = 5.0; // acceleration cannot be zero, between 0 to 7.0 mm/s^2
const uint32_t accel_default_cts = mm_to_cts(accel_default_mm_s_2);

using namespace std;

LinuxSerialDevice device;
TestStandCommHost comm(device);

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

BOOL gStartHome;
BOOL gStartMove;
HNDLE handleHome;
HNDLE handleMove;

void start_move(INT hDB, INT hkey, void *info)
{
  // TOFIX: add some checks that we aren't already moving

  if(!gStartMove) return; // Just return if move not requested...

  // Little magic to reset the key to 'n' without retriggering hotlink
  BOOL move = false;
  db_set_data_index1(hDB, handleMove, &move, sizeof(move), 0, TID_BOOL, FALSE);

  string path;
  path += "/Equipment/";
  path += EQ_NAME;
  path += "/Settings";

  // Get the destination position (absolute distance)
  string destpath = path + "/Destination";
  float destination[2] = {0,0};
  int size_dest = sizeof(destination);
  if (db_get_value(hDB, 0, destpath.c_str(), &destination, &size_dest, TID_FLOAT, TRUE) != DB_SUCCESS) {
      // TODO error message
      return;
  }

  // Get the velocity
  string velpath = path + "/Velocity";
  float velocity[2] = {0,0};
  int size_vel = sizeof(velocity);
  if (db_get_value(hDB, 0, velpath.c_str(), &velocity, &size_vel, TID_FLOAT, TRUE) != DB_SUCCESS) {
    // TODO error message
    return;
  }

  // Get the acceleration (have a default value)
  // string accelpath = path + "/Acceleration";
  // float acceleration[2] = {0,0};
  // int size_accel = sizeof(acceleration);
  // if (db_get_value(hDB, 0, accelpath.c_str(), &acceleration, &size_accel, TID_FLOAT, TRUE) != DB_SUCCESS) {
  //   // TODO error message
  //   return;
  // }
  
  //error check user-input data
  if (destination[AXIS_X] < gantry_x_min_mm || destination[AXIS_X] > gantry_x_max_mm) {
    cm_msg(MERROR, "start_move", "Destination on x-axis should be between %f and %f inclusive.\n", gantry_x_min_mm, gantry_x_max_mm);
    printf("Destination on x-axis should be between %f mm and %f mm inclusive.\n", gantry_x_min_mm, gantry_x_max_mm);
    return;
  }

  if (destination[AXIS_Y] < gantry_y_min_mm || destination[AXIS_Y] > gantry_y_max_mm) {
    cm_msg(MERROR, "start_move", "Destination on y-axis should be between %f and %f inclusive.\n", gantry_y_min_mm, gantry_y_max_mm);
    printf("Destination on y-axis should be between %f mm and %f mm inclusive.\n", gantry_y_min_mm, gantry_y_max_mm);
    return;
  }

  if (velocity[AXIS_X] < vel_min_mm_s || velocity[AXIS_X] > vel_max_mm_s
   || velocity[AXIS_Y] < vel_min_mm_s || velocity[AXIS_Y] > vel_max_mm_s) {
    cm_msg(MERROR, "start_move", "Velocity should be between %f and %f inclusive.\n", vel_min_mm_s, vel_max_mm_s);
    printf("Velocity should be between %f and %f inclusive.\n", vel_min_mm_s, vel_max_mm_s);
    return;
  }

  // if (acceleration[AXIS_X] <= accel_min_mm_s_2 || acceleration[AXIS_X] > accel_max_mm_s_2
  //  || acceleration[AXIS_Y] <= accel_min_mm_s_2 || acceleration[AXIS_Y] > accel_max_mm_s_2) {
  //   cm_msg(MERROR, "start_move", "Acceleration should be between %f and %f and cannot be zero.\n", accel_min_mm_s_2, accel_max_mm_s_2);
  //   printf("Acceleration should be between %f and %f and cannot be zero.\n", accel_min_mm_s_2, accel_max_mm_s_2);
  //   return;
  // }
  
  //get motor position from Arduino
  
  // if (!comm.get_data(DATA_MOTOR)) {
  //   printf("Error getting data from the Arduino.");
  //   return 0;
  // }
  //commented out until MSG_ID_DATA is implemented
  // if (!(comm.recv_message(MSG_RECEIVE_TIMEOUT) && comm.received_message().id == MSG_ID_DATA)) {
  //   printf("Error: timeout or invalid ID received.");
  //   return 0;
  // }

  // uint8_t *gantry_position = comm.received_message().data;
  // uint16_t gantry_position_x = NTOHL(gantry_position);
  // uint16_t gantry_position_y = NTOHL(gantry_position+4);

  // placeholder - initialize x and y position as 0
  uint16_t gantry_position_x = 0;
  uint16_t gantry_position_y = 0;

  printf("check x gantry pos: %d\n", gantry_position_x);
  printf("check y gantry pos: %d\n", gantry_position_y);
  
  //convert user values in mm to encoder counts to send to Arduino
  uint32_t user_rel_dist_x_cts = abs_distance_to_rel_cts(gantry_position_x, destination[AXIS_X]);
  uint32_t user_rel_dist_y_cts = abs_distance_to_rel_cts(gantry_position_y, destination[AXIS_Y]);

  printf("check x cts: %d\n", user_rel_dist_x_cts);
  printf("check y cts: %d\n", user_rel_dist_y_cts);

  uint32_t user_vel_x_cts = mm_to_cts(velocity[AXIS_X]);
  uint32_t user_vel_y_cts = mm_to_cts(velocity[AXIS_Y]);

  // uint32_t user_accel_x_cts = mm_to_cts(acceleration[AXIS_X]);
  // uint32_t user_accel_y_cts = mm_to_cts(acceleration[AXIS_Y]);

  // float dest_mm, uint32_t curr_pos_cts
  Direction user_x_dir = get_direction(gantry_position_x,destination[AXIS_X]);
  Direction user_y_dir = get_direction(gantry_position_x,destination[AXIS_Y]);

  // instruct the Arduino to move to the specified destination at specified speed.
  if (comm.move(accel_default_cts,
                user_vel_x_cts,
                user_rel_dist_x_cts,
                AXIS_X,
                user_x_dir)) {
    printf("Move x-direction OK\n");
  } else {
    printf("Error sending move command in x-direction\n");
    return;
  }
    //echo message back 
  if (comm.recv_message(MSG_RECEIVE_TIMEOUT) && comm.received_message().id == MSG_ID_LOG) {
    printf("%s\n", &(comm.received_message().data[1]));
  }

  if (comm.move(accel_default_cts,
                user_vel_y_cts,
                user_rel_dist_y_cts,
                AXIS_Y,
                user_y_dir)) {
    printf("Move y-direction OK\n");
  } else {
    printf("Error sending move command in y-direction\n");
    //TODO: should we stop both motors
    return;
  }

    //echo message back 
  if (comm.recv_message(MSG_RECEIVE_TIMEOUT) && comm.received_message().id == MSG_ID_LOG) {
    printf("%s\n", &(comm.received_message().data[1]));
  }

  printf("Moving to position P_x=%f, P_y=%f\n",destination[AXIS_X],destination[AXIS_Y]);
  printf("Moving with velocity V_x=%f, V_y=%f\n",velocity[AXIS_X],velocity[AXIS_Y]);
  printf("Moving with default acceleration %f (mm/s^2) = %d (counts)\n",accel_default_mm_s_2,accel_default_cts);
}

void start_home(INT hDB, INT hkey, void *info)
{
  // TOFIX: add some checks that we aren't already moving

  if(!gStartHome) return; // Just return if home not requested...

  printf("Start home...\n");
  sleep(3);
  // TOFIX: instruct the Arduino to home
  printf("Finished home\n");

  // Little magic to reset the key to 'n' without retriggering hotlink
  BOOL home = false;
  db_set_data_index1(hDB, handleHome, &home, sizeof(home), 0, TID_BOOL, FALSE);
}

/*-- Frontend Init -------------------------------------------------*/
INT frontend_init()
{
  // Setup connection to Arduino
  int argc;
  char **argv; 

  mfe_get_args(&argc, &argv);
  for (int i=0 ; i < argc; i++) {
    puts(argv[i]);
  }

  if (argc != 2) {
    printf("\nusage: %s <serial device file>\n\nexample:\n    %s /dev/ttyACM0\n\n", argv[0], argv[0]);
    return 0;
  }

  device.set_device_file(argv[1]);
  if (!device.ser_connect(BAUD_RATE)) return FE_ERR_HW;

  // setup connection to ODB (online database)
  int status = cm_get_experiment_database(&hDB, NULL);
  if (status != CM_SUCCESS) {
    cm_msg(MERROR, "frontend_init", "Cannot connect to ODB, cm_get_experiment_database() returned %d", status);
    return FE_ERR_ODB;
  }

  std::string path;
  path += "/Equipment/";
  path += EQ_NAME;
  path += "/Settings";


  // Setup hot-links (open record, callbacks) to StartHome variable

  // This is variable name
  std::string varpath = path + "/StartHome";

  // Get the current value (just to initialize it...)
  gStartHome = false;
  int size = sizeof(gStartHome);
  status = db_get_value(hDB, 0, varpath.c_str(), &gStartHome, &size, TID_BOOL, TRUE);
   
  // Setup actual hot-link
  status = db_find_key (hDB, 0, varpath.c_str(), &handleHome);

  /* Enable hot-link on StartHome of the equipment */
  if ((status = db_open_record(hDB, handleHome, &gStartHome, size, MODE_READ, start_home, NULL)) != DB_SUCCESS)
    return status;
  
  // Setup hot-links (open record, callbacks) to StartMove variable

  // This is variable name
  varpath = path + "/StartMove";
  
  // Get the current value (just to initialize it...)
  gStartMove = false;
  size = sizeof(gStartMove);
  status = db_get_value(hDB, 0, varpath.c_str(), &gStartMove, &size, TID_BOOL, TRUE);
   
  // Setup actual hot-link
  status = db_find_key (hDB, 0, varpath.c_str(), &handleMove);

  /* Enable hot-link on StartMove of the equipment */
  if ((status = db_open_record(hDB, handleMove, &gStartMove, size, MODE_READ, start_move, NULL)) != DB_SUCCESS)
    return status;

  return SUCCESS;
}

/*-- Frontend Exit -------------------------------------------------*/
INT frontend_exit()
{

  // Close connection to Arduino
  // TOFIX!!!
  //

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

int dummy_counter = 0;

/*-- Event readout -------------------------------------------------*/
INT read_arduino_state(char *pevent, INT off)
{

  // Create event header
  bk_init32(pevent);
  
  // Create a bank with some unsigned integers (move control variables))   
  
  // Bank data of unsigned int
  uint32_t *pddata;
  
  // Bank names must be exactly four char
  bk_create(pevent, "MOTO", TID_DWORD, (void**)&pddata);
  
  // Read the state of the motors from Arduino
  // TOFIX!!!
  uint32_t moving = 1;
  uint32_t homing = 0;
  uint32_t is_initialized = 0;
  uint32_t at_limit = 0x0202;
  uint32_t counter = dummy_counter++;
  uint32_t gantry_position_x = (dummy_counter %100);  // Maybe these should be floats?
  uint32_t gantry_position_y = 200 - (dummy_counter %100);  // Maybe these should be floats?

  // Save variables in bank
  *pddata++ = moving;
  *pddata++ = homing;
  *pddata++ = is_initialized;
  *pddata++ = at_limit;
  *pddata++ = counter;
  *pddata++ = gantry_position_x;
  *pddata++ = gantry_position_y;
    
  bk_close(pevent, pddata);	

  // Create a bank with some float (temperature))  
  
  // Bank data of float
  float *pddata2;
  
  // Bank names must be exactly four char
  bk_create(pevent, "TEMP", TID_FLOAT, (void**)&pddata2);
  
  // Read the temperature from Arduino
  // TOFIX!!!
  float temp[5] = {34.2, 25.1, 34.4, 26.5, 34.3};
  temp[3] += dummy_counter %4;
  temp[4] -= dummy_counter %3;

  // Save temperature variables in bank
  for(int i = 0; i < 5; i++) *pddata2++ = temp[i];
    
  bk_close(pevent, pddata2);	
  
  
  return bk_size(pevent);

}
