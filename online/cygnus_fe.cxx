/********************************************************************	\

  Name:         cygnus_fe.c
  Created by:   Francesco Renga

  Contents: Frontend program for CYGNUS_RD

  $Id$

\********************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <time.h>
#include <iostream>
#include <unistd.h>
#include "midas.h"
#include "experim.h"
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <omp.h>

using namespace std;


#define DEBUG false


#define HAVE_CAMERA
#define HAVE_CAEN_BRD
#ifdef HAVE_CAEN_BRD
  #define HAVE_CAEN_DGTZ
#endif

#ifdef HAVE_CAEN_BRD
  #include "vme/caenBridge.h"
#endif

#ifdef HAVE_CAEN_DGTZ
  #include "CAENDigitizer.h"
#endif

#ifdef HAVE_CAMERA
  #include "dcamapi4.h"
  #include "dcamprop.h"
  #include "camera/common.h"
  #include "camera/console4.h"
#endif

/* make frontend functions callable from the C framework */

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
char *frontend_name = "cygnus_daq";
/* The frontend file name, don't change it */
char *frontend_file_name = __FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = FALSE;

/* a frontend status page is displayed with this frequency in ms */
INT display_period = 3000;

/* maximum event size produced by this frontend */
INT max_event_size = 150000000; //1000000000;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 5 * 1024 * 1024;

/* buffer size to hold events */
INT event_buffer_size = 1000000000; //2000000000


int      picIndex = 0;
DWORD    timeZero = 0;

/*-- Function declarations -----------------------------------------*/

/**
 * @brief Initialize the frontend.
 * 
 * Called once at program startup to initialize frontend resources.
 * 
 * @return Status code.
 */
INT frontend_init();
/**
 * @brief Clean up frontend resources before exiting.
 * 
 * Called once when the frontend is shutting down.
 * 
 * @return Status code.
 */
INT frontend_exit();
/**
 * @brief Operations to be executed at the beginning of a run.
 * 
 * @param run_number The number of the run being started.
 * @param error Buffer to store error message if an error occurs.
 * @return Status code.
 */
INT begin_of_run(INT run_number, char *error);
/**
 * @brief Operations to be executed at the end of a run.
 * 
 * @param run_number The number of the run being ended.
 * @param error Buffer to store error message if an error occurs.
 * @return Status code.
 */
INT end_of_run(INT run_number, char *error);
/**
 * @brief Operations to be executed when a run is paused.
 * 
 * @param run_number The number of the run being paused.
 * @param error Buffer to store error message if an error occurs.
 * @return Status code.
 */
INT pause_run(INT run_number, char *error);
/**
 * @brief Operations to be executed when a run is resumed after being paused.
 * 
 * @param run_number The number of the run being resumed.
 * @param error Buffer to store error message if an error occurs.
 * @return Status code.
 */
INT resume_run(INT run_number, char *error);
/**
 * @brief Periodic frontend loop.
 * 
 * This function is called repeatedly in the main loop. It is optional and
 * can be used for polling or monitoring tasks.
 * 
 * @return Status code.
 */
INT frontend_loop();
/**
 * @brief Read a single event into the event buffer.
 * 
 * @param pevent Pointer to the event buffer.
 * @param off Offset in the buffer to start writing the event.
 * @return Status code.
 */
INT read_event(char *pevent, INT off);
/**
 * @brief Poll for events or device activity.
 * 
 * @param source Event source ID.
 * @param count Number of polls to perform.
 * @param test If true, run in test mode without triggering actions.
 * @return Status code (success is 0).
 */
INT poll_event(INT source, INT count, BOOL test);
/**
 * @brief Configure hardware interrupts.
 * 
 * @param cmd Command code for interrupt configuration.
 * @param source Interrupt source ID.
 * @param adr Pointer to configuration data.
 * @return Status code.
 */
INT interrupt_configure(INT cmd, INT source, POINTER_T adr);


/*-- Custom routines  -----------------------------------------*/

/**
 * @brief Initialize VME modules
 * 
 * This function initializes the VME modules used in the frontend.
 * 
 * @return Status code.
 */
INT init_vme_modules();
/**
 * @brief Configure the CAEN Bridge.
 * 
 * This function sets up the CAEN Bridge hardware.
 * 
 * @return Status code.
 */
INT ConfigBridge();
/**
 * @brief Configure the digitizer.
 * 
 * This function sets up the digitizer hardware.
 * 
 * @return Status code.
 */
INT ConfigDgtz();
/**
 * @brief Configure the discriminator.
 * 
 * This function sets up the discriminator hardware.
 * 
 * @return Status code.
 */
INT ConfigDisc();
/**
 * @brief Configure the cameras.
 * 
 * This function sets up the camera hardware.
 * 
 * @param icam Camera index.
 * @return Status code.
 */
INT ConfigCamera(int icam);
/**
 * @brief Print the camera configuration.
 * 
 * @param icam Camera index.
 * @return Status code.
 */
INT PrintCamConfig(int icam);
/**
 * @brief Disable the trigger.
 * 
 * This function disables the trigger functionality.
 * 
 * @return Status code.
 */
INT disable_trigger();
/**
 * @brief Enable the trigger.
 * 
 * This function enables the trigger functionality.
 * 
 * @return Status code.
 */
INT enable_trigger();
/**
 * @brief Clear the device.
 * 
 * This function clears the device, optionally clearing digitizer data.
 * 
 * @param clear_dgtz_data If true, clear digitizer data.
 * @return Status code.
 */
INT ClearDevice(BOOL clear_dgtz_data);
/**
 * @brief Read the TDC data into an event buffer.
 * 
 * @param pevent Pointer to the event buffer.
 * @return Status code.
 */
INT read_tdc(char *pevent);
/**
 * @brief Read the digitizer data into an event buffer.
 * 
 * @param pevent Pointer to the event buffer.
 * @return Status code.
 */
INT read_dgtz(char *pevent);
/**
 * @brief Read the camera data into an event buffer.
 * 
 * @param pevent Pointer to the event buffer.
 * @param icam Camera index.
 * @return Status code.
 */
INT read_camera(char *pevent, int icam);
/**
 * @brief Read the camera status into an event buffer.
 * 
 * @param pevent Pointer to the event buffer.
 * @param off Offset in the buffer to start writing the event.
 * @return Status code.
 */
INT read_camera_status(char *pevent, INT off);
/**
 * @brief Read the trigger status into an event buffer.
 * 
 * @param pevent Pointer to the event buffer.
 * @param off Offset in the buffer to start writing the event.
 * @return Status code.
 */
void ReadDgtzConfig();
/**
 * @brief Free allocated arrays.
 */
void Free_arrays();

#ifdef HAVE_CAEN_DGTZ
  #define MAX_BASE_INPUT_FILE_LENGTH 1000

  /** @brief Save correction tables to a file.
   *
   * This function saves the DRS4 correction tables to a specified file.
   *
   * @param outputFileName Name of the output file.
   * @param groupMask Mask specifying which groups to save.
   * @param tables Pointer to the correction tables structure.
   * @return Status code.
   */
  int SaveCorrectionTables(char *outputFileName, uint32_t groupMask, CAEN_DGTZ_DRS4Correction_t *tables);
#endif

#ifdef HAVE_CAMERA
  #define NCAM_MAX 6
#endif


/*-- Equipment list ------------------------------------------------*/

BOOL equipment_common_overwrite = TRUE;

