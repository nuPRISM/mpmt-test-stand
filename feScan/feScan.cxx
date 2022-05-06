/********************************************************************\

Program to perform scan of laser over PMTs.
Setups up then performs a rectangular scan in X and Y.

T. Lindner
March 2022

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
#include <chrono>
#include "odbxx.h"

#include "feArduino.h"
#include "shared_defs.h"

#define  EQ_NAME   "Scan"
#define  EQ_EVID   1
#define  EQ_TRGMSK 0x1111

#define SCAN_STATUS_STOPPED 0
#define SCAN_STATUS_STARTED 1
#define SCAN_STATUS_MOVING 2
#define SCAN_STATUS_MEASURING 3

/* Hardware */
extern HNDLE hDB;
BOOL equipment_common_overwrite = FALSE;

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
BOOL gbl_called_BOR = FALSE;
bool gGantryWasMoving = false; // Was gantry previously moving?
int gbl_current_point = -1; // Which point are we on?
bool gNewScanningPoint = false;
std::vector<std::pair<float,float> > gScanPoints;  // All the points in the present scan (X, Y)
float gScanTime; // Scan time in milliseconds.
typedef std::chrono::high_resolution_clock Clock;
Clock::time_point timeStartMeasurement;  // time at the start of the measurement at particular point.   
DWORD gScanStatus;

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
      RO_RUNNING | RO_EOR | RO_ODB,  /* read during and at end of run */
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




/*-- Frontend Init -------------------------------------------------*/
INT frontend_init()
{


  // setup connection to ODB (online database)
  int status = cm_get_experiment_database(&hDB, NULL);
  if (status != CM_SUCCESS) {
    cm_msg(MERROR, "frontend_init", "Cannot connect to ODB, cm_get_experiment_database() returned %d", status);
    return FE_ERR_ODB;
  }

  gScanStatus = SCAN_STATUS_STOPPED;

  return SUCCESS;
}

/*-- Frontend Exit -------------------------------------------------*/
INT frontend_exit()
{

  return SUCCESS;
}



