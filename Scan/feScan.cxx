/********************************************************************\

Program to perform scan of laser over PMTs.

T. Lindner
March, 2002

  $Id$
\********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "midas.h"
#include "mfe.h"
#include "unistd.h"
#include "time.h"
#include "sys/time.h"
#include <stdint.h>
#include <iostream>


#define  EQ_NAME   "Scan"
#define  EQ_EVID   1
#define  EQ_TRGMSK 0x1111

/* Hardware */
extern HNDLE hDB;

/* make frontend functions callable from the C framework */

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
const char *frontend_name = "feScan";
/* The frontend file name, don't change it */
const char *frontend_file_name = (char*)__FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = TRUE;

/* a frontend status page is displayed with this frequency in ms */
INT display_period = 000;

/* maximum event size produced by this frontend */
INT max_event_size = 32 * 34000;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 5 * 1024 * 1024;

/* buffer size to hold events */
INT event_buffer_size = 2 * max_event_size + 10000;

// Scan paramaters


/* Scan Type Flags  */
BOOL first_time = FALSE;  /* used by routine move_next_position to know if this is begin_of_run */
BOOL gbl_first_call = TRUE; /* used for deferred stop routine */
BOOL gbl_called_BOR = FALSE;
int gbl_current_point = 0; // Which point are we on?
std::vector<std::pair<float,float> > gScanPoints;  // All the points in the present scan (X, Y)


/*-- Function declarations -----------------------------------------*/
INT frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();
extern void interrupt_routine(void);
INT read_scan_state(char *pevent, INT off);


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
      RO_RUNNING | RO_ODB,             /* read always */
      500,                    /* poll for 500ms */
      0,                      /* stop run after this event limit */
      0,                      /* number of sub events */
      1,                      /* do log history */
      "", "", "",
    },
    read_scan_state,       /* readout routine */
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
  if(0){ // Something went wrong
    cm_msg(MERROR,"start_move","Error in move! \n");
  }

  if(!gStartMove) return 0; // Just return if move not requested...

  std::string path;
  path += "/Equipment/";
  path += EQ_NAME;
  path += "/Settings";

  // Get the destination position
  std::string destpath = path + "/Destination";
  float destination[2] = {0,0};
  int size = sizeof(destination);
  int status = db_get_value(hDB, 0, destpath.c_str(), &destination, &size, TID_FLOAT, TRUE);

  // Get the velocity
  std::string velpath = path + "/Velocity";
  float velocity[2] = {1,1};
  size = sizeof(velocity);
  status = db_get_value(hDB, 0, velpath.c_str(), &velocity, &size, TID_FLOAT, TRUE);
        
  //printf("Moving to position X=%f, Y=%f with speed %f %f\n",destination[0],destination[1],velocity[0], velocity[1]);
  cm_msg(MINFO,"start_move","Moving to position X=%f, Y=%f with speed %f %f\n",destination[0],destination[1],velocity[0], velocity[1]);

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
  // TOFIX!!!
  // 


  // setup connection to ODB (online database)
  int status = cm_get_experiment_database(&hDB, NULL);
  if (status != CM_SUCCESS) {
    cm_msg(MERROR, "frontend_init", "Cannot connect to ODB, cm_get_experiment_database() returned %d", status);
    return FE_ERR_ODB;
  }


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

  // Get Scan parameters...
    std::string path;
  path += "/Equipment/";
  path += EQ_NAME;
  path += "/Settings";


  // Setup hot-links (open record, callbacks) to StartHome variable

  // This is variable name
  std::string varpath = path + "/StepSize";

  // Step size
  float step_size = 10.0;  // step size in mm.
  int size = sizeof(step_size);
  int status = db_get_value(hDB, 0, varpath.c_str(), &step_size, &size, TID_FLOAT, TRUE);
   
  // Start Position
  varpath = path + "/StartPosition";
  float start_position[2] = {50.0,50.0};  // in mm.
  size = sizeof(start_position);
  status = db_get_value(hDB, 0, varpath.c_str(), &start_position, &size, TID_FLOAT, TRUE);
 
  // Start Position
  varpath = path + "/Distance";
  float distance[2] = {100.0,100.0};  // in mm.
  size = sizeof(distance);
  status = db_get_value(hDB, 0, varpath.c_str(), &distance, &size, TID_FLOAT, TRUE);
 
  // Add some error checking that the distances and starting point are reasonable!!!
  // *** ASDASDASD ASD*** ** 
  if(distance[0] < step_size || distance[1] < step_size){
    cm_msg(MERROR,"begin_of_run","Error, distance (%f %f) must be greater than step size %f \n",
	   distance[0],distance[1],step_size);  
  }

  //status = gen_scan_path();
  
  gScanPoints.clear();
  
  // Setup a rectangular scan, moving in X first.
  bool moving_forward = true; //ie, moving in +X direction.
  float current_x_pos = start_position[0];
  float current_y_pos = start_position[1];
  float final_y_pos = start_position[1] + distance[1];

  // Keep adding points until we go too far in +Y  
  while(current_y_pos < final_y_pos && gScanPoints.size() < 100000){

    // Add a point
    std::cout << "Adding point (" <<gScanPoints.size()<< "): " 
	      << current_x_pos << " " << current_y_pos << std::endl;
    gScanPoints.push_back(std::pair<float,float>(current_x_pos,current_y_pos));

    // Decide what the next point will be
    if(moving_forward){  // Moving in +X direction
      // Can we keep moving in +X direction?
      float next_x_pos = current_x_pos + step_size;
      if((next_x_pos - start_position[0]) > distance[0]){ // No! That's too far 
	moving_forward = false;
	current_y_pos += step_size;
      }else{ // Yes. Can keep going in +X
	current_x_pos = next_x_pos;
      }
    }else{  // Moving in -X direction
      // Can we keep moving in -X direction?
      float next_x_pos = current_x_pos - step_size;
      if((next_x_pos - start_position[0]) < 0){ // No! That's too far 
	moving_forward = true;
	current_y_pos += step_size;
      }else{ // Yes. Can keep going in -X
	current_x_pos = next_x_pos;
      }
    }
  }

  cm_msg(MINFO,"begin_of_run","Setup scan: step size = %f mm, distance = {%f,%f}mm, total points = %i",
	 step_size,distance[0],distance[1],gScanPoints.size());  

  // We are starting to move;
  gbl_called_BOR = TRUE;



  ss_sleep(1000); /* sleep before starting loop*/

  /* Start cycle */
  first_time = TRUE;
  gbl_current_point = 0;




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
INT read_scan_state(char *pevent, INT off)
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
 


 