EQUIPMENT equipment[] = {
  
  {"Trigger",               /* equipment name */
   {1, 0,                   /* event ID, trigger mask */
    "SYSTEM",               /* event buffer */
    EQ_POLLED,              /* equipment type */
    0,                      /* event source */
    "MIDAS",                /* format */
    TRUE,                   /* enabled */
    RO_RUNNING              /* read only when running */
    //|            
    //RO_ODB                /* and update ODB */
    ,
    100,                    /* poll for 100ms */
    0,                      /* stop run after this event limit */
    0,                      /* number of sub events */
    0,                      /* don't log history */
    "", "", "",},
   read_event,      /* readout routine */
  },
#ifdef HAVE_CAMERA
  {"CameraStatus",              /* equipment name */
    {7, 0,                /* event ID, trigger mask */
      "SYSTEM",           /* event buffer */
      EQ_PERIODIC,        /* equipment type */
      0,                  /* event source */
      "MIDAS",            /* format */
      TRUE,               /* enabled */
      RO_RUNNING |        /* read when running and on transitions */
      RO_ODB,             /* and update ODB */
      10000,               /* read every sec */
      0,                  /* stop run after this event limit */
      0,                  /* number of sub events */
      0,                 /* log history every ten seconds*/
      "", "", "",},
    read_camera_status,   /* readout routine */
  },
#endif
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

#ifdef HAVE_CAEN_BRD
  MVME_INTERFACE *gVme = 0;
  int gDisBase   = 0xEE000000;
  int gTdcBase   = 0x33330000;
#endif

#ifdef HAVE_CAEN_DGTZ
  int nboard = 2;
  int *gDGTZ;
  char **buffer_dgtz;
  int *posttrg;   //=  70;

  double **DGTZ_OFFSET;
  uint32_t *NCHDGTZ;        // = 40000;
  uint32_t *ndgtz;          //= 1024;
  uint32_t *SAMPLING;    //250;
  uint32_t *gDigBase;     //0x22220000;
  uint32_t *gDigLink;     //1
  char **BoardName;
#endif


#ifdef HAVE_CAMERA
  int nCamera = 1;
  vector<HDCAM> gCam(NCAM_MAX, 0);
  vector<HDCAMWAIT> hwait(NCAM_MAX, 0);
  vector<bool> CamMask(NCAM_MAX, false);
  vector<DCAMWAIT_OPEN> waitopen(NCAM_MAX);
  //HDCAMWAIT hwait = 0;
#endif

int rec_ev = 0;

std::vector<int> cache_cam = {-999, -999, -999, -999, -999, -999, -999, -999, -999, -999};

/*-- Frontend Init -------------------------------------------------*/

INT frontend_init()
{

#ifdef HAVE_CAEN_BRD

  #ifdef HAVE_CAEN_DGTZ
    ReadDgtzConfig();
  #endif

  CaenBrdgSetPar(cvV1718,0);

  int status = mvme_open(&gVme,1);

  //mvme_set_am(gVme, MVME_AM_A32/*MVME_AM_A24_ND*/);
  mvme_set_am(gVme, MVME_AM_A24_ND);

  printf("gVme %d status %d\n",gVme,status);

  init_vme_modules();

#endif

#ifdef HAVE_CAMERA

  HNDLE hDB;
  cm_get_experiment_database(&hDB, NULL);

  int size = sizeof(int);
  db_get_value(hDB, 0, "/Equipment/Trigger/Settings/nCamera",&nCamera,&size,TID_INT,TRUE);

  if(nCamera <= 0) {
    cerr<<"fontend_init: ncamera = "<<nCamera<<" read by ODB is invalid. Please check."<<endl;
    cm_msg(MERROR, "cygnus_daq", ("fontend_init: ncamera = "+to_string(nCamera)+" read by ODB is invalid. Please check.\n").c_str());
    exit(EXIT_FAILURE);
  } else if (nCamera > NCAM_MAX) {
    cerr<<"fontend_init: ncamera = "<<nCamera<<" read by ODB is greater than maximum value ("<<NCAM_MAX<<"). Please check."<<endl;
    cm_msg(MERROR, "cygnus_daq", ("fontend_init: ncamera = "+to_string(nCamera)+
                   " read by ODB is greater than maximum value ("+to_string(NCAM_MAX)+"). Please check.\n").c_str());
    exit(EXIT_FAILURE);
  }

  DCAMERR err;

  if(nCamera == 1) { // if only one camera is used, then use the default camera

    gCam[0] = dcamcon_init_open(true);
    if(gCam[0] == NULL) {
      cout << "CAMERA NOT FOUND" << endl;
      exit(EXIT_FAILURE);
    }
    dcamcon_show_dcamdev_info(gCam[0]);

  } else {
    for(int icam = 0; icam<nCamera; icam++) {

      // get camera serial number from ODB
      char cam_serial_n[64]="00000";
      size = sizeof(cam_serial_n);
      char query[256];
      sprintf(query,"/Equipment/Trigger/Settings/CameraSN[%i]",icam);
      db_get_value(hDB, 0, query,&cam_serial_n,&size,TID_STRING,TRUE);
      // cout<<"DEBUG SN = "<<cam_serial_n<<endl; //DEBUG

      // Get camera handle by serial number
      cout<<"Getting handle for camera "<<cam_serial_n<<"..."<<endl;
      gCam[icam] = dcamcon_init_open_serial((string)cam_serial_n);
      if(gCam[icam] == NULL) {
        cout << "CAMERA WITH SN "<<cam_serial_n<<" NOT FOUND" << endl;
        exit(EXIT_FAILURE);
      }
    }

    // Get cam mask from ODB
    for(int icam =0; icam<NCAM_MAX; icam ++) {
      BOOL imask = false;
      size = sizeof(imask);  // TID_BOOL is 4 BITs
      char query[256];
      sprintf(query,"/Equipment/Trigger/Settings/CameraMask[%i]",icam);
      db_get_value(hDB, 0, query, &imask,&size,TID_BOOL,TRUE);
      //cerr<<"DEBUG MASK "<<icam<<"= "<<imask<<endl<<flush;
      CamMask[icam] = (bool)imask;
    }

    // Check that masks are "consistent", meaning
    // 1. at least one mask flag among the first nCamera entries is true
    // 2. the mask options for the other NCAM_MAX - nCamera entries will
    //    not be considered
    bool checkMasks = false;
    for(int icam=0; icam<nCamera; icam ++) {
      if(CamMask[icam]) checkMasks = true;
    }
    if(!checkMasks) {
      cm_msg(MERROR, "cygnus_daq", "frontend_init: Masks in the ODB are not consistent with input nCamera options. Please check /Equipment/Trigger/Settings/. Closing cygnus_fe.\n");
      throw runtime_error("frontend_init: Masks in the ODB are not consistent with input nCamera options. Please check /Equipment/Trigger/Settings/. Closing cygnus_fe.\n");
    }


  }

  // Configure cameras
  for(int icam =0; icam<nCamera; icam++) {
    ConfigCamera(icam);
  }

#endif
  
  // Disable trigger at startup
  disable_trigger();

  return SUCCESS;
}

/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit()
{

  // Disable trigger at exit
  disable_trigger();

#ifdef HAVE_CAMERA
  // Stop camera acquisition and release resources
  dcamdev_close( gCam[0] );
  dcamapi_uninit();
#endif
  
#ifdef HAVE_CAEN_DGTZ
  // Free digitizer arrays
  Free_arrays();
#endif
  
  return SUCCESS;

}

/*-- Begin of Run --------------------------------------------------*/

INT begin_of_run(INT run_number, char *error)
{

  rec_ev = 0;
  picIndex = 0;
  
#ifdef HAVE_CAEN_BRD

  ConfigBridge();

  //TO BE CHECKED FOR V3718
  //Stop pulser at the beginning of the run
  CAENVME_StopPulser(gVme->handle,cvPulserA);

  #ifdef HAVE_CAEN_DGTZ
    // Configure digitizer
    ConfigDgtz();
  #endif

#endif

  
#ifdef HAVE_CAMERA

  for(int icam = 0; icam<nCamera; icam++) { 
    if(gCam[icam] == NULL) {
      cout << "CAMERA "<<icam<<" NOT FOUND" << endl;
      exit(EXIT_FAILURE);
    }
  }

  HNDLE hDB;
  cm_get_experiment_database(&hDB, NULL);int mode;

  int size = sizeof(int);
  db_get_value(hDB, 0, "/Configurations/TriggerMode",&mode,&size,TID_INT,TRUE);

  // Get cam mask from ODB
  for(int icam = 0; icam<NCAM_MAX; icam ++) {
    BOOL imask = false;
    size = sizeof(imask);  // TID_BOOL is 4 BITs
    char query[256];
    sprintf(query,"/Equipment/Trigger/Settings/CameraMask[%i]",icam);
    db_get_value(hDB, 0, query, &imask,&size,TID_BOOL,TRUE);
    //cerr<<"DEBUG MASK "<<icam<<"= "<<imask<<endl<<flush;
    CamMask[icam] = (bool)imask;
  }

  // Check that masks are "consistent", meaning
  // 1. at least one mask flag among the first nCamera entries is true
  // 2. the mask options for the other NCAM_MAX - nCamera entries will
  //    not be considered
  bool checkMasks = false;
  for(int icam=0; icam<nCamera; icam ++) {
    if(CamMask[icam]) checkMasks = true;
  }
  if(!checkMasks) {
    cm_msg(MERROR, "cygnus_daq", "begin_of_run: Masks in the ODB are not consistent with input nCamera options. Please check /Equipment/Trigger/Settings/. Closing cygnus_fe.\n");
    throw runtime_error("begin_of_run: Masks in the ODB are not consistent with input nCamera options. Please check /Equipment/Trigger/Settings/. Closing cygnus_fe.\n");
  }

  
  for(int icam =0; icam < nCamera; icam ++) {
    // Check camera mask flag
    if(!CamMask[icam]) continue;

    // Configure each camera
    ConfigCamera(icam);
  
    DCAMERR err;
  
    // Setup wait handle for each camera
    
    memset( &waitopen[icam], 0, sizeof(waitopen[icam]) );
    waitopen[icam].size = sizeof(waitopen[icam]);
    waitopen[icam].hdcam = gCam[icam];
    err = dcamwait_open( &waitopen[icam] );
    if(failed(err)) throw runtime_error(("unable to open camera wait handle for camera"+to_string(icam)+".\n").c_str());
    hwait[icam] = waitopen[icam].hwait; 

    dcambuf_alloc( gCam[icam], 1);
    dcamcap_start( gCam[icam], DCAMCAP_START_SEQUENCE );

  }

  // Enable trigger at the beginning of the run
  //cerr<<"Enabling trigger..."<<endl<<flush;
  enable_trigger();
  usleep(10);



  //CAENVME_SetOutputRegister(gVme->handle, cvOut1Bit|cvOut2Bit);
  CAENVME_SetOutputRegister(gVme->handle, cvOut2Bit);

  // Start pulser for camera synchronization
  if (mode == 3) {
    CAENVME_StartPulser(gVme->handle,cvPulserA);
  } /*else {
    dcamcap_firetrigger(gCam[0],0);
  }*/
  
#endif
  

  return SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/

INT end_of_run(INT run_number, char *error)
{

  // Disable trigger at the end of the run
  disable_trigger();

#ifdef HAVE_CAEN_BRD
  //WRONG FOR V3718
  // Stop pulser at the end of the run
  CAENVME_StopPulser(gVme->handle,cvPulserA);
#endif

#ifdef HAVE_CAMERA

  // Stop camera acquisition and release resources
  for(int icam =0; icam<nCamera;icam++) {
    // Check camera mask flag
    if(!CamMask[icam]) continue;

    dcambuf_release( gCam[icam] );
    dcamwait_close( hwait[icam] );
    dcamcap_stop( gCam[icam] );
  }

#endif
   
 return SUCCESS;

}

/*-- Pause Run -----------------------------------------------------*/

INT pause_run(INT run_number, char *error)
{
  // Pause of run is not supported yet
  cerr<<"pause_run: pause of run via midas is not supported yet. Please stop the run using 'STOP RUN'. Closing the frontend for unexpected behavior."<<endl;
  cm_msg(MERROR, "cygnus_daq", "pause_run: pause of run via midas is not supported yet. Please stop the run using 'STOP RUN'. Closing the frontend for unexpected behavior.\n");
  exit(EXIT_FAILURE);
  disable_trigger();
  
  return SUCCESS;
}

/*-- Resume Run ----------------------------------------------------*/

INT resume_run(INT run_number, char *error)
{
  // Resume of run is not supported yet
  enable_trigger();
  
  return SUCCESS;
}

/*-- Frontend Loop -------------------------------------------------*/

INT frontend_loop()
{
  /* if frontend_call_loop is true, this routine gets called when
     the frontend is idle or once between every event */
  return SUCCESS;
}

/*------------------------------------------------------------------*/

/********************************************************************\

  Readout routines for different events

\********************************************************************/

/*-- Trigger event routines ----------------------------------------*/

INT poll_event(INT source, INT count, BOOL test)
/* Polling routine for events. Returns TRUE if event
   is available. If test equals TRUE, don't return. The test
   flag is used to time the polling */
{

  if (test) return SUCCESS;

  //cerr<<"Polling event ...."<<endl<<flush;

  int maxevents;
  bool freerun;
  int mode;
  int size = sizeof(int);
  HNDLE hDB;
  cm_get_experiment_database(&hDB, NULL);

  size = sizeof(int);
  db_get_value(hDB, 0, "/Configurations/TriggerMode",&mode,&size,TID_INT,TRUE);
  size = 4*sizeof(bool); // TID_BOOL is 4 BITs
  db_get_value(hDB, 0, "/Configurations/FreeRunning",&freerun,&size,TID_BOOL,TRUE);
  size = sizeof(int);
  db_get_value(hDB, 0, "/Configurations/MaxEvents",&maxevents,&size,TID_INT,TRUE);

  // If more than needed event then do not acquire
  double events;
  size = sizeof(double);
  db_get_value(hDB, 0, "/Equipment/Trigger/Statistics/Events sent",&events,&size,TID_DOUBLE,TRUE);
  if(maxevents > 0 && events >= maxevents) return 0;


  int i;

  DWORD flag;
  int lamTDC = 1;
  int lamDGTZ = 1;
  int lamCAM = 0;


  /* poll hardware and set flag to TRUE if new event is available */
#ifdef HAVE_CAMERA

  DCAMERR err1;

  for(int icam = 0; icam < nCamera; icam++) {
    // Check camera mask flag
    if(!CamMask[icam]) continue;

    if(gCam[icam] == NULL) {
      cout << "CAMERA "<<icam<<" NOT FOUND" << endl;
      exit(EXIT_FAILURE);
    }
    if(hwait[icam] == NULL) {
      cout << "WAIT HANDLE OF CAMERA "<<icam<<" NOT FOUND" << endl;
      exit(EXIT_FAILURE);
    }
  }

  //wait for frame ready


  //cerr<<"Waiting for frameready event ...."<<endl<<flush;
  // Setup of DCAMWAIT object
  vector<DCAMWAIT_START> waitstart(NCAM_MAX);

  //#pragma omp parallel for num_threads(nCamera)
  for(int icam = 0; icam < nCamera; icam++) {
    // Check camera mask flag
    if(!CamMask[icam]) continue;

    // Get camera exposure
    double exposure;
    err1 = dcamprop_getvalue(gCam[icam], DCAM_IDPROP_EXPOSURETIME, &exposure);
    if(failed(err1)) cm_msg(MERROR, "cygnus_daq", "poll_event error in get DCAM_IDPROP_EXPOSURETIME");
      
    // Get GEDelay from camera
    double delay;
    err1 = dcamprop_getvalue(gCam[icam], DCAM_IDPROP_TIMING_GLOBALEXPOSUREDELAY, &delay);
    if(failed(err1)) cm_msg(MERROR, "cygnus_daq", "poll_event error in get TIMING_GLOBALEXPOSUREDELAY");
      
    if((mode==1 || mode==2) && nCamera == 1) delay = 360./1000.;
    

    memset( &waitstart[icam], 0, sizeof(waitstart[icam]) );
    waitstart[icam].size = sizeof(waitstart[icam]);

    if (mode==3) {
        waitstart[icam].timeout = DCAMWAIT_TIMEOUT_INFINITE;
        //waitstart[icam].timeout = 5000; // Set very large timeout
    }
    else
    {
        //waitstart[icam].timeout = DCAMWAIT_TIMEOUT_INFINITE;
        waitstart[icam].timeout = (int)((delay+exposure)*1000) + 100; //in ms --> max wait = 2*exposure + USB transfer time // 30 before
    }
    if (mode == 3){
      waitstart[icam].eventmask = DCAMWAIT_CAPEVENT_FRAMEREADY;
    } else {
      waitstart[icam].eventmask = DCAMWAIT_CAPEVENT_FRAMEREADY;
    }
  }

  int pics = 1;
  if(rec_ev==0) {
    pics = 2;
    //CAENVME_ClearOutputRegister(gVme->handle,cvOut2Bit); 
  }

  
  /*// Get number of acquired pictures so far
  DCAMERR errtest;
  DCAMCAP_TRANSFERINFO captransferinfo;
  memset( &captransferinfo, 0, sizeof(captransferinfo) );
  captransferinfo.size	= sizeof(captransferinfo);

  errtest = dcamcap_transferinfo( gCam[0], &captransferinfo );
  if(failed(errtest)) throw runtime_error("poll_event: unable to get captransferinfo.\n");
    
    
  string numframe = to_string((int)captransferinfo.nFrameCount);
  
  cm_msg(MINFO, "cygnus_daq", numframe.c_str());*/
  if (rec_ev==0) {
    usleep(500); //this is necessary do not delete
    CAENVME_ClearOutputRegister(gVme->handle,cvOut2Bit);
  }
  for(int jj=0;jj<pics;jj++){
    

    if(pics ==2 && jj==0) {
      CAENVME_ClearOutputRegister(gVme->handle,cvOut1Bit);

      //cerr<<"---> GATE SET TO 0"<<endl<<flush;
    }
    

    //if(rec_ev == 0 && mode != 3 ) CAENVME_ClearOutputRegister(gVme->handle,cvOut1Bit);
    //if(rev_ev == 0) CAENVME_ClearOutputRegister(gVme->handle,cvOut1Bit);

    //send a trigger to the camera if mode is not continuous
    if(mode!=3 && nCamera == 1) dcamcap_firetrigger(gCam[0],0);
    else if(mode!=3 && nCamera >1) {
      cerr<<"poll_event: operation with more than one camera is supported only with mode = 3."<<endl;
      cm_msg(MERROR, "cygnus_daq", "poll_event: operation with more than one camera is supported only with mode = 3. \n");
      exit(EXIT_FAILURE);
    }
      
    
    // Wait for frameready
    //#pragma omp parallel for num_threads(nCamera) // parallelize the wait for each camera
    for(int icam = 0; icam < nCamera; icam++) {
      // Check camera mask flag
      if(!CamMask[icam]) continue;
      //cerr<<"Starting camera "<<icam<<endl<<flush;
      //cerr<<"Waiting for camera "<<icam<<endl<<flush;
      err1 = dcamwait_start( hwait[icam], &waitstart[icam] );
      if(failed(err1)) {
        cerr<<"poll_event: dcamwait_start failed for camera "<<icam<<" with error "<<err1<<endl<<flush;
      }
    }
    //err1 = dcamwait_start( hwait[0], &waitstart);

    //usleep(1000); // wait 200 us to avoid synch problems with the camera
    //err1 = dcamwait_start( hwait[1], &waitstart2 );

    if(err1 == DCAMERR_TIMEOUT) cm_msg(MERROR, "cygnus_daq", "poll_event: dcamwait_start timeout %d", jj);

    if(pics ==2 && jj==0) {
      CAENVME_SetOutputRegister(gVme->handle,cvOut1Bit);
      //CAENVME_SetOutputRegister(gVme->handle,cvOut4Bit);
      //cerr<<"---> GATE SET TO 1"<<endl<<flush;
    }
    //if(rec_ev == 0 && mode != 3 ) CAENVME_SetOutputRegister(gVme->handle,cvOut1Bit);
    //if(rev_ev == 0) CAENVME_SetOutputRegister(gVme->handle,cvOut1Bit)nd_init: ncamera = 0 read by ;

    lamCAM = 1;
      
    if(err1 == DCAMERR_TIMEOUT) {
      lamCAM = 0;
    }
    if(err1 != DCAMERR_TIMEOUT && failed(err1) && !test) {
      lamCAM = 0;
      /*if(failed(err1)) { // [FIXME]: it provokes termination of the program when stopping the run
          cm_msg(MERROR, "cygnus_fe", "Unable to open the camera wait handle for uknown reasons. Killing cygnus_fe.");
          throw runtime_error("Unable to open the camera wait handle for uknown reasons. Killing cygnus_fe.\n");
      }*/
    }
  }
    
#endif

#ifdef HAVE_V1190
  if(!freerun){
    lamTDC = v1190_DataReady(gVme,gTdcBase);
  }
#endif
    
#ifdef HAVE_CAEN_DGTZ

  // If not contiuous readout and not freerun then check the board for data ready
  if (mode !=3) {
    vector<uint32_t> st(nboard);
    if(!freerun){
      uint32_t status;
      for(int jj=0;jj<nboard;jj++){
        CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_ReadRegister(gDGTZ[jj],CAEN_DGTZ_ACQ_STATUS_ADD,&status); /* read status register */
        st[jj] = status;
        lamDGTZ &= ((status & 0x8)>>3); /* 4th bit is data ready */
      }
    }
  }
#endif

  // If not continuous mode, consider board for acquisition. Otherwise not.
  if(mode!=3) {
      flag = (lamTDC && lamDGTZ && lamCAM);
  } else flag = lamCAM;
  

  // If conditions are satisfied and test is FALSE then return TRUE
  if (flag && !test){

#ifdef HAVE_CAEN_BRD	
	//SET OUT_1 to 0 (busy)
	//WRONG FOR V3718
  if(mode != 3) {
	  CAENVME_ClearOutputRegister(gVme->handle,cvOut1Bit);
	  //cerr<<"---> GATE SET TO 0"<<endl<<flush;
  }

#endif
    return TRUE;
  }
        
#ifdef HAVE_CAEN_BRD
    
#ifdef HAVE_V1190
  if(lamTDC) {
    v1190_SoftClear(gVme,gTdcBase);
  }
#endif
    
#ifdef HAVE_CAEN_DGTZ
  if(lamDGTZ && mode != 3) {
  //if(lamDGTZ) {
    for(int i=0;i<nboard;i++){
      CAEN_DGTZ_ClearData(gDGTZ[i]);
    }
  }
#endif
    
  //Reset GATE (pulser B)
  //WRONG FOR V3718
  //CAENVME_StopPulser(gVme->handle,cvPulserB);
    
#endif

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

INT read_event(char *pevent, INT off)
{

  //CHRONO NOW useless comment
  std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

  rec_ev++;
  
  /* init bank structure */
  bk_init32(pevent);
  INT defaultEvSize = bk_size(pevent);

  //////READ SYSTEMS

#ifdef HAVE_CAMERA
//#pragma omp parallel for// num_threads(nCamera)
  for(int icam=0; icam <nCamera; icam++) {
    // Check camera mask flag
    if(!CamMask[icam]) continue;

    //cerr<<"Reading event from camera "<<icam<<".... "<<endl<<flush;
    read_camera(pevent, icam);
  }
  //read_camera(pevent);
#endif


#ifdef HAVE_CAEN_BRD
  
#ifdef HAVE_V1190
  read_tdc(pevent);
#endif

#ifdef HAVE_CAEN_DGTZ
  HNDLE hDB;

  cm_get_experiment_database(&hDB, NULL);


  int mode;
  int size = sizeof(int);
  db_get_value(hDB, 0, "/Configurations/TriggerMode",&mode,&size,TID_INT,TRUE);

  bool freerun;
  size = 4*sizeof(bool);

  db_get_value(hDB, 0, "/Configurations/FreeRunning",&freerun,&size,TID_BOOL,TRUE);
  if(!freerun && mode != 3) read_dgtz(pevent);

  else if(!freerun && mode == 3) {

    int lamDGTZ = 1;

    // Check if boards are data ready
    vector<uint32_t> st(nboard);

    CAENVME_ClearOutputRegister(gVme->handle,cvOut1Bit);
    //cerr<<"---> GATE SET TO 0"<<endl<<flush;

    uint32_t status;
    for(int jj=0;jj<nboard;jj++){
      CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_ReadRegister(gDGTZ[jj],CAEN_DGTZ_ACQ_STATUS_ADD,&status); /* read status register */
      st[jj] = status;
      lamDGTZ &= ((status & 0x8)>>3); /* 4th bit is data ready */

      //cerr<<"-->"<<jj<<" - "<<st[jj]<<endl<<flush;

      if(ret != CAEN_DGTZ_Success) cerr<<"DEBUG unlucky"<<endl;

    }

    // DEBUG:
    //cerr<<"reading dgtz...??"<<endl;
    //cerr<<lamDGTZ<<endl;

    if(lamDGTZ == 1) {

      //cerr<<"Reading event from dgtz.... "<<endl<<flush;
      read_dgtz(pevent);
    } 
  }

#endif

#endif
  //////////////////
  
  if(mode==3) ClearDevice(false);
  else ClearDevice(true);

  CAENVME_SetOutputRegister(gVme->handle,cvOut1Bit);
  //cerr<<"---> GATE SET TO 1"<<endl<<flush;

  // CHRONO AFTER
  std::chrono::time_point<std::chrono::system_clock> after = std::chrono::system_clock::now();
  //std::cerr<<"DEBUG TIME TO READ EVENT: "<<chrono::duration_cast<chrono::milliseconds>(after - now).count()<<" ms"<<endl;

  if (bk_size(pevent)==defaultEvSize ) { return 0; }


  //cerr<<"End of read_event. Returning event with size "<<bk_size(pevent)<<".... "<<endl<<flush;

  return bk_size(pevent);

}

#ifdef HAVE_CAMERA
INT read_camera_status(char *pevent, INT off) {
    //cerr<<"Reading camera status for "<<nCamera<<" cameras...."<<endl<<flush;
    //HNDLE hDB;
    //cm_get_experiment_database(&hDB, NULL);
    
    double *pdata;
    
    /* init bank structure */
    bk_init(pevent);

    //TIME_STAMP(pevent) = (std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch())).count();
    
    /* create SCLR bank */
    bk_create(pevent, "TCAM", TID_DOUBLE, (void **)&pdata);

    for(int icam = 0; icam < nCamera; icam++) { // Always read temp from all connected cameras for history
      // Check camera mask flag
      //if(!CamMask[icam]) {
      //  *pdata++ = 0.0;
      //}
      
      double cam_temperature;
      dcamprop_getvalue( gCam[icam], DCAM_IDPROP_SENSORTEMPERATURE, &cam_temperature);
      *pdata++ = cam_temperature;
      //cout<<"DEBUG: cam_temperature = "<<cam_temperature<<endl; //DEBUG

    }

    //db_set_value(hDB, 0, "/Equipment/CameraStatus/Variables/Sensor Temperature", &cam_temperature, sizeof(double), 1, TID_DOUBLE);

    bk_close(pevent, pdata);
    
    //cerr<<"End of read_camera_status Returning event with size "<<bk_size(pevent)<<"..."<<endl<<flush;
    return bk_size(pevent);
    
}
#endif

///////CUSTOM ROUTINES

#ifdef HAVE_CAEN_DGTZ
void ReadDgtzConfig(){

  CAEN_DGTZ_ErrorCode ret;
  HNDLE hDB;

  cm_get_experiment_database(&hDB, NULL);

  /////Number of boards
  int size = sizeof(int);
  db_get_value(hDB, 0, "/Configurations/Number of Digitizers",&nboard,&size,TID_INT,TRUE);

  gDGTZ = new int[nboard];
  gDigBase = new int[nboard];     //0x22220000;
  gDigLink = new int[nboard];     //1;
  BoardName = new char*[nboard];
  buffer_dgtz = new char*[nboard];
  NCHDGTZ = new int[nboard];        // = 40000;
  DGTZ_OFFSET = new double*[nboard];
  /////Number of samples per waveform and sampling rate
  ndgtz = new int[nboard];          //= 1024;
  SAMPLING = new int[nboard] ;    //250;
  /////Horizontal offset
  posttrg = new int[nboard];   //=  70;

  //read from the database and initialise gDigBase
  for(int i=0;i<nboard;i++){

    /////Open the board
    char query[100];
    sprintf(query,"/Configurations/Digitizer Base Address[%d]",i);
    db_get_value(hDB, 0, query,&gDigBase[i],&size,TID_INT,TRUE);
    sprintf(query,"/Configurations/Digitizer Link Number[%d]",i);
    db_get_value(hDB, 0, query,&gDigLink[i],&size,TID_INT,TRUE);
    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB,gDigLink[i],0,gDigBase[i],&gDGTZ[i]);
    if(ret != CAEN_DGTZ_Success) {
      printf("Can't open digitizer, board number %d %d\n-- Error %d\n",i,gDigBase[i],ret);
      //ret = CAEN_DGTZ_Reset(gDGTZ[i]);
      //if(ret != CAEN_DGTZ_Success) {
      //printf("Unable to clear digitizer, board number %d %d\n-- Error %d\n",i,gDigBase[i],ret);
      //}
    }

    ////Board name
    CAEN_DGTZ_BoardInfo_t BoardInfo;
    CAEN_DGTZ_GetInfo(gDGTZ[i], &BoardInfo);
    BoardName[i] = new char[10];
    strcpy(BoardName[i],BoardInfo.ModelName);
    printf("%s\n",BoardName[i]);

    ////Buffer preparation
    buffer_dgtz[i]=NULL;

    ////Number of channels
    NCHDGTZ[i] = BoardInfo.Channels;
    if(strcmp(BoardName[i],"V1742")==0)
      NCHDGTZ[i] *= 8;

    ////Vertical Offsets
    DGTZ_OFFSET[i]=new double[32];
    for(int j=0;j<32;j++) DGTZ_OFFSET[i][j]=0.;

  }

}
#endif

#ifdef HAVE_CAEN_BRD

INT init_vme_modules(){

  /* BRIDGE INITIALIZATION */
  unsigned int data;

  //SET POSITIVE POLARITIES OF LEDS AND SIGNALS
  //TO BE CHECKED FOR V3718
  //data = 0x7F;
  //CAENVME_WriteRegister(gVme->handle,cvLedPolRegClear,data);
  //WRONG FOR V3718
  //data = 0x3E0;
  data = 0x3FFF;
  CAENVME_WriteRegister(gVme->handle,cvOutMuxRegClear,data);

  //USE OUT_1 as VME VETO and OUT_3 as LED driver
  //data = 0xCC;
  //CAENVME_WriteRegister(gVme->handle,cvOutMuxRegSet,data);

  //TO BE CHECKED FOR V3718
  //Set OUT_0 as pulser A for periodic trigger to the camera
  CAENVME_SetOutputConf(gVme->handle,cvOutput0,cvDirect,cvActiveHigh,cvMiscSignals);
  
  //TO BE CHECKED FOR V3718
  //Set OUT_2 as pulser B for a single gate
  //CAENVME_SetOutputConf(gVme->handle,cvOutput2,cvInverted,cvActiveHigh,cvMiscSignals);

  //WRONG FOR V3718
  //USE OUT_1 as VME VETO
  //data = 0xC;
  //CAENVME_WriteRegister(gVme->handle,cvOutMuxRegSet,data);

  //data = 0xC0;
  //CAENVME_WriteRegister(gVme->handle,cvOutMuxRegSet,data);
  //data = 0x3FE;
  //data = 0x3E;
  data = 0x003E;
  CAENVME_WriteRegister(gVme->handle,cvOutMuxRegSet,data);

  ConfigBridge();
  
  //WRONG FOR V3718
  //CAENVME_StopPulser(gVme->handle,cvPulserA);
  //CAENVME_StopPulser(gVme->handle,cvPulserB);

#ifdef HAVE_V895

  /* DISCRIMINATOR INITIALIZATION */

  v895_Status(gVme,gDisBase);
  
  v895_writeReg16(gVme,gDisBase,0x40,255); // width 0-7
  v895_writeReg16(gVme,gDisBase,0x42,255); // width 8-15
  v895_writeReg16(gVme,gDisBase,0x4A,0xFFFF); // enable all channels
  
  ConfigDisc();
  
#endif

#ifdef HAVE_V1190

  /* TDC INITIALIZATION */

  v1190_SoftClear(gVme,gTdcBase);

  v1190_SetEdgeDetection(gVme,gTdcBase,V1190_ED_Leading);
  v1190_EdgResolutionSet(gVme,gTdcBase);
  v1190_OffsetSet_ns(gVme,gTdcBase,-500); //-400
  v1190_WidthSet_ns(gVme,gTdcBase,1200); //1200
  v1190_SubtractTriggerTimeON(gVme,gTdcBase);
  v1190_TriggerMatchingSet(gVme,gTdcBase);
  v1190_SetMaxNOfHitsPerEve(gVme,gTdcBase,V1190_HE_32);
  v1190_EnableERRORMark(gVme,gTdcBase);
  v1190_EnableHeaderAndTrailer(gVme,gTdcBase);
  v1190_Status(gVme,gTdcBase,V1190B);

#endif

#ifdef HAVE_CAEN_DGTZ
  
  /* DIGITIZER INITIALIZATION */
  
  CAEN_DGTZ_BoardInfo_t *BoardInfo= new CAEN_DGTZ_BoardInfo_t[nboard] ;
  CAEN_DGTZ_ErrorCode ret;

  for(int i=0;i<nboard;i++)
    {
      ret = CAEN_DGTZ_GetInfo(gDGTZ[i], &BoardInfo[i]);

      printf("\nConnected to CAEN Digitizer Model %s %d -- %d channels\n", BoardInfo[i].ModelName,BoardInfo[i].FamilyCode,NCHDGTZ[i]);

      if(NCHDGTZ[i] > 32) {
	printf("Error in NCHDGTZ %d\n",NCHDGTZ[i]);
	exit(EXIT_FAILURE);
      }
      
      ////Reset the board
      ret |= CAEN_DGTZ_Reset(gDGTZ[i]);                                               /* Reset Digitizer */
      ret |= CAEN_DGTZ_ClearData(gDGTZ[i]);

      //#ifdef HAVE_V1761
      ////Waveform Setup
      if(strcmp(BoardName[i],"V1761")==0 || strcmp(BoardName[i],"V1720E")==0){
	ret |= CAEN_DGTZ_SetChannelEnableMask(gDGTZ[i],(int)(pow(2,NCHDGTZ[i]))-1);                        /* Enabl
e channel 0 and 1*/
	cout << "enable " << (int)(pow(2,NCHDGTZ[i]))-1 << endl;
      }
      else if(strcmp(BoardName[i],"V1742")==0)
	CAEN_DGTZ_SetGroupEnableMask(gDGTZ[i],0xF); 
      //#endif
      //#ifdef HAVE_V1742
      //ret |= CAEN_DGTZ_SetGroupEnableMask(gDGTZ,0x20);                        /* Enable channel 0 and 1*/
      //#endif
   
      ret |= CAEN_DGTZ_SetSWTriggerMode(gDGTZ[i],CAEN_DGTZ_TRGMODE_DISABLED);
      //ret |= CAEN_DGTZ_SetChannelSelfTrigger(gDGTZ,CAEN_DGTZ_TRGMODE_DISABLED,???); //TO BE FIXED 
      ret |= CAEN_DGTZ_SetExtTriggerInputMode(gDGTZ[i],CAEN_DGTZ_TRGMODE_ACQ_ONLY);
      ret |= CAEN_DGTZ_SetMaxNumEventsBLT(gDGTZ[i],256);//128);                                /* Set the max number of events to transfer in a sigle readout */
  
      //Acquisition mode
      ret |= CAEN_DGTZ_SetAcquisitionMode(gDGTZ[i],CAEN_DGTZ_SW_CONTROLLED);          /* Set the acquisition mode */

      if(ret != CAEN_DGTZ_Success) {  
	printf("Errors during Digitizer Initialization, board number %d.\n",i);
	cm_msg(MERROR, "cygnus_daq", "Errors during Digitizer Initialization, board number %d.", i);
	
      }

    }
 ConfigDgtz();
 
 delete[] BoardInfo;
#endif  

  return SUCCESS;
  
}
  
#endif

#ifdef HAVE_CAEN_BRD
INT ConfigBridge(){

  //TO BE CHECKD FOR V3718
  //Configure pulser A for camera trigger
  //---Start and reset from SW
  CVTimeUnits unit = cvUnit410us;
  //DWORD period = 100000/410; //in units of 410 us
  //DWORD width = 10; //in units of 410 us
  unsigned char width = 1;
  unsigned char period = 2;
  
  CAENVME_SetPulserConf(gVme->handle,cvPulserA,period,width,unit,0,cvManualSW,cvManualSW);

  //TO BE CHECKD FOR V3718
  //Configure pulser B for a single gate
  //---Start from IN_0, reset from SW, infinite length
  unit = cvUnit25ns;
  period = 100; //in units of 104 ms 
  width = 255; //in units of 104 ms
  CAENVME_SetPulserConf(gVme->handle,cvPulserB,period,width,unit,0,cvInputSrc0,cvManualSW);

  return SUCCESS;
  
}
#endif

#ifdef HAVE_CAEN_DGTZ
INT ConfigDgtz(){


  cerr<<"configuring dgtz..."<<endl;
  int size = sizeof(int);

  HNDLE hDB;
  char query[64];
  int  maxtriggersize;
  cm_get_experiment_database(&hDB, NULL);
  
  db_get_value(hDB, 0,"/Configurations/MultiTriggerMaxSize",&maxtriggersize,&size,TID_INT,TRUE);

  //for_V1761
  for(int i=0;i<nboard;i++){

    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;

    sprintf(query,"/Configurations/DigitizerSamples[%d]",i);
    db_get_value(hDB, 0, query,&ndgtz[i],&size,TID_INT,TRUE);
    
    CAEN_DGTZ_DRS4Frequency_t DRS4Frequency = CAEN_DGTZ_DRS4_5GHz;

    if(strcmp(BoardName[i],"V1761")==0)   
      {
	if(ndgtz[i] > (7.2e6/maxtriggersize) ) ndgtz[i] = (int)(7.2e6/maxtriggersize);
	ret |= CAEN_DGTZ_SetRecordLength(gDGTZ[i],ndgtz[i]);                                /* Set the lenght of each waveform (in samples) */
	SAMPLING[i] = 4000;
      }
     //for_V1720E
    else if(strcmp(BoardName[i],"V1720E")==0)   
      {
	if(ndgtz[i] > (1.25e6/maxtriggersize) ) ndgtz[i] = (int)(1.25e6/maxtriggersize);                                         
	ret |= CAEN_DGTZ_SetRecordLength(gDGTZ[i],ndgtz[i]);                                /* Set the lenght of each waveform (in samples) */
	SAMPLING[i] = 250;
      }
    //#ifdef HAVE_V1742
    else if(strcmp(BoardName[i],"V1742")==0)     
      {
        ndgtz[i] = 1024;
        int fsampling = 0;
        sprintf(query,"/Configurations/SamplingFrequency[%d]",i);
        db_get_value(hDB, 0, query,&fsampling,&size,TID_INT,TRUE);
        switch(fsampling){
        case 0:
          SAMPLING[i] = (int)((1000.0/750.0)*1000.0);
                    DRS4Frequency=CAEN_DGTZ_DRS4_750MHz;		
                    break;
        case 1:
          SAMPLING[i] = (int)1000;
          DRS4Frequency=CAEN_DGTZ_DRS4_1GHz;		
          break;
        case 2:
          SAMPLING[i] = (int)400;
          DRS4Frequency=CAEN_DGTZ_DRS4_2_5GHz;			
          break;
        case 3:
          SAMPLING[i] = (int)200;
          DRS4Frequency=CAEN_DGTZ_DRS4_5GHz;			
          break;
        default:
          SAMPLING[i] = (int)200;
          DRS4Frequency=CAEN_DGTZ_DRS4_5GHz;			
          break;
        }
        ret |= CAEN_DGTZ_SetDRS4SamplingFrequency(gDGTZ[i],DRS4Frequency);
      }
    
    
    //#endif

    sprintf(query,"/Configurations/DigitizerPostTrg[%d]",i);
    db_get_value(hDB, 0, query,&posttrg[i],&size,TID_INT,TRUE);
    ret |= CAEN_DGTZ_SetPostTriggerSize(gDGTZ[i],posttrg[i]);                               /* Trigger position */
    
    //Print the firmware version
    /*
    CAEN_DGTZ_BoardInfo_t BoardInfo;

    for(int i=0;i<nboard;i++){
      CAEN_DGTZ_GetInfo(gDGTZ[i], &BoardInfo);
      std::cerr << "Digitizer " << i << " (" << BoardName[i] << ") - Firmware: " << BoardInfo.ROC_FirmwareRel << " / " << BoardInfo.AMC_FirmwareRel << std::endl;
    }
    */


    //VITO: ENABLING EGTTT 60 bit 
    for(int i=0;i<nboard;i++){
      // Read the register than turn on the bit 20
      uint32_t enable_egttt;
      CAEN_DGTZ_ReadRegister(gDGTZ[i], 0x8000, &enable_egttt);
      enable_egttt = enable_egttt | 0x00100000;  // bit 20
      CAEN_DGTZ_WriteRegister(gDGTZ[i], 0x8004, enable_egttt);
    }


    //////

    size = sizeof(double);
    for(int ich=0;ich<NCHDGTZ[i];ich++){
      
      sprintf(query,"/Configurations/DigitizerOffset[%d]",i*32+ich);      
      db_get_value(hDB,0,query,&DGTZ_OFFSET[i][ich],&size,TID_DOUBLE,TRUE);
      
      if(DGTZ_OFFSET[i][ich] > 0.5) DGTZ_OFFSET[i][ich] = 0.5;
      else if(DGTZ_OFFSET[i][ich] < -0.5) DGTZ_OFFSET[i][ich] = -0.5;
      
      if( (strcmp(BoardName[i],"V1761")==0) || (strcmp(BoardName[i],"V1720E")==0) )    
        ret |= CAEN_DGTZ_SetChannelDCOffset(gDGTZ[i],ich,(uint32_t)(DGTZ_OFFSET[i][ich]*65536 + 32767));
      else if(strcmp(BoardName[i],"V1742")==0){      //da decidere
        int grreg = 0x1098 | (ich/8 << 8);
        int data = (ich%8<<16) | (int)(DGTZ_OFFSET[i][ich]/2.*65536 + 32767);
        //CAEN_DGTZ_SetGroupDCOffset(gDGTZ[i],ich/8,(uint32_t)(DGTZ_OFFSET[i][ich]*65536 + 32767));
        ret |= CAEN_DGTZ_WriteRegister(gDGTZ[i],grreg,data);
		
      }
      
      //if(ret != CAEN_DGTZ_Success) {
      //   cout<<BoardName[i]<<"  "<<endl;
      //   printf("%d  %d  Errors during Digitizer Configuration.\n", i, ich);
      //}
	
    }
    
    //Uploading and enabling the automatic correction for the 1742 digitizer
    bool enable_corrections;
    size = 4*sizeof(bool);
    db_get_value(hDB,0,"/Configurations/DRS4Correction",&enable_corrections, &size, TID_BOOL,TRUE);
    
    CAEN_DGTZ_ErrorCode ret2;
    
    
    
    if(strcmp(BoardName[i],"V1742")==0)      //da decidere
      {
      	if(!enable_corrections) {
        //std::cout<<"DEBUG NO CORRECTION"<<std::endl;
		ret2 = CAEN_DGTZ_DisableDRS4Correction(gDGTZ[i]);
		if(ret2 != CAEN_DGTZ_Success) {
			cerr<<"Error in DisableDRS4Correction"<<endl;
		}
	} else {
      
		if ((ret2 = CAEN_DGTZ_LoadDRS4CorrectionData(gDGTZ[i],DRS4Frequency)) != CAEN_DGTZ_Success) 
		  { 
		    cerr<<"Error in LoadDRS4Correction"<<endl;
		    exit(EXIT_FAILURE);
		  } 
		if ((ret2 = CAEN_DGTZ_EnableDRS4Correction(gDGTZ[i])) != CAEN_DGTZ_Success)
		  { 
		    cerr<<"Error in EnableDRS4Correction"<<endl;
		    exit(EXIT_FAILURE);
		  }
        
        
        // Print correction tables
//         CAEN_DGTZ_DRS4Correction_t CTable[MAX_X742_GROUP_SIZE];
//         ret = CAEN_DGTZ_GetCorrectionTables(gDGTZ[i], DRS4Frequency, (void*)CTable);
//         if(ret != CAEN_DGTZ_Success) {
//             throw std::runtime_error("DEBUG ctables.\n");
//         } else {
//             SaveCorrectionTables("./ctables/ctables_DCO", (uint32_t)(pow(2,NCHDGTZ[i]))-1, CTable);
//         }
        
          
        }
      }

    else{

      //Calibration
      //ret |= CAEN_DGTZ_Calibrate(gDGTZ[i]);

    }
    

    //Buffer allocation
    uint32_t bsize;
    ret |= CAEN_DGTZ_MallocReadoutBuffer(gDGTZ[i],&buffer_dgtz[i],&bsize);

    if(ret != CAEN_DGTZ_Success) {
      printf("Errors during Digitizer Configuration.\n");
      //cm_msg(MERROR, "cygnus_daq", "Errors during Digitizer Configuration.");
      //throw runtime_error("Errors during Digitizer Digitizer Configuration.");
    }
    
  }//end for cycle on boards
  return SUCCESS;
  
}
#endif

#ifdef HAVE_V895
INT ConfigDisc(){

  HNDLE hDB;

  cm_get_experiment_database(&hDB, NULL);
  
  int thr;
  int size = sizeof(int);
  
  db_get_value(hDB, 0, "/Configurations/Threshold[0]",&thr,&size,TID_INT,TRUE);
  v895_writeReg16(gVme,gDisBase,0x00 ,thr);
  db_get_value(hDB, 0, "/Configurations/Threshold[1]",&thr,&size,TID_INT,TRUE);
  v895_writeReg16(gVme,gDisBase,0x02 ,thr);
  db_get_value(hDB, 0, "/Configurations/Threshold[2]",&thr,&size,TID_INT,TRUE);
  v895_writeReg16(gVme,gDisBase,0x04 ,thr);
  db_get_value(hDB, 0, "/Configurations/Threshold[3]",&thr,&size,TID_INT,TRUE);
  v895_writeReg16(gVme,gDisBase,0x06 ,thr);
  db_get_value(hDB, 0, "/Configurations/Threshold[4]",&thr,&size,TID_INT,TRUE);
  v895_writeReg16(gVme,gDisBase,0x08 ,thr);
  db_get_value(hDB, 0, "/Configurations/Threshold[5]",&thr,&size,TID_INT,TRUE);
  v895_writeReg16(gVme,gDisBase,0x0A ,thr);
  db_get_value(hDB, 0, "/Configurations/Threshold[6]",&thr,&size,TID_INT,TRUE);
  v895_writeReg16(gVme,gDisBase,0x0C ,thr);
  db_get_value(hDB, 0, "/Configurations/Threshold[7]",&thr,&size,TID_INT,TRUE);
  v895_writeReg16(gVme,gDisBase,0x0E ,thr);
  db_get_value(hDB, 0, "/Configurations/Threshold[8]",&thr,&size,TID_INT,TRUE);
  v895_writeReg16(gVme,gDisBase,0x10 ,thr);
  db_get_value(hDB, 0, "/Configurations/Threshold[9]",&thr,&size,TID_INT,TRUE);
  v895_writeReg16(gVme,gDisBase,0x12 ,thr);
  db_get_value(hDB, 0, "/Configurations/Threshold[10]",&thr,&size,TID_INT,TRUE);
  v895_writeReg16(gVme,gDisBase,0x14 ,thr);
  db_get_value(hDB, 0, "/Configurations/Threshold[11]",&thr,&size,TID_INT,TRUE);
  v895_writeReg16(gVme,gDisBase,0x16 ,thr);
  db_get_value(hDB, 0, "/Configurations/Threshold[12]",&thr,&size,TID_INT,TRUE);
  v895_writeReg16(gVme,gDisBase,0x18 ,thr);
  db_get_value(hDB, 0, "/Configurations/Threshold[13]",&thr,&size,TID_INT,TRUE);
  v895_writeReg16(gVme,gDisBase,0x1A ,thr);
  db_get_value(hDB, 0, "/Configurations/Threshold[14]",&thr,&size,TID_INT,TRUE);
  v895_writeReg16(gVme,gDisBase,0x1C ,thr);
  db_get_value(hDB, 0, "/Configurations/Threshold[15]",&thr,&size,TID_INT,TRUE);
  v895_writeReg16(gVme,gDisBase,0x1E ,thr);
  
  int wdt = 255;
  v895_writeReg16(gVme,gDisBase,0x40 ,wdt);
  v895_writeReg16(gVme,gDisBase,0x42 ,wdt);
  
  int maj = 2;
  v895_writeReg16(gVme,gDisBase,0x48 , round((maj*50-25)/4));
  
  int inib = 0xC000; //0b1100000000000000
  v895_writeReg16(gVme,gDisBase,0x4A ,inib);
  
  return 0;
  
}
#endif

#ifdef HAVE_CAMERA
INT ConfigCamera(int icam)
{
  
  cout<<"Configuring camera "<<icam<<" ... "<<endl;

  HNDLE hDB;

  cm_get_experiment_database(&hDB, NULL);

  //Set exposure time in seconds
  DCAMERR err;

  double exposure;
  int size = sizeof(double);
  db_get_value(hDB, 0, "/Configurations/Exposure",&exposure,&size,TID_DOUBLE,TRUE);


  

  //double exp_test;
  //dcamprop_getvalue( gCam[icam], DCAM_IDPROP_EXPOSURETIME, &exp_test);

  //cerr<<"DEBUG exposure before setup = "<<exp_test<<endl;
  
  //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + 256, exposure + 0.18);
  //if(failed(err)) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + step" << endl;

  int mode;
  size = sizeof(int);
  db_get_value(hDB, 0, "/Configurations/TriggerMode",&mode,&size,TID_INT,TRUE);
  if(mode==1) err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_TRIGGER_GLOBALEXPOSURE, DCAMPROP_TRIGGER_GLOBALEXPOSURE__GLOBALRESET); 
  else if(mode==2) err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_TRIGGER_GLOBALEXPOSURE, DCAMPROP_TRIGGER_GLOBALEXPOSURE__EMULATE); 
  else err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_TRIGGER_GLOBALEXPOSURE, DCAMPROP_TRIGGER_GLOBALEXPOSURE__DELAYED);


  dcamprop_setvalue( gCam[icam], DCAM_IDPROP_SENSORMODE, DCAMPROP_SENSORMODE__AREA);
  dcamprop_setvalue( gCam[icam], DCAM_IDPROP_READOUTSPEED, DCAMPROP_READOUTSPEED__SLOWEST);
  dcamprop_setvalue( gCam[icam], DCAM_IDPROP_INTERNAL_FRAMEINTERVAL, 0.033326);
  dcamprop_setvalue( gCam[icam], DCAM_IDPROP_BITSPERCHANNEL, 16);
  dcamprop_setvalue( gCam[icam], DCAM_IDPROP_BINNING, 1);
  dcamprop_setvalue( gCam[icam], DCAM_IDPROP_FRAMEBUNDLE_MODE,DCAMPROP_MODE__OFF);
  dcamprop_setvalue( gCam[icam], DCAM_IDPROP_DEFECTCORRECT_MODE, DCAMPROP_DEFECTCORRECT_MODE__OFF);
  dcamprop_setvalue( gCam[icam], DCAM_IDPROP_SPOTNOISEREDUCER, DCAMPROP_MODE__OFF);

  
  if(mode != 3) {
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_EXPOSURETIME, exposure);
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_EXPOSURETIME" << endl;
  }

  //dcamprop_getvalue( gCam[icam], DCAM_IDPROP_EXPOSURETIME, &exp_test);
  //cerr<<"DEBUG exposure after setup = "<<exp_test<<endl;

  //continous stream mode
  if(mode==3) {
    //if(icam == 0) {
      err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_TRIGGER_MODE, DCAMPROP_TRIGGER_MODE__NORMAL );
      if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGER_MODE" << endl;
    //} else {
    //  err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_TRIGGER_MODE, DCAMPROP_TRIGGER_MODE__NORMAL );
    //  if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGER_MODE" << endl;
    //}

  } else {
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_TRIGGER_MODE, DCAMPROP_TRIGGER_MODE__NORMAL );
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGER_MODE" << endl;
  }

  //Software trigger
