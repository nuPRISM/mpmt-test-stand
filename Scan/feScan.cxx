/********************************************************************\

Program to perform scan of laser over PMTs.
Setups up then performs a rectangular scan in X and Y.

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
#include <chrono>

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
BOOL gbl_called_BOR = FALSE;
bool gGantryWasMoving = false; // Was gantry previously moving?
int gbl_current_point = 0; // Which point are we on?
std::vector<std::pair<float,float> > gScanPoints;  // All the points in the present scan (X, Y)
//time_t timeStartMeasurement; // time at the start of the measurement at particular point.
typedef std::chrono::high_resolution_clock Clock;
Clock::time_point timeStartMeasurement;  // time at the start of the measurement at particular point.   

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




/*-- Frontend Init -------------------------------------------------*/
INT frontend_init()
{


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



  ss_sleep(1000); /* sleep before starting loop*/

  /* Start cycle */
  first_time = TRUE;
  gbl_current_point = 0;
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

  INT status;
  char str[128];
  
  // Only want to start checking if the begin_of_run has been called.                                   
  if (!gbl_called_BOR) return SUCCESS;
  
  // Make sure we are running
  if (run_state != STATE_RUNNING) return SUCCESS;
  
  
  // Are we making a move?
  DWORD gantry_moving = false;  // step size in mm.
  int size = sizeof(gantry_moving);
  status = db_get_value(hDB, 0, "/Equipment/ARDUINO/Variables/MOTO[0]", &gantry_moving, &size, TID_DWORD, TRUE);
  // Reset for moment...
  uint32_t test = rand()*100000;
  gantry_moving = true;
  if(test %100000 == 0) gantry_moving = false; 


  //  time_t timeNow;
  //time(&timeNow);        
  Clock::time_point timeNow = Clock::now();
  
  //  double timediff = difftime(timeNow, timeStartMeasurement);
  
  std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - timeStartMeasurement);
  double timediff = ms.count();

  //  std::cout << "out " << timediff << std::endl;
  gantry_moving = true;
  if(timediff < 5000 || timediff > 10000){ // If we haven't finished scan time, then keep waiting)
    gantry_moving = false;
  }


  if (!gantry_moving ) { // No, we are not moving; 

    if(gGantryWasMoving){ // We just finished moving.  Start the measurement (record current time)
      //      time(&timeStartMeasurement);        
      timeStartMeasurement = Clock::now();
      gGantryWasMoving = false;
      printf("Starting measurement at this point\n");
    }

    //    double timediff = difftime(timeNow, timeStartMeasurement);

    //std::cout << timediff << std::endl;
    if(timediff < 5000.0){ // If we haven't finished scan time, then keep waiting)
      return SUCCESS;
    }

    //std::cout << "test/moving: " << test << " " << gantry_moving << std::endl;

    //    if (gbl_waiting_measurement) {
    //return SUCCESS; /* processing has not yet finished i.e. all equipments have not run */
    //}
    //cm_msg(MDEBUG, "frontend_loop", "running: IN_CYCLE is false");
    
    std::cout << "On point: " << gbl_current_point << " " <<  gScanPoints.size() << std::endl;;
    
    // Terminate the sequence once we have finished last move.
    if (gbl_current_point >= gScanPoints.size()) {
      cm_msg(MINFO, "frontend_loop", "Stopping run after all points are done. Resetting current point number.");
      gbl_current_point = 0;
      // db_set_data(hDB, hCurrentPoint, &gbl_current_point, sizeof(INT), 1, TID_INT);
      // Stop the run
      status = cm_transition(TR_STOP, 0, str, sizeof(str), TR_SYNC, 0);   
      return status;
    }
    
    // = TRUE;
    //time_move_next_position = ss_millitime();
    //status = move_next_position();
    

    // Set the next destination
    float destination[2];
    destination[0] = gScanPoints[gbl_current_point].first;
    destination[1] = gScanPoints[gbl_current_point].second;
    size = sizeof(destination);
    status = db_set_value(hDB, 0, "/Equipment/ARDUINO/Settings/Destination", &destination, size, 2, TID_FLOAT);    
    // Start the move
    BOOL start = 1;
    size = sizeof(start);
    status = db_set_value(hDB, 0, "/Equipment/ARDUINO/Settings/StartMove", &start, size, 1, TID_BOOL);    
    
    printf("Started move to position %f %f\n",destination[0],destination[1]);
    gGantryWasMoving = true;    
    
    //    sleep(6);
    
    // increment for next position
    gbl_current_point++;
    
  }else{  // Yes, we are moving;
    gGantryWasMoving = true;    
    usleep(1);
  }


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
  
  *pddata++ = 0; // In scan?
  *pddata++ = gbl_current_point;
  *pddata++ = gScanPoints.size();
  bk_close(pevent, pddata);	

  return bk_size(pevent);

}
 


 
