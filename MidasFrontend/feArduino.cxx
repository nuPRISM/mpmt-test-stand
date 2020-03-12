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
// #include "unistd.h"
#include "time.h"
#include "sys/time.h"

#include "LinuxSerialDevice.h"
#include "TestStandCommHost.h"

#define  EQ_NAME   "ARDUINO"
#define  EQ_EVID   1
#define  EQ_TRGMSK 0x1111

#define BAUD_RATE 115200

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
  KEY key;

  printf("odb ... Settings %x touched\n", hseq);
}

BOOL gStartHome;
BOOL gStartMove;
HNDLE handleHome;
HNDLE handleMove;


INT start_move(){

  // TOFIX: add some checks that we aren't already moving

  if(!gStartMove) return 0; // Just return if move not requested...

  std::string path;
  path += "/Equipment/";
  path += EQ_NAME;
  path += "/Settings";

  // Get the destination position
  std::string destpath = path + "/Destination";
  float destination[2] = {0,0};
  int size_dest = sizeof(destination);
  int status_dest = db_get_value(hDB, 0, destpath.c_str(), &destination, &size_dest, TID_FLOAT, TRUE);

  // Get the velocity
  std::string velpath = path + "/Velocity";
  float velocity[2] = {0,0};
  int size_vel = sizeof(velocity);
  int status_vel = db_get_value(hDB, 0, velpath.c_str(), &velocity, &size_vel, TID_FLOAT, TRUE);

  // Get the acceleration (have a default value)
  std::string accelpath = path + "/Acceleration";
  float acceleration[2] = {0,0};
  int size_accel = sizeof(acceleration);
  int status_accel = db_get_value(hDB, 0, accelpath.c_str(), &acceleration, &size_accel, TID_FLOAT, TRUE);

  printf("Moving to position P_x=%f, P_y=%f\n",destination[0],destination[1]);
  printf("Moving with velocity V_x=%f, V_y=%f\n",velocity[0],velocity[1]);
  printf("Moving with acceleration A_x=%f, A_y=%f\n",acceleration[0],acceleration[1]);
  
  // TOFIX: instruct the Arduino to move to the specified destination at specified speed.
  
  for(int i = 0; i < 5; i++){
    sleep(1);
    printf(".");
  }
  printf("\nFinished move\n");

  // Little magic to reset the key to 'n' without retriggering hotlink
  BOOL move = false;
  db_set_data_index1(hDB, handleMove, &move, sizeof(move), 0, TID_BOOL, FALSE);

  return 0;

}

INT start_home(){

  // TOFIX: add some checks that we aren't already moving

  if(!gStartHome) return 0; // Just return if home not requested...


  printf("Start home...\n");
  sleep(3);
  // TOFIX: instruct the Arduino to home
  printf("Finished home\n");

  // Little magic to reset the key to 'n' without retriggering hotlink
  BOOL home = false;
  db_set_data_index1(hDB, handleHome, &home, sizeof(home), 0, TID_BOOL, FALSE);

  return 0;
}

/*-- Frontend Init -------------------------------------------------*/
INT frontend_init()
{
  // Setup connection to Arduino
  int argc;
  char **argv; 

  mfe_get_args(&argc, &argv);
  for (int i=0 ; i<argc ; i++)
    puts(argv[i]); 
  
  if (argc != 2) {
        printf("\nusage: %s <serial device file>\n\nexample:\n    %s /dev/ttyACM0\n\n", argv[0], argv[0]);
        return 0;
    }

    device.set_device_file(argv[1]);
    if (!device.ser_connect(BAUD_RATE)) return 1;
  // 


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

  /* Enable hot-link on StartHome of the equipment */
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
 


 