//if(icam == 0) { //if camera is master then trigger is raised frim the DAQ software
    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_TRIGGERSOURCE, DCAMPROP_TRIGGERSOURCE__SOFTWARE );
    //if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGERSOURCE" << endl;
  //} else {*/ // if camera is not master trigger is raised by the hardware and sent to EXT. TRIG.
  if(mode == 3) {

    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_TRIGGERSOURCE, DCAMPROP_TRIGGERSOURCE__EXTERNAL );
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGERSOURCE" << endl;

    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_TRIGGERACTIVE, DCAMPROP_TRIGGERACTIVE__SYNCREADOUT );
    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_TRIGGERACTIVE, DCAMPROP_TRIGGERACTIVE__EDGE );
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGERACTIVE" << endl;


    long int triggertimes = (long int)(exposure/0.000820);
    //long int triggertimes = (long int)(exposure/0.001);
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_TRIGGERTIMES, triggertimes );
    cout<<"Trigger times configuration: "<<triggertimes<<endl;
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGERTIMES" << endl;


    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_TRIGGERPOLARITY, DCAMPROP_TRIGGERPOLARITY__POSITIVE);
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGERPOLARITY" << endl;

    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_EXPOSURETIME, exposure);
    //DCAMPROP_TRIGGERENABLE_POLARITY__POSITIVE

  //}
  //if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGERSOURCE" << endl;
  
  
  
  
    //Global exposure as output signal of OUT 1 with positive polarity
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_KIND, DCAMPROP_OUTPUTTRIGGER_KIND__EXPOSURE );
    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_KIND, DCAMPROP_OUTPUTTRIGGER_KIND__PROGRAMABLE );
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_KIND" << endl;
    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_SOURCE, DCAMPROP_OUTPUTTRIGGER_SOURCE__HSYNC);
    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_PERIOD, 0.039347);

    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_POLARITY, DCAMPROP_OUTPUTTRIGGER_POLARITY__POSITIVE );  
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_POLARITY" << endl;
    
    //Exposure as output signal of OUT 2 with positive polarity
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_KIND + 256, DCAMPROP_OUTPUTTRIGGER_KIND__PROGRAMABLE);
    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_KIND + 256, DCAMPROP_OUTPUTTRIGGER_KIND__TRIGGERREADY);
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_KIND + step" << endl;
      
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + 256, DCAMPROP_OUTPUTTRIGGER_SOURCE__TRIGGER);
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + step" << endl;
  
  
  
    //MY GET VALUE
    double delay;

    err = dcamprop_getvalue(gCam[icam], DCAM_IDPROP_TIMING_GLOBALEXPOSUREDELAY, &delay);
    if(failed(err)) cm_msg(MERROR, "cygnus_daq", "ConfigCamera error in get TIMING_GLOBALEXPOSUREDELAY");


    //dcamprop_getvalue( gCam[icam], DCAM_IDPROP_EXPOSURETIME, &exposure);

    //cout<<"DEBUG icam = "<<icam<<" - delay = "<<delay<<" - exposure = "<<exposure<<endl;
    
    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_TIMESTAMP_PRODUCER, DCAMPROP_TIMESTAMP_PRODUCER__IMAGINGDEVICE);
    //if(failed(err)) cm_msg(MERROR, "cygnus_daq", "ConfigCamera error in set TIMESTAMP_PRODUCER.");
    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_DEVICEBUFFER_MODE, DCAMPROP_DEVICEBUFFER_MODE__THRU);
    //if(failed(err)) cout << "ERROR IN DCAM_IDPROP_DEVICEBUFFER_MODE" << endl;
    
    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + 256, DCAMPROP_OUTPUTTRIGGER_SOURCE__TRIGGER);
    //if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + step" << endl;
      
    
    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_ACTIVE + 256, DCAMPROP_OUTPUTTRIGGER_ACTIVE__EDGE);
    //if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + step" << endl;  
  
    // OLD
    // err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + 256, exposure + delay);
    // if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + step" << endl;
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + 256, exposure+delay);
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + step" << endl;
      
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_POLARITY+256, DCAMPROP_OUTPUTTRIGGER_POLARITY__POSITIVE);  
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_POLARITY + step" << endl;

  } else if (mode == 0) {

    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_TRIGGERSOURCE, DCAMPROP_TRIGGERSOURCE__SOFTWARE );
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGERSOURCE" << endl;
  

    //Global exposure as output signal of OUT 1 with positive polarity
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_KIND, DCAMPROP_OUTPUTTRIGGER_KIND__EXPOSURE );
    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_KIND, DCAMPROP_OUTPUTTRIGGER_KIND__PROGRAMABLE );
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_KIND" << endl;
    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_SOURCE, DCAMPROP_OUTPUTTRIGGER_SOURCE__HSYNC);
    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_PERIOD, 0.039347);

    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_POLARITY, DCAMPROP_OUTPUTTRIGGER_POLARITY__POSITIVE );  
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_POLARITY" << endl;



    
    //Exposure as output signal of OUT 2 with positive polarity
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_KIND + 256, DCAMPROP_OUTPUTTRIGGER_KIND__PROGRAMABLE);
    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_KIND + 256, DCAMPROP_OUTPUTTRIGGER_KIND__TRIGGERREADY);
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_KIND + step" << endl;
      
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + 256, DCAMPROP_OUTPUTTRIGGER_SOURCE__TRIGGER);
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + step" << endl;
  
    //MY GET VALUE
    double delay;

    err = dcamprop_getvalue(gCam[icam], DCAM_IDPROP_TIMING_GLOBALEXPOSUREDELAY, &delay);
    if(failed(err)) cm_msg(MERROR, "cygnus_daq", "ConfigCamera error in get TIMING_GLOBALEXPOSUREDELAY");

    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + 256, exposure+delay);
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + step" << endl;

    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_POLARITY+256, DCAMPROP_OUTPUTTRIGGER_POLARITY__POSITIVE);  
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_POLARITY + step" << endl;
  }
  /*if(icam == 0) {  
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_POLARITY+256, DCAMPROP_OUTPUTTRIGGER_POLARITY__POSITIVE);  
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_POLARITY + step" << endl;
    
    
    // Exposure as output signal of OUT 3 with positive polarity change comment
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_KIND + 256*2, DCAMPROP_OUTPUTTRIGGER_KIND__PROGRAMABLE);
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_KIND + step" << endl;
    
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + 256*2, DCAMPROP_OUTPUTTRIGGER_SOURCE__VSYNC);
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + step" << endl;

    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + 256*2, delay);
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + step" << endl;
      
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_POLARITY+256*2, DCAMPROP_OUTPUTTRIGGER_POLARITY__POSITIVE);  
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_POLARITY + step" << endl;
  } else {

    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_POLARITY+256, DCAMPROP_OUTPUTTRIGGER_POLARITY__POSITIVE);  
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_POLARITY + step" << endl;
    
    
    // Exposure as output signal of OUT 3 with positive polarity change comment
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_KIND + 256*2, DCAMPROP_OUTPUTTRIGGER_KIND__PROGRAMABLE);
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_KIND + step" << endl;
    
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + 256*2, DCAMPROP_OUTPUTTRIGGER_SOURCE__TRIGGER);
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + step" << endl;

    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + 256*2, delay);
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + step" << endl;
      
    err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_POLARITY+256*2, DCAMPROP_OUTPUTTRIGGER_POLARITY__POSITIVE);  
    if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_POLARITY + step" << endl;


  }*/

  PrintCamConfig(icam);
  
  return 0;
  
}