/*-- Begin of Run --------------------------------------------------*/
INT begin_of_run(INT run_number, char *error)
{
  gScanStatus = SCAN_STATUS_STARTED;
  gNewScanningPoint = false;
  // Get Scan parameters...
  std::string path;
  path += "/Equipment/";
  path += EQ_NAME;
  path += "/Settings";

  std::string varpath;
  int status;
  int size;

  // Step size
  varpath =  path + "/StepSize";
  float step_size = 10.0;  // step size in mm.
  size = sizeof(step_size);
  status = db_get_value(hDB, 0, varpath.c_str(), &step_size, &size, TID_FLOAT, TRUE);
  if (status != DB_SUCCESS) {
      cm_msg(MERROR, "begin_of_run", "Failed to retrieve StepSize from ODB. Error: %d", status);
      return CM_DB_ERROR;
  }


  // Start Position
  varpath = path + "/StartPosition";
  float start_position[2] = {50.0,50.0};  // in mm.
  size = sizeof(start_position);
  status = db_get_value(hDB, 0, varpath.c_str(), &start_position, &size, TID_FLOAT, TRUE);
  if (status != DB_SUCCESS) {
      cm_msg(MERROR, "begin_of_run", "Failed to retrieve StartPosition from ODB. Error: %d", status);
      return CM_DB_ERROR;
  }

  // Start Position
  varpath = path + "/Distance";
  float distance[2] = {100.0,100.0};  // in mm.
  size = sizeof(distance);
  status = db_get_value(hDB, 0, varpath.c_str(), &distance, &size, TID_FLOAT, TRUE);
  if (status != DB_SUCCESS) {
      cm_msg(MERROR, "begin_of_run", "Failed to retrieve Distance from ODB. Error: %d", status);
      return CM_DB_ERROR;
  }

  // Add some error checking that the distances and starting point are reasonable!!!
  // *** ASDASDASD ASD*** ** 
  if(distance[0] < step_size || distance[1] < step_size){
    cm_msg(MERROR,"begin_of_run","Error, distance (%f %f) must be greater than step size %f \n",
       distance[0],distance[1],step_size);  
  }

  // Scan Time in Milliseconds
  varpath = path + "/ScanTime";
  gScanTime = 5000.0;
  size = sizeof(gScanTime);
  status = db_get_value(hDB, 0, varpath.c_str(), &gScanTime, &size, TID_FLOAT, TRUE);
  if (status != DB_SUCCESS) {
      cm_msg(MERROR, "begin_of_run", "Failed to retrieve ScanTime from ODB. Error: %d", status);
      return CM_DB_ERROR;
  }


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

  cm_msg(MINFO,"begin_of_run","Setup scan: step size = %f mm, distance = {%f,%f}mm, scan time = %f total points = %i",
	 step_size,distance[0],distance[1],gScanTime, (int)gScanPoints.size());  



  ss_sleep(1000); /* sleep before starting loop*/

  /* Start cycle */
  gbl_current_point = -1;
  gGantryWasMoving = false;

  // We are starting to move;
  gbl_called_BOR = TRUE;


  //------ FINAL ACTIONS before BOR -----------
  printf("End of BOR\n");

  return SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/
INT end_of_run(INT run_number, char *error)
{

  //Finished moving
  gbl_called_BOR = FALSE;
  gGantryWasMoving = false;

  gScanStatus = SCAN_STATUS_STOPPED;

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

INT request_move(float x_mm, float y_mm)
{
  
  midas::odb move_set = {
    {"Destination", std::array<float, 2>{}},
    {"Start Move", false},
  };

  move_set.connect("/Equipment/Move/Settings");

  // Set destination
  move_set["Destination"][0] = x_mm/1000.0;
  move_set["Destination"][1] = y_mm/1000.0;

  //Start move
  usleep(10000);
  move_set["Start Move"] = true;

  // Done!
  usleep(10000);
  printf("Request move: %f %f \n", x_mm, y_mm);

  return SUCCESS;
}

void start_measurement()
{
  // This is a placeholder for triggering the digitizer
  timeStartMeasurement = Clock::now();
}

bool measurement_complete()
{
  // This is a placeholder for checking if the digitizer has finished acquiring data
  Clock::time_point timeNow = Clock::now();
  std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - timeStartMeasurement);
  double timediff = ms.count();

  return (timediff >= (gScanTime + 1000));
}

int feloop_counter = 0;
/*-- Frontend Loop -------------------------------------------------*/
INT frontend_loop()
{
  INT status;
  char str[128];

  feloop_counter++;

  midas::odb move_var = {
    {"Completed", false},
    {"Moving", false},
    {"Position", std::array<float, 2>{}},

  };

  move_var.connect("/Equipment/Move/Variables");


  // Only want to start checking if the begin_of_run has been called.                                   
  if (!gbl_called_BOR) return SUCCESS;

  // Make sure we are running
  if (run_state != STATE_RUNNING) return SUCCESS;


  // Are we making a move?
  bool gantry_moving = (bool)move_var["Moving"];
  
  //std::cout << "Checking " << gantry_moving << " " << gGantryWasMoving << std::endl;
  if (!gantry_moving ) { // No, we are not moving; 

    if(gGantryWasMoving) { // We just finished moving.  Start the measurement (record current time)
      start_measurement();
      gScanStatus = SCAN_STATUS_MEASURING;
      //      gGantryWasMoving = false;
      gNewScanningPoint = true;
      std::cout << "    Starting measurement at this point. " << feloop_counter << std::endl;
    }
    gGantryWasMoving = false;

    // Have we finished doing the measurements at this point (or have we not moved at all yet)
    if (measurement_complete() || (gbl_current_point < 0)) {
      if (gbl_current_point >= 0) {
        std::cout << "    Finished measurement at this point. " << feloop_counter << std::endl;
      }
    }
    else { // If no, keep waiting
      return SUCCESS;
    }

    // Terminate the sequence once we have finished last move.
    if (gbl_current_point >= (int)gScanPoints.size()) {
      cm_msg(MINFO, "frontend_loop", "Stopping run after all points are done. Resetting current point number.");
      gbl_current_point = 0;
      // Stop the run
      status = cm_transition(TR_STOP, 0, str, sizeof(str), TR_SYNC, 0);   
      return status;
    }

    // increment for next position
    gbl_current_point++;
    printf("POINT %d / %d: %i\n", (gbl_current_point + 1), (int)gScanPoints.size(),feloop_counter);

    // Move
    float x_mm = gScanPoints[gbl_current_point].first;
    float y_mm = gScanPoints[gbl_current_point].second;
    status = request_move(x_mm, y_mm);
    if (status == SUCCESS) {
        gScanStatus = SCAN_STATUS_MOVING;
        gGantryWasMoving = true;
        printf("    Started move to position (%.2f mm, %.2f mm) %i\n", x_mm, y_mm,feloop_counter);
	// Sleep for 0.3s to make sure feMotor and feMove have correctly noticed that the move has started
	usleep(500000);
    }
    else { // Move failed
      // Stop the run
      printf("    MoveRequest failed. Error: %d\n", status);
      status = cm_transition(TR_STOP, 0, str, sizeof(str), TR_SYNC, 0);
      return status;
    }

  } else {  // Yes, we are moving;
    gGantryWasMoving = true;
    usleep(1000);
  }
  usleep(1000);

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
  
  // Create bank with scan state
  
  // Bank data of unsigned int
  uint32_t *pddata;

  
  bk_create(pevent, "SCAN", TID_DWORD, (void**)&pddata);
  
  *pddata++ = gScanStatus;
  *pddata++ = (gbl_current_point + 1); // gbl_current_point is 0-indexed
  *pddata++ = gScanPoints.size();
  bk_close(pevent, pddata);	




  printf("Saving scan point bank CYC0; point %i \n",gbl_current_point);
  midas::odb move_var = {
    {"Position", std::array<float, 2>{}},
  };
  move_var.connect("/Equipment/Move/Variables");
  
  /* CYCI Bank Contents: 1 per measuring point! */
  double *pwdata;
  bk_create(pevent, "CYC0", TID_DOUBLE, (void **)&pwdata);
  *pwdata++ = (double) gbl_current_point;
  
  // Save the X and Y positions twice
  *pwdata++ = (double) move_var["Position"][0];
  *pwdata++ = (double) move_var["Position"][1];
  for(int i = 0; i < 3; i++){ *pwdata++ = 0.0; } // Fill some blanks in bank
  *pwdata++ = (double) move_var["Position"][0];
  *pwdata++ = (double) move_var["Position"][1];
  for(int i = 0; i < 3; i++){ *pwdata++ = 0.0; } // Fill some blanks in bank
  
  *pwdata++ = (double) 0.0;
  bk_close(pevent, pwdata);
  

  if(gNewScanningPoint){

    char bk_name[4] = "EOM";
    double *unused_pointer2;
    bk_create(pevent, bk_name, TID_DOUBLE,(void **) &unused_pointer2);
    *unused_pointer2++ = (double) gbl_current_point;
    bk_close(pevent, unused_pointer2);

  }

  gNewScanningPoint = false;


  return bk_size(pevent);

}
 


 