INT PrintCamConfig(int icam) {

  DCAMERR err;

  cout<<"Configuration of camera "<<icam<<": "<<endl;

  double dVar;
  int    iVar;
  err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_EXPOSURETIME, &dVar);
  if(failed(err)) cout << "ERROR IN READOUT OF DCAM_IDPROP_EXPOSURETIME" << endl;
  cout << "DCAM_IDPROP_EXPOSURETIME = "<<dVar<<endl;


  err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_TRIGGER_GLOBALEXPOSURE, &dVar);
  if(failed(err)) cout << "ERROR IN READOUT OF DCAM_IDPROP_TRIGGER_GLOBALEXPOSURE" << endl;
  cout << "DCAM_IDPROP_TRIGGER_GLOBALEXPOSURE = "<<dVar<<endl;

  /*
  dcamprop_setvalue( gCam[icam], DCAM_IDPROP_SENSORMODE, DCAMPROP_SENSORMODE__AREA);
  dcamprop_setvalue( gCam[icam], DCAM_IDPROP_READOUTSPEED, DCAMPROP_READOUTSPEED__SLOWEST);
  dcamprop_setvalue( gCam[icam], DCAM_IDPROP_INTERNAL_FRAMEINTERVAL, 0.033326);
  dcamprop_setvalue( gCam[icam], DCAM_IDPROP_BITSPERCHANNEL, 16);
  dcamprop_setvalue( gCam[icam], DCAM_IDPROP_BINNING, 1);
  dcamprop_setvalue( gCam[icam], DCAM_IDPROP_FRAMEBUNDLE_MODE,DCAMPROP_MODE__OFF);
  dcamprop_setvalue( gCam[icam], DCAM_IDPROP_DEFECTCORRECT_MODE, DCAMPROP_DEFECTCORRECT_MODE__OFF);
  dcamprop_setvalue( gCam[icam], DCAM_IDPROP_SPOTNOISEREDUCER, DCAMPROP_MODE__OFF);
  */
  
  //continous stream mode
  if(icam == 0) {

    err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_TRIGGER_MODE, &dVar );
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGER_MODE" << endl;
    cout << "DCAM_IDPROP_TRIGGER_MODE = "<<dVar<<endl;

  } else {

    err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_TRIGGER_MODE, &dVar);
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGER_MODE" << endl;
    cout << "DCAM_IDPROP_TRIGGER_MODE = "<<dVar<<endl;

  }

  
  //Software trigger
  if(icam == 0) { //if camera is master then trigger is raised frim the DAQ software

    err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_TRIGGERSOURCE, &dVar );
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGERSOURCE" << endl;
    cout << "DCAM_IDPROP_TRIGGERSOURCE = "<<dVar<<endl;

  } else { // if camera is not master trigger is raised by the hardware and sent to EXT. TRIG.

    err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_TRIGGERSOURCE, &dVar );
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGERSOURCE" << endl;
    cout << "DCAM_IDPROP_TRIGGERSOURCE = "<<dVar<<endl;

    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_TRIGGERACTIVE, DCAMPROP_TRIGGERACTIVE__SYNCREADOUT );
    err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_TRIGGERACTIVE, &dVar );
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGERACTIVE" << endl;
    cout << "DCAM_IDPROP_TRIGGERACTIVE = "<<dVar<<endl;

    err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_TRIGGERPOLARITY, &dVar);
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGERPOLARITY" << endl;
    cout << "DCAM_IDPROP_TRIGGERPOLARITY = "<<dVar<<endl;

    //err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_EXPOSURETIME, exposure);
    //DCAMPROP_TRIGGERENABLE_POLARITY__POSITIVE

  }
  //if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGERSOURCE" << endl;
  
  
  //Global exposure as output signal of OUT 1 with positive polarity
  err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_KIND, &dVar );
  if(failed(err)) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_KIND" << endl;
  cout << "DCAM_IDPROP_OUTPUTTRIGGER_KIND = "<<dVar<<endl;

  err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_POLARITY, &dVar );  
  if(failed(err)) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_POLARITY" << endl;
  cout << "DCAM_IDPROP_OUTPUTTRIGGER_POLARITY = "<<dVar<<endl;
  

  //Exposure as output signal of OUT 2 with positive polarity
  err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_KIND + 256, &dVar);
  if(failed(err)) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_KIND + step" << endl;
  cout << "DCAM_IDPROP_OUTPUTTRIGGER_KIND + 256 = "<<dVar<<endl;
    

  err = dcamprop_getvalue(gCam[icam], DCAM_IDPROP_TIMING_GLOBALEXPOSUREDELAY, &dVar);
  if(failed(err)) cm_msg(MERROR, "cygnus_daq", "ConfigCamera error in get TIMING_GLOBALEXPOSUREDELAY");
  cout << "DCAM_IDPROP_TIMING_GLOBALEXPOSUREDELAY = "<<dVar<<endl;


  dcamprop_getvalue( gCam[icam], DCAM_IDPROP_EXPOSURETIME, &dVar);
  if(failed(err)) cm_msg(MERROR, "cygnus_daq", "ConfigCamera error in get DCAM_IDPROP_EXPOSURETIME");
  cout << "DCAM_IDPROP_EXPOSURETIME = "<<dVar<<endl;

  
  err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_TIMESTAMP_PRODUCER, &dVar);
  if(failed(err)) cm_msg(MERROR, "cygnus_daq", "ConfigCamera error in get TIMESTAMP_PRODUCER.");
  cout << "DCAM_IDPROP_TIMESTAMP_PRODUCER = "<<dVar<<endl;
  
  err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + 256, &dVar);
  if(failed(err)) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + step" << endl;
  cout << "DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + 256 = "<<dVar<<endl;
 
  // OLD
  // err = dcamprop_setvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + 256, exposure + delay);
  // if(failed(err) && DEBUG) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + step" << endl;
  err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + 256, &dVar);
  if(failed(err)) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + step" << endl;
  cout << "DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + 256 = "<<dVar<<endl;

  
  if(icam == 0) {  

    err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_POLARITY+256, &dVar);  
    if(failed(err) ) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_POLARITY + 2 step" << endl;
    cout << "DCAM_IDPROP_OUTPUTTRIGGER_POLARITY + 256 = "<<dVar<<endl;
    
    // Exposure as output signal of OUT 3 with positive polarity change comment
    err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_KIND + 256*2, &dVar);
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_KIND + 2 step" << endl;
    cout << "DCAM_IDPROP_OUTPUTTRIGGER_KIND + 2*256 = "<<dVar<<endl;
    
    err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + 256*2, &dVar);
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + 2 step" << endl;
    cout << "DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + 2*256 = "<<dVar<<endl;

    err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + 256*2, &dVar);
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + 2 step" << endl;
    cout << "DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + 2*256 = "<<dVar<<endl;
      
    err = dcamprop_getvalue( gCam[icam], DCAM_IDPROP_OUTPUTTRIGGER_POLARITY+256*2, &dVar);  
    if(failed(err)) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_POLARITY + 2 step" << endl;
    cout << "DCAM_IDPROP_OUTPUTTRIGGER_POLARITY + 2*256 = "<<dVar<<endl;

  }

  return 0;
}


#endif

INT disable_trigger()
{

#ifdef HAVE_CAEN_BRD

  //WRONG FOR V3718
  //SET OUT_1 to 0 (busy)
  CAENVME_ClearOutputRegister(gVme->handle,cvOut1Bit);
  //CAENVME_SetOutputRegister(gVme->handle,cvOut1Bit); 

#ifdef HAVE_CAEN_DGTZ
  for(int i=0;i<nboard;i++){
    CAEN_DGTZ_SWStopAcquisition(gDGTZ[i]);
  }
#endif

#endif 

  return 0;
  
}
 
INT enable_trigger()
{
  
  ClearDevice(true);

  //CAENVME_SetOutputRegister(gVme->handle,cvOut1Bit);

#ifdef HAVE_CAEN_DGTZ
  for(int i=0;i<nboard;i++){
    CAEN_DGTZ_SWStartAcquisition(gDGTZ[i]);
  }
#endif

  return 0;

}

INT ClearDevice(BOOL clear_dgtz_data)
{

#ifdef HAVE_CAEN_BRD

#ifdef HAVE_V1190
  v1190_SoftClear(gVme,gTdcBase);
#endif

// THE DGTZ AFFAIR:
HNDLE hDB;

cm_get_experiment_database(&hDB, NULL);

int mode;
int size = sizeof(int);
db_get_value(hDB, 0, "/Configurations/TriggerMode",&mode,&size,TID_INT,TRUE);

#ifdef HAVE_CAEN_DGTZ
  if (clear_dgtz_data) {
    for(int i=0;i<nboard;i++){
      CAEN_DGTZ_ClearData(gDGTZ[i]);
    }
  }
#endif

  //WRONG FOR V3718
  //SET OUT_1 to 1 (not busy)
  //CAENVME_ClearOutputRegister(gVme->handle,cvOut1Bit);  
  //CAENVME_ClearOutputRegister(gVme->handle,cvOut1Bit);

  //cerr<<"---> GATE SET TO 1"<<endl<<flush;

  //TO BE CHECKD FOR V3718
  //Reset GATE (pulser B)
  //CAENVME_StopPulser(gVme->handle,cvPulserB);

#endif

  return 0;
  
}

#ifdef HAVE_V1190
INT read_tdc(char *pevent) {

  int count, outWords=0;
  bool wrieGLHD_TR = true;
  const int kDataSize = 10000;
  DWORD data[kDataSize];
  DWORD* pdata32=NULL;

  v1190_EventRead(gVme, gTdcBase, data, &count);

  if (count > 0) {
    //TIME_STAMP(pevent) = (std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch())).count();
    bk_create(pevent, "TDC0", TID_DWORD/*TID_BITFIELD*/, &pdata32);

    bool skipEmptyBank=false;
    for (int i=0; i<count; i++) {
      int code = (data[i] & V1190_DATA_FLAG);

      if (data[i] == 0) { continue; }

      switch (code) {

      case V1190_DATA_FLAG_DATA:
	{
	  *pdata32++ = data[i];
	  ++outWords;
	  //std::cerr << "i "<<i<<" = "<< (0x7FFFF&data[i]) <<std::endl;
	}
	break;

      case V1190_DATA_FLAG_TDHD:
	{
	  int testEmptyBank = i+1;
	  if (testEmptyBank<count) {
	    if ( (data[testEmptyBank] & V1190_DATA_FLAG)==V1190_DATA_FLAG_TDTR ) { skipEmptyBank=true; }
	  }
	  if (!skipEmptyBank) { *pdata32++ = data[i]; ++outWords; }
	}
	break;
      case V1190_DATA_FLAG_GLHD:
	if (wrieGLHD_TR) { *pdata32++ = data[i]; ++outWords; }
	break;
      case V1190_DATA_FLAG_GLTR:
	if (wrieGLHD_TR) {
	  ++outWords;
	  if ( ((data[i] & 0x1FFFE0)>>5)==outWords ) { *pdata32++ = data[i]; }
	  else {
	    DWORD tmpData = (data[i] & 0xFFE0001F);
	    tmpData += (outWords<<5);
	    *pdata32++ = tmpData;
	  }
	}
	break;
      default:
	*pdata32++ = data[i];
	++outWords;
	break;

      }

      if (skipEmptyBank) {++i; skipEmptyBank=false; continue;}

    }

    bk_close(pevent, pdata32);

  }

  return 1;

}
#endif

#ifdef HAVE_CAEN_DGTZ
int read_dgtz(char* pevent){

  //cout<<"Start reading..."<<endl<<flush;

  uint32_t bsize;
  char * evtptr = NULL;
  uint32_t NumEvents;
  uint32_t events_max = 128;
  CAEN_DGTZ_EventInfo_t eventInfo;
    
    

  WORD* pdata16 = NULL;
  //TIME_STAMP(pevent) = (std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch())).count();
  bk_create(pevent, "DIG0", TID_WORD, &pdata16);
  
  std::vector<std::vector<uint32_t>> TRGTTAG(nboard);
  std::vector<std::vector<uint32_t>> TRGTTAG1(nboard);
  std::vector<int> EVTSNUM(nboard);
  std::vector<uint16_t> StartIndexCell(128);
    
  // DEBUG
  //ofstream myfile;
  //myfile.open("debug.txt", ios_base::app);
  //myfile << "read_dgtz ------"<<endl;
    
  
  for(int i=0;i<nboard;i++){
    //cerr<<"Start reading board i = "<<i<<"..."<<endl<<flush;
    int event_i = 0;  
      
    std::vector<uint32_t> tmp_trgttag(128);
    std::vector<uint32_t> tmp_trgttag1(128);
    uint64_t tmp_EGTT;
    
    CAEN_DGTZ_ReadData(gDGTZ[i],CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,buffer_dgtz[i],&bsize);
    CAEN_DGTZ_GetNumEvents(gDGTZ[i],buffer_dgtz[i],bsize,&NumEvents);
    NumEvents =std::min(NumEvents, events_max);  
    //myfile<<"i = "<<i<<" numevents = "<<NumEvents<<endl;
    //if(NumEvents != 1) cout << "---------- ERROR!!!!! DGTZ > 1 event!!! ----------" << endl;
    //cout << "NumEvents = " << NumEvents << endl;
    
    //#ifdef HAVE_V1761
    if(strcmp(BoardName[i],"V1761")==0 || strcmp(BoardName[i],"V1720E")==0){
      
      CAEN_DGTZ_UINT16_EVENT_t *Evt = NULL;

      for(int iev=0;iev<NumEvents;iev++){

        CAEN_DGTZ_AllocateEvent(gDGTZ[i], (void**)&Evt);

        CAEN_DGTZ_GetEventInfo(gDGTZ[i],buffer_dgtz[i],bsize,iev,&eventInfo,&evtptr);
        CAEN_DGTZ_DecodeEvent(gDGTZ[i],evtptr,(void**)&Evt);

        tmp_trgttag[iev] = eventInfo.TriggerTimeTag; // TO BE CHECKED ON x761 and x720
        //tmp_trgttag[event_i] = eventInfo.TriggerTimeTag; // TO BE CHECKED ON x761 and x720
          //event_i ++;

        for(int j=0;j<NCHDGTZ[i];j++){

          for (uint32_t k=0; k<ndgtz[i]; ++k) {

            uint16_t temp = (uint16_t)(Evt->DataChannel[j][k]);
            *pdata16++ = temp;

          }

        }

        CAEN_DGTZ_FreeEvent(gDGTZ[i],&Evt);

      }

    }

    //#ifdef HAVE_V1742       ///////////////////////// VITO : MODIFICO QUI
    else if(strcmp(BoardName[i],"V1742")==0){

      
      CAEN_DGTZ_X742_EVENT_t *Evt = NULL;


      //cerr<<"    NumEvents = "<<NumEvents<<endl<<flush;

      for(int iev=0;iev<NumEvents;iev++){

        //cerr<<"    Allocating Evt iev = "<<iev<<endl<<flush;
        CAEN_DGTZ_ErrorCode retall = CAEN_DGTZ_AllocateEvent(gDGTZ[i], (void**)&Evt);
        if(retall != CAEN_DGTZ_Success) {
          cerr <<"Error allocating event. ErrorCode = "<<retall<<endl<<flush;
          //return TRUE;
        }
        //cerr<<"    Getting info Evt iev = "<<iev<<endl<<flush;				

        if( Evt == NULL) cerr<<"Event from DGTZ is NULL."<<endl<<flush;

        CAEN_DGTZ_GetEventInfo(gDGTZ[i],buffer_dgtz[i],bsize,iev,&eventInfo,&evtptr);
        //cerr<<"    Decoding Evt iev = "<<iev<<endl<<flush;				

        CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_DecodeEvent(gDGTZ[i],evtptr,(void**)&Evt);
        if(ret != CAEN_DGTZ_Success) cerr <<"    FLAG NO SUCCESSO"<<endl<<flush;

        //cerr<<"    Saving TTTs and STIs for Evt iev= "<<iev<<endl<<flush;	
        tmp_trgttag[iev]    = Evt->DataGroup[0].TriggerTimeTag & 0x3FFFFFFF;
        tmp_trgttag1[iev]    = Evt->DataGroup[1].TriggerTimeTag & 0x3FFFFFFF;

        tmp_EGTT = (tmp_trgttag1[iev] << 30) | tmp_trgttag[iev];
        // DEBUG
        //cerr<<"------------------------------"<<endl<<flush;
        //cerr<<"old TTT is:   "<< tmp_trgttag[iev] * 8.5 * 1e-6 <<endl<<flush;
        //cerr<<"The EGTTT is: "<< tmp_EGTT * 8.5 * 1e-6 <<endl<<flush;
        //cerr<<"------------------------------"<<endl<<flush;

        StartIndexCell[iev] = Evt->DataGroup[0].StartIndexCell;
        //StartIndexCell[iev] = Evt->DataGroup[1].StartIndexCell;
        //tmp_trgttag[event_i] = Evt->DataGroup[0].TriggerTimeTag;
          //event_i++;

        //cerr<<"    Saving event on pdata16 = "<<iev<<endl<<flush;
        for(int j=0;j<NCHDGTZ[i];j++){

          uint32_t ig = j/8;
          uint32_t ich = j%8;

          for (uint32_t k=0; k<ndgtz[i]; ++k) {

            uint16_t temp = (uint16_t)(Evt->DataGroup[ig].DataChannel[ich][k]);
            *pdata16++ = temp;

          }

        }
        //cerr<<"    Freeing DGTZ Evts..."<<endl<<flush; // DEBUG
        CAEN_DGTZ_FreeEvent(gDGTZ[i],&Evt);
        //cerr<<"    DGTZ Evt freed..."<<endl<<flush;

      }
    } 

    //#endif 

    TRGTTAG[i] = tmp_trgttag;
    TRGTTAG1[i] = tmp_trgttag1;
    EVTSNUM[i] = NumEvents; //event_i;
    
    //cout<<"       "<< endl<<endl<<endl<<endl;
    //cout<<atoi(&BoardName[i][1])<<endl;
    //for(unsigned int j=0; j<EVTSNUM[i]; j++) {
    //  cout<<TRGTTAG[i][j]<<" "<<endl;
    //}
    //if(i==1) throw std::runtime_error("DEBUG");
    
    
  }//end for on boards for reading data

  //cerr<<"Closing DIG0 bank..."<<endl<<flush;

  bk_close(pevent, pdata16);


  //cerr<<"DIG0 bank closed"<<endl<<flush;
    
  uint32_t* hdata = NULL;
  uint32_t header_data = 0;
  //TIME_STAMP(pevent) = (std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch())).count();
  bk_create(pevent, "DGH0", TID_DWORD, (void **)&hdata);

  header_data = nboard;
  *hdata++ = header_data;

  for(int i=0;i<nboard;i++){

    // uint32_t* hdata = NULL;
    // sprintf(query,"DGH%d",i);
    //bk_create(pevent, query, TID_DWORD, (void **)&hdata);
    
    // uint32_t header_data = 0;
    
    header_data = atoi(&BoardName[i][1]);
    *hdata++ = header_data;
    
    header_data = ndgtz[i];
    *hdata++ = header_data;
    
    header_data = NCHDGTZ[i];
    *hdata++ = header_data;
    
    header_data = EVTSNUM[i];
    *hdata++ = header_data;
    
    CAEN_DGTZ_BoardInfo_t BoardInfo;
    CAEN_DGTZ_GetInfo(gDGTZ[i], &BoardInfo);

    header_data = (uint32_t)pow(2,BoardInfo.ADC_NBits);
    *hdata++ = header_data;
    
    header_data = SAMPLING[i];
    *hdata++ = header_data;
    
    for(int j=0;j<NCHDGTZ[i];j++){
      header_data = (uint32_t)(DGTZ_OFFSET[i][j]*65536 + 32768);
      *hdata++ = header_data;
    }
    
    //cout<<"=========="<<endl;
    for(unsigned int j=0; j<EVTSNUM[i]; j++) {
      //cout<<TRGTTAG[i][j]<<" "<<endl;
      *hdata++ = TRGTTAG[i][j];
    }

    for(unsigned int j=0; j<EVTSNUM[i]; j++) {
      //cout<<TRGTTAG[i][j]<<" "<<endl;
      *hdata++ = TRGTTAG1[i][j];
    }

    //se ora faccio TRGTTAG con todo list di update cygnolib e ora si legge bank come raw
    
    if(strcmp(BoardName[i], "V1742")==0) {
    	for(unsigned int j=0; j<EVTSNUM[i]; j++) {
      		*hdata++ = (uint32_t)StartIndexCell[j];
    	}
    }
    
  }//end for on boards for header
  
  
  //myfile.close();  //debug
    
  bk_close(pevent, hdata);

  //throw std::runtime_error("DEBUG");

  return 0;

}
#endif

#ifdef HAVE_CAMERA
INT read_camera(char *pevent, int icam)
{

  // transferinfo param
  
  DCAMERR err;
  DCAMCAP_TRANSFERINFO captransferinfo;
  
  /*if(DEBUG) {
	memset( &captransferinfo, 0, sizeof(captransferinfo) );
	captransferinfo.size	= sizeof(captransferinfo);

	// get number of captured image
	err = dcamcap_transferinfo( gCam[icam], &captransferinfo );
	if(failed(err)) throw runtime_error("read_camera: unable to get captransferinfo.\n");
	}*/
  memset( &captransferinfo, 0, sizeof(captransferinfo) );
  captransferinfo.size    = sizeof(captransferinfo);
  err = dcamcap_transferinfo( gCam[icam], &captransferinfo );
  if(failed(err)) throw runtime_error("read_camera: unable to get captransferinfo.\n");

  DCAMBUF_FRAME bufframe;
  memset( &bufframe, 0, sizeof(bufframe) );
  bufframe.size= sizeof(bufframe);
  bufframe.iFrame = -1;

  //////Read the data
  dcambuf_lockframe( gCam[icam], &bufframe );


  
  //////Create the bank
  WORD* pdata = NULL;
  
  uint64_t timetot = (std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch())).count();
  if (picIndex == 0) {
    timeZero = (DWORD)(timetot/1000);
  }
  timetot -= (uint64_t)timeZero*(uint64_t)1000;

  TIME_STAMP(pevent) = (unsigned int)timetot;
  
  //cout<<"DEBUG"<<picIndex<<"---"<<timeZero<<","<<timetot<<","<<(unsigned int)timetot<<endl;
  if (picIndex==0 && icam == 0) {
    DWORD *ptime =NULL;
    bk_create(pevent, "TIME", TID_DWORD, &ptime);
    *ptime++ = timeZero;
    bk_close(pevent, ptime);
  }
  
  picIndex++;
  
  //std::cout<<"DEBUG: "<<(std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch())).count()<<std::endl;
  bk_create(pevent, ("CAM"+to_string(icam)).c_str(), TID_WORD, &pdata);
    
    
  std::vector<int> picture(10);

  //////Copy data into the bank
  /*
  WORD* pSrc = (WORD*)bufframe.buf;
  WORD* pDst = new WORD[bufframe.width*bufframe.height];
  
  for( y = 0; y < bufframe.height; y++ ){
    
    //Copy one row
    memcpy_s( _secure_ptr( pDst, bufframe.rowbytes ), pSrc, bufframe.rowbytes );
    
    //go through the row
    for( x = 0; x < bufframe.width; x++ ){
      
      WORD tmpData = *pDst++;
      *pdata++ = tmpData;
      
    }
    
    pSrc += bufframe.rowbytes;
    
  }
  */
  
  /*
  if(DEBUG) {
	  ofstream myfile;
	  myfile.open("debug.txt", ios_base::app);
	  
	  DCAM_TIMESTAMP timestamp = bufframe.timestamp;
	  uint32_t secs = timestamp.sec;
	  uint32_t microsecs = timestamp.microsec;
	  
	  myfile<<"TIME STAMP = "<<secs<<" sec ; "<<microsecs<<" microsecs"<<endl;
	  myfile<<"NFRAMES = "<<captransferinfo.nFrameCount<<endl;
	  myfile.close();
  }
  */
  

  const char* pSrc = (const char*)bufframe.buf;

  //////FULL MATRIX
  for(int y = 0; y < bufframe.height; y++ ){

    //Copy one row
    const unsigned short* pDst = (const unsigned short*)pSrc;
    //if(y < 10) picture[y] = *pDst;

    //go through the row
    for(int x = 0; x < bufframe.width; x++ ){

      WORD tmpData = *pDst++; 
      *pdata++ = tmpData;
      //

    }

    pSrc += bufframe.rowbytes;

  }

  
  
  bk_close(pevent, pdata);
  
  
  // check cache to find duplicates
  /*bool check = TRUE;
  for(int kk = 0; kk < 10; kk++) {
  	if(picture[kk]!=cache_cam[kk]) {
  		check = FALSE;
  	}
  	cache_cam[kk] = picture[kk];
  }
  
  if(check) {
  	cm_msg(MERROR, "read_camera", "Two identical pics in event %d. pdata = [%d, %d, ...]", rec_ev, cache_cam[0], cache_cam[1]);
  	//myfile << "Two identical pics in event %d. pdata = ["<< cache_cam[0]<<", "<< cache_cam[1]<<", ...]"<<endl;
  	
  	
  	//throw std::runtime_error("Two identical pics.");
  }*/
  
  
   
  /*
  unsigned short int threshold = 0;
  
  //////SPARSE MATRIX
  for(unsigned short int y = 0; y < bufframe.height; y++ ){

    //Copy one row
    const unsigned short* pDst = (const unsigned short*)pSrc;
    
    //go through the row
    for(unsigned short int x = 0; x < bufframe.width; x++ ){

      WORD tmpData = *pDst++; 
      
      if(tmpData > threshold){
	
	*pdata++ = x;
	*pdata++ = y;
	*pdata++ = tmpData;
	
      }
      
    }

    pSrc += bufframe.rowbytes;

  }
  */
  
    /////timestamp from camera now
    DCAM_TIMESTAMP timestamp = bufframe.timestamp;
    DWORD *ptsp =NULL;
    bk_create(pevent, ("TSP"+to_string(icam)).c_str(), TID_DWORD, &ptsp);
    *ptsp++ = (DWORD)timestamp.sec;
    *ptsp++ = (DWORD)timestamp.microsec;
    bk_close(pevent, ptsp);

    /////frame index
    DWORD *pfid =NULL;
    bk_create(pevent, ("FID"+to_string(icam)).c_str(), TID_DWORD, &pfid);
    *pfid++ = (DWORD)captransferinfo.nFrameCount;//bufframe.iFrame;
    bk_close(pevent, pfid);

    // DEBUG
    //cerr<<"nFrameCount = "<<captransferinfo.nFrameCount<<" - Framestamp "<<bufframe.framestamp<<" - newestframeindex = "<<captransferinfo.nNewestFrameIndex<<endl;
  
  //dcambuf_release(gCam[icam] );

  return 1;

}


#endif

#ifdef HAVE_CAEN_DGTZ
void Free_arrays(){

  delete[] gDGTZ;
  delete[] gDigBase;
  delete[] NCHDGTZ;
  delete[] ndgtz;
  delete[] SAMPLING;
  delete[] posttrg;
  for(int i=0;i<nboard;i++){
    delete[] BoardName[i];
    delete[] buffer_dgtz[i];      //This may raise a break for multiple free of memory, in case just comment this line
    delete[] DGTZ_OFFSET[i];
  }
  delete[]  BoardName;
  delete[]  buffer_dgtz;
  delete[]  DGTZ_OFFSET;
}
#endif







#ifdef HAVE_CAEN_DGTZ
int SaveCorrectionTables(char *outputFileName, uint32_t groupMask, CAEN_DGTZ_DRS4Correction_t *tables) {
    char fnStr[MAX_BASE_INPUT_FILE_LENGTH + 1];
    int ch,i,j, gr;
    FILE *outputfile;

    if((int)(strlen(outputFileName) - 17) > MAX_BASE_INPUT_FILE_LENGTH)
        return -1; // Too long base filename
    
    std::cout<<"DEBUG MAX_X742_GROUP_SIZE = "<<MAX_X742_GROUP_SIZE<<std::endl<<std::flush;
    
    for(gr = 0; gr < MAX_X742_GROUP_SIZE; gr++) {
        std::cout<<"DEBUG (start) gr = "<<gr<<std::endl<<std::flush;
        CAEN_DGTZ_DRS4Correction_t *tb;

        if(!((groupMask>>gr)&0x1))
            continue;
        tb = &tables[gr];
        sprintf(fnStr, "%s_gr%d_cell.txt", outputFileName, gr);
        printf("Saving correction table cell values to %s\n", fnStr);
        if((outputfile = fopen(fnStr, "w")) == NULL)
            return -2;
        for(ch=0; ch<MAX_X742_CHANNEL_SIZE; ch++) {
            fprintf(outputfile, "Calibration values from cell 0 to 1024 for channel %d:\n\n", ch);
            for(i=0; i<1024; i+=8) {
                for(j=0; j<8; j++)
                    fprintf(outputfile, "%d\t", tb->cell[ch][i+j]);
                fprintf(outputfile, "cell = %d to %d\n", i, i+7);
            }
        }
        fclose(outputfile);
        
        sprintf(fnStr, "%s_gr%d_nsample.txt", outputFileName, gr);
        printf("Saving correction table nsamples values to %s\n", fnStr);
        if((outputfile = fopen(fnStr, "w")) == NULL)
            return -3;
        for(ch=0; ch<MAX_X742_CHANNEL_SIZE; ch++) {
            fprintf(outputfile, "Calibration values from cell 0 to 1024 for channel %d:\n\n", ch);
            for(i=0; i<1024; i+=8) {
                for(j=0; j<8; j++)
                    fprintf(outputfile, "%d\t", tb->nsample[ch][i+j]);
                fprintf(outputfile, "cell = %d to %d\n", i, i+7);
            }
        }
        fclose(outputfile);

        sprintf(fnStr, "%s_gr%d_time.txt", outputFileName, gr);
        printf("Saving correction table time values to %s\n", fnStr);
        if((outputfile = fopen(fnStr, "w")) == NULL)
            return -4;
        fprintf(outputfile, "Calibration values (ps) from cell 0 to 1024 :\n\n");
        for(i=0; i<1024; i+=8) {
            for(ch=0; ch<8; ch++)
                fprintf(outputfile, "%09.3f\t", tb->time[i+ch]);
            fprintf(outputfile, "cell = %d to %d\n", i, i+7);
        }
        fclose(outputfile);
    }
    
    return 0;
}
#endif
