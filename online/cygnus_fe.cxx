/********************************************************************	\

  Name:         cygnus_fe.cxx
  Created by:   Francesco Renga
  Edited by:    Stefano Piacentini

  Latest modification: 2026-07-08

  Contents: Frontend program for CYGNO-04

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
#include <cstring>
#include <omp.h>
// Debug
#include <bitset>

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
BOOL frontend_call_loop = TRUE;

/* a frontend status page is displayed with this frequency in ms */
INT display_period = 3000;

/* maximum event size produced by this frontend */
INT max_event_size = 150000000; //1000000000;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 5 * 1024 * 1024;

/* buffer size to hold events */
INT event_buffer_size = 1000000000; //2000000000

/* stop of run requested only if not already requested */
BOOL stop_already_requested = FALSE;

/* fatal camera error to skip cleanup */
BOOL fatal_camera_error = FALSE;

/* latch fatal camera error -> stop run from frontend_loop() */
BOOL stop_requested = FALSE;
BOOL stop_sent = FALSE;
DWORD stop_error_code = 0;

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

#ifdef HAVE_CAEN_DGTZ

static constexpr uint32_t DGTZ_ACQ_STATUS      = CAEN_DGTZ_ACQ_STATUS_ADD; // usually 0x8104
static constexpr uint32_t DGTZ_ACQ_CONTROL     = 0x8100;

// x742/V1742 group registers
static inline uint32_t V1742_GROUP_STATUS(int gr)    { return 0x1088 + 0x100 * gr; }
static inline uint32_t V1742_GROUP_OCCUPANCY(int gr) { return 0x1094 + 0x100 * gr; }

static CAEN_DGTZ_ErrorCode ReadRegisterRetry(
    int handle,
    uint32_t addr,
    uint32_t* value,
    int ntry = 3,
    useconds_t delay_us = 100
) {
    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_CommError;

    for (int itry = 0; itry < ntry; ++itry) {
        ret = CAEN_DGTZ_ReadRegister(handle, addr, value);
        if (ret == CAEN_DGTZ_Success)
            return ret;

        usleep(delay_us);
    }

    return ret;
}

static bool ReadV1742GroupBusyOrFull(int handle, int board, bool verbose = false)
{
    bool busy_or_full = false;

    for (int gr = 0; gr < 4; ++gr) {
        uint32_t gs = 0;
        uint32_t occ = 0;

        CAEN_DGTZ_ErrorCode ret1 = ReadRegisterRetry(handle, V1742_GROUP_STATUS(gr), &gs);
        CAEN_DGTZ_ErrorCode ret2 = ReadRegisterRetry(handle, V1742_GROUP_OCCUPANCY(gr), &occ);

        if (ret1 != CAEN_DGTZ_Success || ret2 != CAEN_DGTZ_Success) {
            cerr << "board " << board
                 << " group " << gr
                 << " status/occupancy read failed: ret1=" << ret1
                 << " ret2=" << ret2 << endl << flush;

            // Treat unreadable group status as unsafe for readout.
            return true;
        }

        bool mem_full = gs & (1u << 0);
        bool mem_empty = gs & (1u << 1);
        bool drs_busy = gs & (1u << 8);

        if (verbose) {
            cerr << "board " << board
                 << " group " << gr
                 << " GSTAT=0x" << hex << gs << dec
                 << " DRS_BUSY=" << drs_busy
                 << " MEM_FULL=" << mem_full
                 << " MEM_EMPTY=" << mem_empty
                 << " OCC=" << (occ & 0x7ff)
                 << endl << flush;
        }

        if (mem_full || drs_busy)
            busy_or_full = true;
    }

    return busy_or_full;
}

bool IsSupportedDigitizer(const char *model)
{
  return strcmp(model, "V1742") == 0 || strcmp(model, "V1720E") == 0;
}

bool IsV1742(const char *model)
{
  return strcmp(model, "V1742") == 0;
}

bool IsV1720E(const char *model)
{
  return strcmp(model, "V1720E") == 0;
}
#endif

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
INT read_camera(char *pevent, int icam, bool crop_image, int crop_size, int crop_origin_x, int crop_origin_y);
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
    cerr<<"frontend_init: ncamera = "<<nCamera<<" read by ODB is invalid. Please check."<<endl;
    cm_msg(MERROR, "cygnus_daq", ("frontend_init: ncamera = "+to_string(nCamera)+" read by ODB is invalid. Please check.\n").c_str());
    exit(EXIT_FAILURE);
  } else if (nCamera > NCAM_MAX) {
    cerr<<"frontend_init: ncamera = "<<nCamera<<" read by ODB is greater than maximum value ("<<NCAM_MAX<<"). Please check."<<endl;
    cm_msg(MERROR, "cygnus_daq", ("frontend_init: ncamera = "+to_string(nCamera)+
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
      cerr<<"frontend_init: Masks in the ODB are not consistent with input nCamera options. Please check /Equipment/Trigger/Settings/. Closing cygnus_fe."<<endl;
      return CM_SET_ERROR;
      //throw runtime_error("frontend_init: Masks in the ODB are not consistent with input nCamera options. Please check /Equipment/Trigger/Settings/. Closing cygnus_fe.\n");
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
  for (int icam = 0; icam < nCamera; icam++) {
    if (gCam[icam]) {
        dcamdev_close(gCam[icam]);
        gCam[icam] = NULL;
    }
  }
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
  stop_already_requested = FALSE;
  stop_requested = FALSE;
  stop_sent = FALSE;
  fatal_camera_error = FALSE;
  stop_error_code = 0;
  //uint32_t zero = 0;

#ifdef HAVE_CAMERA
  // DEBUG: TO BE FIXED
  for (int icam = 0; icam < nCamera; icam++) {
      if (!CamMask[icam]) continue;

      cerr << "DEBUG: begin_of_run: reinitializing camera..." << endl;

      cerr << "DEBUG: begin_of_run: closing wait..." << endl;
      // 1. close wait
      if (hwait[icam]) {
          dcamwait_close(hwait[icam]);
          hwait[icam] = NULL;
      }

      cerr << "DEBUG: begin_of_run: closing camera..." << endl;
      // 2. close camera
      if (gCam[icam] != NULL) {
          dcamdev_close(gCam[icam]);
          gCam[icam] = NULL;
      }

      cerr << "DEBUG: begin_of_run: reopen camera..." << endl;
      // 3. reopen
      gCam[icam] = dcamcon_init_open(true);
      if (gCam[icam] == NULL) {
          cm_msg(MERROR, "cygnus_daq", "begin_of_run: Failed to reopen camera");
          return FE_ERR_HW;
      }
  }
#endif

  HNDLE hDB;
  cm_get_experiment_database(&hDB, NULL);
  int mode;

  // reset chiavi ODB usate dall'alarm/restart logic
  INT8 zero = 0;

  INT status1 = db_set_value(hDB, 0, "/Configurations/RunControl/StopRequested",
                             &zero, sizeof(zero), 1, TID_INT8);

  const char* empty_reason = "";
  INT status2 = db_set_value(hDB, 0, "/Configurations/RunControl/StopReason",
                             empty_reason, 1, 1, TID_STRING);

  if (status1 != DB_SUCCESS || status2 != DB_SUCCESS) {
    cm_msg(MERROR, "cygnus_daq",
           "begin_of_run: failed to reset ODB stop flags: StopRequested=%d StopReason=%d",
           status1, status2);

    if (error)
      strcpy(error, "Cannot reset /Configurations/RunControl stop flags");

    return FE_ERR_ODB;
  }

  // reset esplicito dell'allarme se e' ancora attivo
  INT status3 = al_reset_alarm("Run Autorestart");
  if (status3 != AL_RESET && status3 != AL_SUCCESS) {
    cm_msg(MERROR, "cygnus_daq",
           "begin_of_run: al_reset_alarm failed, status=%d", status3);
  }

  // cm_msg(MINFO, "cygnus_daq",
  //        "begin_of_run: reset /Configurations/RunControl stop flags");

  
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
      // exit(EXIT_FAILURE);
      //set_equipment_status("Trigger", "Camera not found", "red");
      if(error) {
        strcpy(error, "Camera not found");
      }
      return FE_ERR_HW;
    }
  }


  // Auto restart: commented because it must be intensively tested
  // For solution #2, see the poll_event function
  //db_set_value(hDB, 0, "/Logger/Run duration", &zero, sizeof(zero), 1, TID_UINT32);

  // For solution #2, ---
  //BOOL pending = FALSE;
  //INT sp = sizeof(pending);
  //db_get_value(hDB, 0, "/Equipment/Trigger/Variables/_PendingAutoRestart", &pending, &sp, TID_BOOL, TRUE);
  //if (pending) { // if pending flag this new run as "autorestarted"

  //  // Set AutoRestarted flag
  //  BOOL v = TRUE;
  //  db_set_value(hDB, 0, "Equipment/Trigger/Variables/AutoRestarted", &v, sizeof(v), 1, TID_BOOL);

  //  // Set AutoRestart counter
  //  DWORD arcount = 0;
  //  INT sc = sizeof(arcount);
  //  db_get_value(hDB, 0, "Equipment/Trigger/Variables/AutoRestartCount", &arcount, &sc, TID_DWORD, TRUE);
  //  arcount += 1;
  //  db_set_value(hDB, 0, "Equipment/Trigger/Variables/AutoRestartCount", &arcount, sc, 1, TID_DWORD);

  //  // Reset Pending flag
  //  BOOL v2 = FALSE;
  //  db_set_value(hDB, 0, "/Equipment/Trigger/Variables/_PendingAutoRestart", &v2, sizeof(v2), 1, TID_BOOL);

  //} else { // if not pending: normal begin of run

  //  // Run not autorestarted
  //  BOOL v = FALSE;
  //  db_set_value(hDB, 0, "Equipment/Trigger/Variables/AutoRestarted", &v, sizeof(v), 1, TID_BOOL);

  //  // AutoRestart counter is zero
  //  DWORD arcount = 0;
  //  db_set_value(hDB, 0, "Equipment/Trigger/Variables/AutoRestartCount", &arcount, sizeof(arcount), 1, TID_DWORD);
  //}

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
    cm_msg(MERROR, "cygnus_daq", "Please check camera masks.");
    cerr<<"begin_of_run: Masks in the ODB are not consistent with input nCamera. Please check /Equipment/Trigger/Settings/. Closing cygnus_fe."<<endl;
    return CM_SET_ERROR;
    //throw runtime_error("begin_of_run: Masks in the ODB are not consistent with input nCamera. Please check /Equipment/Trigger/Settings/. Closing cygnus_fe.");
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
    if(failed(err)) {
      cm_msg(MERROR, "cygnus_daq", "begin_of_run: unable to open camera wait handle for camera %d.", icam);
      cerr<<"begin_of_run: unable to open camera wait handle for camera "<<icam<<endl;
      //exit(EXIT_FAILURE);

      //set_equipment_status("Trigger", "Unable to open camwait handle", "red");
      if(error) {
        strcpy(error, "Unable to open camwait handle");
      }
      return FE_ERR_HW;
      //throw runtime_error(("unable to open camera wait handle for camera"+to_string(icam)+".\n").c_str());
    }
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
  cerr<<"DEBUG: Ending the run ..."<<endl<<flush;
  stop_requested = FALSE;
  stop_sent = FALSE;
  stop_already_requested = FALSE;
  stop_error_code = 0;
  
  #ifdef HAVE_CAMERA

  // Stop camera acquisition and release resources
  for(int icam =0; icam<nCamera;icam++) {
    // Check camera mask flag
    if(!CamMask[icam]) continue;

    // [Fixme: check order]
    //cerr<<"DEBUG: releasing buffer frame..."<<endl<<flush;
    //dcambuf_release( gCam[icam] );
    //cerr<<"DEBUG: closing waiting camera handle..."<<endl<<flush;
    //dcamwait_close( hwait[icam] );
    //cerr<<"DEBUG: stopping camera ..."<<endl<<flush;
    //dcamcap_stop( gCam[icam] );
    
    // DEBUG ONLY: TO BE FIXED FOR REAL DATATAKING
    if(!fatal_camera_error) {
        cerr<<"DEBUG: aborting wating handle ..."<<endl<<flush;
        dcamwait_abort( hwait[icam]);
        cerr<<"DEBUG: stopping camera ..."<<endl<<flush;
        dcamcap_stop( gCam[icam] );
        cerr<<"DEBUG: closing waiting camera handle..."<<endl<<flush;
        dcamwait_close( hwait[icam] );
        cerr<<"DEBUG: releasing buffer frame..."<<endl<<flush;
        dcambuf_release( gCam[icam] );
        cerr<<"DEBUG: Done."<<endl<<flush;
    } else {
      cerr<<"DEBUG: camera fatal error, skipping cleanup"<<endl<<flush;
    }
    
    //gCam[0] = dcamcon_init_open(true);
  }

#endif

  cerr<<"DEBUG: disabling trigger..."<<endl<<flush;

  // Disable trigger at the end of the run
  disable_trigger();
  cerr<<"DEBUG: Done"<<endl<<flush;

#ifdef HAVE_CAEN_BRD
  // Remember: WRONG FOR V3718
  // Stop pulser at the end of the run
  cerr<<"DEBUG: stopping pulser..."<<endl<<flush;
  CAENVME_StopPulser(gVme->handle,cvPulserA);
  cerr<<"DEBUG: Done"<<endl<<flush;
#endif

  HNDLE hDB;
  cm_get_experiment_database(&hDB, NULL);int mode;

  // Reset chiavi ODB usate dall'alarm/watcher
  INT8 zero = 0;
  INT status1 = db_set_value(hDB, 0, "/Configurations/RunControl/StopRequested",
                             &zero, sizeof(zero), 1, TID_INT8);

  const char* empty_reason = "";
  INT status2 = db_set_value(hDB, 0, "/Configurations/RunControl/StopReason",
                             empty_reason, 1, 1, TID_STRING);

  if (status1 != DB_SUCCESS || status2 != DB_SUCCESS) {
    cm_msg(MERROR, "cygnus_daq",
           "end_of_run: failed to reset ODB stop flags: StopRequested=%d StopReason=%d",
           status1, status2);
  } else {
    cm_msg(MINFO, "cygnus_daq",
           "end_of_run: reset /Configurations/RunControl stop flags");
  }
   
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
  if(stop_requested && !stop_sent) {
    cerr << "DEBUG: frontend_loop: stopping run..." << endl << flush;

    HNDLE hDB;
    cm_get_experiment_database(&hDB, NULL);

    cm_msg(MERROR, "cygnus_daq", 
           "frontend_loop: requesting TR_STOP due to camera error %u",
           stop_error_code);

    // The following must be intensively validated
    //
    //BOOL v = TRUE;
    //db_set_value(hDB, 0, "/Equipment/Trigger/Variables/_PendingAutoRestart",
    //             &v, sizeof(v), 1 TID_BOOL);
    
    char reason[32];
    strcpy(reason, "camera fatal error");
    

    INT8 one = 1;
    INT status1 = db_set_value(hDB, 0, "/Configurations/RunControl/StopRequested",
                          &one, sizeof(one), 1, TID_INT8);

    INT status2 = db_set_value(hDB, 0, "/Configurations/RunControl/StopReason",
                           reason, strlen(reason) + 1, 1, TID_STRING);

    if (status1 == DB_SUCCESS && status2 == DB_SUCCESS) {
      stop_sent = TRUE;

      cm_msg(MERROR, "cygnus_daq",
             "Requested automatic stop+restart: %s", reason);
    } else {
      cm_msg(MERROR, "cygnus_daq",
             "Failed to set stop request in ODB: StopRequested=%d StopReason=%d",
             status1, status2);
    }

    // cerr << "DEBUG: calling cm_transition(TR_STOP)" << endl << flush;
    // INT status = cm_transition(TR_STOP, 0, errmsg, sizeof(errmsg), TR_ASYNC, FALSE);
    // if (status!= CM_SUCCESS) {
    //   cm_msg(MERROR, "cygnus_daq",
    //          "frontend_loop: cm_transition(TR_STOP) failed: %d %s", status, errmsg);
    // }
    
  }
  cerr << "DEBUG: end of frontend_loop..." << endl << flush;
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

  // Do not poll if stop is requested
  if(stop_requested) {
    cerr<<"Skip polling of event ...."<<endl<<flush;
    usleep(1000000);
    return 0;
  }

  cerr<<"Polling event ...."<<endl<<flush;

  int maxevents;
  bool freerun;
  int mode;
  int size = sizeof(int);
  HNDLE hDB;
  cm_get_experiment_database(&hDB, NULL);

  // Do not poll if some process from outside issues a stop transition
  // /Runinfo/Transition in progress can be
  // * 0 == no transition
  // * 1 == start transition
  // * 2 == stop transition
  int tip = 0;
  size = sizeof(int);
  db_get_value(hDB, 0, "/Runinfo/Transition in progress",
               &tip,&size,TID_INT,TRUE);
  if(tip==2) return 0; // if during stop transition

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

      //set_equipment_status("Trigger", "Camera not found", "red");
      //if(error) {
      //  strcpy(error, "Camera not found");
      //}
      return FE_ERR_HW;
    }
    if(hwait[icam] == NULL) {
      cout << "WAIT HANDLE OF CAMERA "<<icam<<" NOT FOUND" << endl;

      //set_equipment_status("Trigger", "Unable to open camwait handle", "red");
      //if(error) {
      //  strcpy(error, "Unable to open camwait handle");
      //}
      return FE_ERR_HW;
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
        //waitstart[icam].timeout = DCAMWAIT_TIMEOUT_INFINITE;
        waitstart[icam].timeout = (int)(exposure * 1000. * 1000.) ; // Set timeout = 3 pics
        // DEBUG
        //waitstart[icam].timeout = (int)(exposure * 1000. * 0.5) ; // Set DEBUG timeout = 0.5 pics
    }
    else
    {
        //waitstart[icam].timeout = DCAMWAIT_TIMEOUT_INFINITE;
        waitstart[icam].timeout = (int)((delay+exposure)*1000) +200; //in ms --> max wait = 2*exposure + USB transfer time // 30 before
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

  // WAIT FOR CAMERA SIGNAL (needed for correct generation of S-IN signal)
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

      // if(failed(err1)) {
      //   cerr<<"poll_event: dcamwait_start failed for camera "<<icam<<" with error "<<err1<<endl<<flush;
      // }

      // New solution [to be tested]
      if(failed(err1)) {
        cerr << "poll_event: dcamwait_start failed for camera "
             << icam << " with error "<< err1 << endl << flush;
        
        // Fatal errors must be added here to stop the run
        if(!stop_already_requested && ((DWORD)err1 == 2147483910u)) {
          stop_already_requested = TRUE;
          stop_requested = TRUE;
          stop_error_code = (DWORD)err1;
	  fatal_camera_error = TRUE;


          cm_msg(MERROR, "cygnus_daq", "poll_event: fatal camera error %u on camera %d, requesting stop",
                 stop_error_code, icam);

          // The following must be intensively validated
          //
          //BOOL v = TRUE;
          //db_set_value(hDB, 0, "/Equipment/Trigger/Variables/_PendingAutoRestart",
          //             &v, sizeof(v), 1 TID_BOOL);

          cerr << "DEBUG: setting stop_requested and leaving poll_event" << endl << flush;

          return 0;

        }


      }

      //// Solution #2: restart the run "tricking" the Logger
      ////if(err1 == DCAMERR_TIMEOUT && !stop_already_requested && rec_ev > 100) { // DEBUG
      //if(failed(err1) && !stop_already_requested && err1 != 2214600705) {
      ////if(err1 == DCAMERR_TIMEOUT && !stop_already_requested) {
      //  cm_msg(MERROR, "cygnus_daq", "poll_event: dcamwait_start timeout %d. Restarting run.", jj);

      //  // Setting we expect an autorestart
      //  BOOL v = TRUE;
      //  db_set_value(hDB, 0, "/Equipment/Trigger/Variables/_PendingAutoRestart", &v, sizeof(v), 1, TID_BOOL);

      //  // Tricking the Logger to autorestart
      //  BOOL yes = TRUE;
      //  //db_set_value(hDB, 0, "/Logger/Auto restart", &yes, sizeof(yes), 1, TID_BOOL);
      //  uint32_t one = 1;
      //  db_set_value(hDB, 0, "/Logger/Run duration", &one, sizeof(one), 1, TID_UINT32);


      //}
    }
    //err1 = dcamwait_start( hwait[0], &waitstart);

    //usleep(1000); // wait 200 us to avoid synch problems with the camera
    //err1 = dcamwait_start( hwait[1], &waitstart2 );

    

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
      flag = (lamDGTZ && lamCAM);
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

    cerr<<"Event found"<<endl;
    return TRUE;
  }
        
#ifdef HAVE_CAEN_BRD
    
#ifdef HAVE_CAEN_DGTZ
  if(lamDGTZ && mode != 3) {
  //if(lamDGTZ) {
    // for(int i=0;i<nboard;i++){
    //   CAEN_DGTZ_ClearData(gDGTZ[i]);
    // }
    cerr << "poll_event: digitizer had data but event was not accepted; "
         << "leaving data untouched, not calling ClearData() during acquisition."
         << endl << flush;
  }
#endif
    
  //Reset GATE (pulser B)
  //WRONG FOR V3718
  //CAENVME_StopPulser(gVme->handle,cvPulserB);
    
#endif

  cerr<<"Event not found"<<endl;
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
  cerr<<"Start reading event "<<rec_ev<<endl;

  rec_ev++;

  //////READ SYSTEMS
  HNDLE hDB;
  INT status;
  status = cm_get_experiment_database(&hDB, NULL);

  int mode;
  int size = sizeof(int);
  db_get_value(hDB, 0, "/Configurations/TriggerMode",&mode,&size,TID_INT,TRUE);
  bool freerun;
  size = 4*sizeof(bool);
  db_get_value(hDB, 0, "/Configurations/FreeRunning",&freerun,&size,TID_BOOL,TRUE);

  int crop_size;
  size = sizeof(int);
  status = cm_get_experiment_database(&hDB, NULL);
  db_get_value(hDB, 0, "/Configurations/CameraCropSettings/crop_size",&crop_size,&size,TID_INT,TRUE);
  int crop_origin_x;
  size = sizeof(int);
  status = cm_get_experiment_database(&hDB, NULL);
  db_get_value(hDB, 0, "/Configurations/CameraCropSettings/crop_origin_x",&crop_origin_x,&size,TID_INT,TRUE);
  //cerr<<"DEBUG crop_origin_x "<<crop_origin_x<<endl;
  int crop_origin_y;
  size = sizeof(int);
  status = cm_get_experiment_database(&hDB, NULL);
  db_get_value(hDB, 0, "/Configurations/CameraCropSettings/crop_origin_y",&crop_origin_y,&size,TID_INT,TRUE);
  //cerr<<"DEBUG crop_origin_y "<<crop_origin_y<<endl;
  bool crop_image;
  size = 4*sizeof(bool);
  status = cm_get_experiment_database(&hDB, NULL);
  db_get_value(hDB, 0, "/Configurations/CameraCropSettings/crop_enable",&crop_image,&size,TID_BOOL,TRUE);
  //cerr<<"DEBUG crop_enable "<<crop_image<<endl;
  
  /* init bank structure */
  bk_init32(pevent);
  INT defaultEvSize = bk_size(pevent);

 
#ifdef HAVE_CAMERA

  //bool crop_image;
  //int sizecam = sizeof(int);
  //sizecam = 4*sizeof(bool);

  //db_get_value(hDB, 0, "/Configurations/FreeRunning",&crop_image,&sizecam,TID_BOOL,TRUE);
//#pragma omp parallel for// num_threads(nCamera)
  for(int icam=0; icam <nCamera; icam++) {
    // Check camera mask flag
    if(!CamMask[icam]) continue;

    //cerr<<"Reading event from camera "<<icam<<".... "<<endl<<flush;
    read_camera(pevent, icam, crop_image, crop_size, crop_origin_x, crop_origin_y);
  }
  //read_camera(pevent);
#endif


#ifdef HAVE_CAEN_BRD
#ifdef HAVE_CAEN_DGTZ

  if(!freerun && mode != 3) read_dgtz(pevent);

  else if(!freerun && mode == 3) {
    int lamDGTZ = 1;

    // Check if boards are data ready
    vector<uint32_t> st(nboard);

    read_dgtz(pevent);

  }
#endif
#endif
  //////////////////
  
  // if(mode==3) ClearDevice(false);
  // else {
  //   ClearDevice(true);
  //   CAENVME_SetOutputRegister(gVme->handle,cvOut1Bit);
  //   //cerr<<"---> GATE SET TO 1"<<endl<<flush;
  // }

  // Do not clear digitizer data during normal acquisition.
  // ReadData() is the operation that should drain accepted events.
  // ClearData() should be reserved for begin/end/reset recovery with gate closed.
  ClearDevice(false);

  #ifdef HAVE_CAEN_BRD
  if(mode != 3) {
    CAENVME_SetOutputRegister(gVme->handle, cvOut1Bit); // reopen trigger gate
  }
  #endif

  if (bk_size(pevent)==defaultEvSize ) { return 0; }

  cerr<<"Stop  reading event "<<rec_ev-1<<endl;
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

  gDGTZ       = new int[nboard];
  gDigBase    = new uint32_t[nboard];     //0x22220000;
  gDigLink    = new uint32_t[nboard];     //1;
  BoardName   = new char*[nboard];
  buffer_dgtz = new char*[nboard];
  NCHDGTZ     = new uint32_t[nboard];        // = 40000;
  DGTZ_OFFSET = new double*[nboard];
  /////Number of samples per waveform and sampling rate
  ndgtz       = new uint32_t[nboard];          //= 1024;
  SAMPLING    = new uint32_t[nboard] ;    //250;
  /////Horizontal offset
  posttrg     = new int[nboard];   //=  70;

  //read from the database and initialise gDigBase
  for(int i=0;i<nboard;i++){

    /////Open the board
    char query[100];
    int dig_base = 0;
    int dig_link = 0;

    sprintf(query, "/Configurations/Digitizer Base Address[%d]", i);
    size = sizeof(dig_base);
    db_get_value(hDB, 0, query, &dig_base, &size, TID_INT, TRUE);

    sprintf(query, "/Configurations/Digitizer Link Number[%d]", i);
    size = sizeof(dig_link);
    db_get_value(hDB, 0, query, &dig_link, &size, TID_INT, TRUE);
    
    gDigBase[i] = static_cast<uint32_t>(dig_base);
    gDigLink[i] = static_cast<uint32_t>(dig_link);


    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB,gDigLink[i],0,gDigBase[i],&gDGTZ[i]);
    if(ret != CAEN_DGTZ_Success) {
      //printf("Can't open digitizer, board number %d %d\n-- Error %d\n",i,gDigBase[i],ret);
      //ret = CAEN_DGTZ_Reset(gDGTZ[i]);
      //if(ret != CAEN_DGTZ_Success) {
      //printf("Unable to clear digitizer, board number %d %d\n-- Error %d\n",i,gDigBase[i],ret);
      //}
      cm_msg(MERROR, "cygnus_daq",
           "Cannot open digitizer board %d base=0x%x link=%d ret=%d",
           i, gDigBase[i], gDigLink[i], ret);
      exit(EXIT_FAILURE);
    }

    ////Board name
    CAEN_DGTZ_BoardInfo_t BoardInfo;
    CAEN_DGTZ_GetInfo(gDGTZ[i], &BoardInfo);
    BoardName[i] = new char[10];
    strcpy(BoardName[i],BoardInfo.ModelName);
    printf("%s\n",BoardName[i]);
    if(!IsSupportedDigitizer(BoardName[i])) {
      cm_msg(MERROR, "cygnus_daq", "Unsupported digitizer model %s on board %d. Only V1742 and V1720E are supported.", BoardName[i], i);
      exit(EXIT_FAILURE);
    }

    ////Buffer preparation
    buffer_dgtz[i]=NULL;

    ////Number of channels
    NCHDGTZ[i] = BoardInfo.Channels;
    if(IsV1742(BoardName[i]))
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

#ifdef HAVE_CAEN_DGTZ
  
  /* DIGITIZER INITIALIZATION */
  
  CAEN_DGTZ_BoardInfo_t *BoardInfo= new CAEN_DGTZ_BoardInfo_t[nboard] ;
  CAEN_DGTZ_ErrorCode ret;

  for(int i=0;i<nboard;i++) {
    ret = CAEN_DGTZ_GetInfo(gDGTZ[i], &BoardInfo[i]);

    printf("\nConnected to CAEN Digitizer Model %s %d -- %d channels\n", BoardInfo[i].ModelName,BoardInfo[i].FamilyCode,NCHDGTZ[i]);

    if(NCHDGTZ[i] > 32) {
      printf("Error in NCHDGTZ %d\n",NCHDGTZ[i]);
      exit(EXIT_FAILURE);
    }
      
    ////Reset the board
    ret |= CAEN_DGTZ_Reset(gDGTZ[i]);                                               /* Reset Digitizer */
    ret |= CAEN_DGTZ_ClearData(gDGTZ[i]);

    ////Waveform Setup
    if(IsV1720E(BoardName[i])){
      ret |= CAEN_DGTZ_SetChannelEnableMask(gDGTZ[i],(int)(pow(2,NCHDGTZ[i]))-1);   /* Enable channel 0 and 1*/
      cout << "enable " << (int)(pow(2,NCHDGTZ[i]))-1 << endl;
    } else if(IsV1742(BoardName[i])) {
      ret |= CAEN_DGTZ_SetGroupEnableMask(gDGTZ[i],0xF);
    }  else {
      cm_msg(MERROR, "cygnus_daq", "Unsupported digitizer model %s. Only V1742 and V1720E are supported.", BoardName[i]);
      return FE_ERR_HW;
    }
  
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

  int previous_channels = 0;

  for(int i=0;i<nboard;i++){

    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;

    sprintf(query,"/Configurations/DigitizerSamples[%d]",i);
    db_get_value(hDB, 0, query,&ndgtz[i],&size,TID_INT,TRUE);
    
    CAEN_DGTZ_DRS4Frequency_t DRS4Frequency = CAEN_DGTZ_DRS4_5GHz;

    // Set V1742 always-one-buffer-free mode on board V1742 to avoid data loss in case of high trigger rate.
    // This is done by setting bit 5 of the acquisition control register.
    if(IsV1742(BoardName[i])) {
      uint32_t acq_ctrl = 0;
      CAEN_DGTZ_ErrorCode rctrl = CAEN_DGTZ_ReadRegister(gDGTZ[i], DGTZ_ACQ_CONTROL, &acq_ctrl);

      if (rctrl == CAEN_DGTZ_Success) {
        acq_ctrl |= (1u << 5); // x742: keep one buffer free / safer FULL recovery
        rctrl = CAEN_DGTZ_WriteRegister(gDGTZ[i], DGTZ_ACQ_CONTROL, acq_ctrl);
      }

      if (rctrl != CAEN_DGTZ_Success) {
        cerr << "Warning: could not set V1742 always-one-buffer-free mode on board " << i << ". ret=" << rctrl << endl << flush;
      }
    }


    if(IsV1720E(BoardName[i])) {
      if(ndgtz[i] > static_cast<uint32_t>(1.25e6/maxtriggersize) ) {
        ndgtz[i] = static_cast<uint32_t>(1.25e6/maxtriggersize);
      }                                         
      ret |= CAEN_DGTZ_SetRecordLength(gDGTZ[i],ndgtz[i]); /* Set the lenght of each waveform (in samples) */
      SAMPLING[i] = 250;
    } else if(IsV1742(BoardName[i])) {
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
    } else {
      cm_msg(MERROR, "cygnus_daq", "Unsupported digitizer model %s. Only V1742 and V1720E are supported.", BoardName[i]);
      return FE_ERR_HW;
    }
    
    if(ret != CAEN_DGTZ_Success) {
      printf("Errors before POSTTRG size.\n");
      //cm_msg(MERROR, "cygnus_daq", "Errors during Digitizer Configuration.");
      //throw runtime_error("Errors during Digitizer Digitizer Configuration.");
    }

    sprintf(query,"/Configurations/DigitizerPostTrg[%d]",i);
    db_get_value(hDB, 0, query,&posttrg[i],&size,TID_INT,TRUE);
    ret |= CAEN_DGTZ_SetPostTriggerSize(gDGTZ[i],posttrg[i]); /* Trigger position */
    
    if(ret != CAEN_DGTZ_Success) {
      printf("Errors after POSTTRG size.\n");
      //cm_msg(MERROR, "cygnus_daq", "Errors during Digitizer Configuration.");
      //throw runtime_error("Errors during Digitizer Digitizer Configuration.");
    }
    //Print the firmware version
    /*
    CAEN_DGTZ_BoardInfo_t BoardInfo;

    for(int i=0;i<nboard;i++){
      CAEN_DGTZ_GetInfo(gDGTZ[i], &BoardInfo);
      std::cerr << "Digitizer " << i << " (" << BoardName[i] << ") - Firmware: " << BoardInfo.ROC_FirmwareRel << " / " << BoardInfo.AMC_FirmwareRel << std::endl;
    }
    */

    //ENABLING EGTTT 60 bit 
    //for(int i=0;i<nboard;i++){
    if(IsV1742(BoardName[i])) {
      // Read the register than turn on the bit 20
      uint32_t enable_egttt;
      CAEN_DGTZ_ReadRegister(gDGTZ[i], 0x8000, &enable_egttt);
      enable_egttt = enable_egttt | 0x00100000u;  // bit 20
      CAEN_DGTZ_WriteRegister(gDGTZ[i], 0x8004, enable_egttt);
    } else if (IsV1720E(BoardName[i])) {
      uint32_t enable_egttt;
      CAEN_DGTZ_ReadRegister(gDGTZ[i], 0x811C, &enable_egttt);
      cout<<"READOUT of ETTT register for board "<<i<<": "<<std::bitset<32>(enable_egttt)<<endl;
      // Set register =0x811C properly: Bits[22:21] to "10"
      enable_egttt = enable_egttt & 0xFF9FFFFFu;
      enable_egttt = enable_egttt | 0x00400000u;
      cout<<"SETUP   of ETTT register for board "<<i<<": "<<std::bitset<32>(enable_egttt)<<endl;
      CAEN_DGTZ_WriteRegister(gDGTZ[i], 0x811C, enable_egttt);
    }

    //}

    if(ret != CAEN_DGTZ_Success) {
      printf("Errors enabling EGTTT size.\n");
      //cm_msg(MERROR, "cygnus_daq", "Errors during Digitizer Configuration.");
      //throw runtime_error("Errors during Digitizer Digitizer Configuration.");
    }

    // SETUP OF INTERNAL TRIGGER LOCIC FOR 1720E (see page 46 of Manual, point 3)
    if(IsV1720E(BoardName[i])) {
      // Read the Global Trigger Mask register
      uint32_t global_trigger_mask;
      CAEN_DGTZ_ReadRegister(gDGTZ[i], 0x810C, &global_trigger_mask);
      cout<<"Readout of GTC register for board "<<i<<": "<<std::bitset<32>(global_trigger_mask)<<endl;
      // Set external trigger only: Bits[31:29] to "010"
      global_trigger_mask = global_trigger_mask & 0x1FFFFFFFu; // Clear Bits[31:29]
      global_trigger_mask = global_trigger_mask | 0x40000000u; // Set Bits[31:29] to "010"
      // Disable self-trigger on all channels: Bits[7:0] to "00000000"
      global_trigger_mask = global_trigger_mask & 0xFFFFFF00u;
      cout<<"Setup of GTC register for board "<<i<<": "<<std::bitset<32>(global_trigger_mask)<<endl;
      CAEN_DGTZ_WriteRegister(gDGTZ[i], 0x810C, global_trigger_mask);
    }

    size = sizeof(double);
    for(int ich=0;ich<NCHDGTZ[i];ich++){
      
      sprintf(query,"/Configurations/DigitizerOffset[%d]",previous_channels+ich);

      db_get_value(hDB,0,query,&DGTZ_OFFSET[i][ich],&size,TID_DOUBLE,TRUE);
      
      if(DGTZ_OFFSET[i][ich] > 0.5) DGTZ_OFFSET[i][ich] = 0.5;
      else if(DGTZ_OFFSET[i][ich] < -0.5) DGTZ_OFFSET[i][ich] = -0.5;

      //printf("DEBUG: i = %d, ich = %d, tosetup = %d, previous_channels = %d \n", i, ich, (uint32_t)(DGTZ_OFFSET[i][ich]*65536 + 32767), previous_channels);

      if( IsV1720E(BoardName[i]) ) {
        ret |= CAEN_DGTZ_SetChannelDCOffset(gDGTZ[i],ich,(uint32_t)(DGTZ_OFFSET[i][ich]*65536 + 32767));
      } else if(IsV1742(BoardName[i])){
        int grreg = 0x1098 | (ich/8 << 8);
        int data = (ich%8<<16) | static_cast<int>(DGTZ_OFFSET[i][ich]/2.*65536 + 32767);
        //CAEN_DGTZ_SetGroupDCOffset(gDGTZ[i],ich/8,(uint32_t)(DGTZ_OFFSET[i][ich]*65536 + 32767));
        ret |= CAEN_DGTZ_WriteRegister(gDGTZ[i],grreg,data);
      }

      if(ret != CAEN_DGTZ_Success) {
        printf("Errors during DCoffset setup of board %d and ch %d.\n", i, ich);
        //cm_msg(MERROR, "cygnus_daq", "Errors during Digitizer Configuration.");
        //throw runtime_error("Errors during Digitizer Digitizer Configuration.");
      }

      
      //if(ret != CAEN_DGTZ_Success) {
      //   cout<<BoardName[i]<<"  "<<endl;
      //   printf("%d  %d  Errors during Digitizer Configuration.\n", i, ich);
      //}
	
    }
    previous_channels += NCHDGTZ[i];
    
    //Uploading and enabling the automatic correction for the 1742 digitizer
    bool enable_corrections;
    size = 4*sizeof(bool);
    db_get_value(hDB,0,"/Configurations/DRS4Correction",&enable_corrections, &size, TID_BOOL,TRUE);
    
    CAEN_DGTZ_ErrorCode ret2;
    
    if(IsV1742(BoardName[i])) {
      if(!enable_corrections) {
      //std::cout<<"DEBUG NO CORRECTION"<<std::endl;
      ret2 = CAEN_DGTZ_DisableDRS4Correction(gDGTZ[i]);
      if(ret2 != CAEN_DGTZ_Success) {
        cerr<<"Error in DisableDRS4Correction"<<endl;
      }
	  } else {
		  if ((ret2 = CAEN_DGTZ_LoadDRS4CorrectionData(gDGTZ[i],DRS4Frequency)) != CAEN_DGTZ_Success) { 
		    cerr<<"Error in LoadDRS4Correction"<<endl;
		    exit(EXIT_FAILURE);
		  } 
		  if ((ret2 = CAEN_DGTZ_EnableDRS4Correction(gDGTZ[i])) != CAEN_DGTZ_Success) { 
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
    } //else{

      //Calibration
      //ret |= CAEN_DGTZ_Calibrate(gDGTZ[i]);

    //}

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

#ifdef HAVE_CAEN_DGTZ
  for(int i=0;i<nboard;i++){
    CAEN_DGTZ_SWStartAcquisition(gDGTZ[i]);
  }
#endif

#ifdef HAVE_CAEN_BRD
  // Force gate open / not busy after enabling acquisition
  CAENVME_SetOutputRegister(gVme->handle, cvOut1Bit);
  //cerr << "DEBUG: enable_trigger: OUT1 set / gate open" << endl << flush;
#endif

  return 0;

}

INT ClearDevice(BOOL clear_dgtz_data) {

#ifdef HAVE_CAEN_BRD
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
  std::vector<std::vector<uint16_t>> StartIndexCell(nboard);
  std::vector<int> EVTSNUM(nboard);
  std::vector<uint32_t> ADCMAX(nboard);

  std::vector<std::vector<CAEN_DGTZ_UINT16_EVENT_t*> > Evt16(nboard);
  std::vector<std::vector<CAEN_DGTZ_X742_EVENT_t*> > Evt742(nboard);

  std::vector<uint32_t> bsize_board(nboard, 0);
  std::vector<bool> read_ok(nboard, false);
    
  /*
    FIRST LOOP:
    Read data from all boards, and store the size of the data read for each board in bsize_board[i].
  */
  for(int i=0;i<nboard;i++){
    
    CAEN_DGTZ_ErrorCode retread = CAEN_DGTZ_ReadData(gDGTZ[i],
                        CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, //CAEN_DGTZ_POLLING_MBLT,
                        buffer_dgtz[i],
                        &bsize_board[i]);

    // If the first read fails, try one delayed retry after a short sleep
    // This is a workaround for occasional read failures that can occur with the CAEN digitizers
    if(retread != CAEN_DGTZ_Success) {
      cerr << "Error in CAEN_DGTZ_ReadData for board " << i
          << ". ErrorCode = " << retread
          << ". Trying one delayed retry." << endl << flush;

      usleep(200); // Must be at least 182 us for V1742, but 200 u is safe for all boards

      uint32_t acq_after_fail = 0;
      CAEN_DGTZ_ErrorCode rstat = ReadRegisterRetry(gDGTZ[i], DGTZ_ACQ_STATUS, &acq_after_fail, 3, 100);

      cerr << "After ReadData failure, board " << i
          << " ACQ_STATUS ret=" << rstat
          << " value=0x" << hex << acq_after_fail << dec
          << endl << flush;

      if(IsV1742(BoardName[i])) {
          ReadV1742GroupBusyOrFull(gDGTZ[i], i, true);
      }

      retread =
        CAEN_DGTZ_ReadData(gDGTZ[i],
                          CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,
                          buffer_dgtz[i],
                          &bsize_board[i]);
    }

    if(retread != CAEN_DGTZ_Success) {
      cerr << "Persistent CAEN_DGTZ_ReadData failure for board " << i
          << ". ErrorCode = " << retread << endl << flush;

      EVTSNUM[i] = 0;
      continue;
    }

    read_ok[i] = true;

  }

  /*
    SECOND LOOP:
    Decode the events and store them in the Evt16 or Evt742 vectors.
    Also store the trigger time tags and patterns in TRGTTAG and TRGTTAG1, and the start index cell for V1742 boards.
  */
  for(int i=0; i<nboard; i++) {
    if(!read_ok[i]) continue;

    //CAEN_DGTZ_ErrorCode retnum = CAEN_DGTZ_GetNumEvents(gDGTZ[i],buffer_dgtz[i],bsize,&NumEvents);
    CAEN_DGTZ_ErrorCode retnum = CAEN_DGTZ_GetNumEvents(gDGTZ[i],buffer_dgtz[i],bsize_board[i],&NumEvents);
    if(retnum != CAEN_DGTZ_Success) {
      cerr << "Error in CAEN_DGTZ_GetNumEvents for board " << i << ". ErrorCode = " << retnum << endl << flush;
      EVTSNUM[i] = 0;
      continue;
    }
    NumEvents =std::min(NumEvents, events_max);

    CAEN_DGTZ_BoardInfo_t BoardInfo;
    CAEN_DGTZ_ErrorCode retinfo_board = CAEN_DGTZ_GetInfo(gDGTZ[i], &BoardInfo);
    if(retinfo_board != CAEN_DGTZ_Success) {
      cerr << "Error in CAEN_DGTZ_GetInfo for board " << i << ". ErrorCode = " << retinfo_board << endl << flush;
      ADCMAX[i] = 0;
    } else {
      ADCMAX[i] = (uint32_t)pow(2, BoardInfo.ADC_NBits);
    }

    TRGTTAG[i].resize(NumEvents, 0);
    TRGTTAG1[i].resize(NumEvents, 0);

    if(IsV1720E(BoardName[i])){
      Evt16[i].resize(NumEvents, NULL);

      int nvalid = 0;

      for(int iev=0;iev<NumEvents;iev++){
        CAEN_DGTZ_UINT16_EVENT_t *Evt = NULL;

        CAEN_DGTZ_ErrorCode retall = CAEN_DGTZ_AllocateEvent(gDGTZ[i], (void**)&Evt);
        if(retall != CAEN_DGTZ_Success) {
          cerr <<"Error allocating event. ErrorCode = "<<retall<<endl<<flush;
          continue;
        }

        //CAEN_DGTZ_ErrorCode retinfo = CAEN_DGTZ_GetEventInfo(gDGTZ[i],buffer_dgtz[i],bsize,iev,&eventInfo,&evtptr);
        CAEN_DGTZ_ErrorCode retinfo = CAEN_DGTZ_GetEventInfo(gDGTZ[i],buffer_dgtz[i],bsize_board[i],iev,&eventInfo,&evtptr);
        if(retinfo != CAEN_DGTZ_Success) {
          cerr << "Unable to get DGTZ event info. ErrorCode = " << retinfo << endl << flush;
          if(Evt != NULL) {
            CAEN_DGTZ_FreeEvent(gDGTZ[i], (void**)&Evt);
          }
          continue;
        }

        CAEN_DGTZ_ErrorCode retdec = CAEN_DGTZ_DecodeEvent(gDGTZ[i],evtptr,(void**)&Evt);
        if(retdec != CAEN_DGTZ_Success) {
          cerr <<"Unable to decode DGTZ event"<<endl<<flush;
          if(Evt != NULL) {
            CAEN_DGTZ_FreeEvent(gDGTZ[i], (void**)&Evt);
          }
          continue;
        }

        TRGTTAG[i][nvalid]  = eventInfo.TriggerTimeTag; // TO BE CHECKED ON x761
        TRGTTAG1[i][nvalid] = eventInfo.Pattern;
        Evt16[i][nvalid] = Evt;

        nvalid++;
      }

      EVTSNUM[i] = nvalid;
      Evt16[i].resize(nvalid);
      TRGTTAG[i].resize(nvalid);
      TRGTTAG1[i].resize(nvalid);
    } else if(IsV1742(BoardName[i])){
      Evt742[i].resize(NumEvents, NULL);
      StartIndexCell[i].resize(NumEvents, 0);

      int nvalid = 0;

      for(int iev=0;iev<NumEvents;iev++){
        CAEN_DGTZ_X742_EVENT_t *Evt = NULL;
        
        CAEN_DGTZ_ErrorCode retall = CAEN_DGTZ_AllocateEvent(gDGTZ[i], (void**)&Evt);
        if(retall != CAEN_DGTZ_Success) {
          cerr <<"Error allocating event. ErrorCode = "<<retall<<endl<<flush;
          continue;
        }

        CAEN_DGTZ_ErrorCode retinfo = CAEN_DGTZ_GetEventInfo(gDGTZ[i],buffer_dgtz[i],bsize_board[i],iev,&eventInfo,&evtptr);
        if(retinfo != CAEN_DGTZ_Success) {
          cerr << "Unable to get DGTZ event info. ErrorCode = " << retinfo << endl << flush;
          if(Evt != NULL) {
            CAEN_DGTZ_FreeEvent(gDGTZ[i], (void**)&Evt);
          }
          continue;
        }

        CAEN_DGTZ_ErrorCode retdec = CAEN_DGTZ_DecodeEvent(gDGTZ[i],evtptr,(void**)&Evt);
        if(retdec != CAEN_DGTZ_Success) {
          cerr <<"Unable to decode DGTZ event"<<endl<<flush;
          if(Evt != NULL) {
            CAEN_DGTZ_FreeEvent(gDGTZ[i], (void**)&Evt);
          }
          continue;
        }

        TRGTTAG[i][nvalid]    = Evt->DataGroup[0].TriggerTimeTag & 0x3FFFFFFF;
        TRGTTAG1[i][nvalid]    = Evt->DataGroup[1].TriggerTimeTag & 0x3FFFFFFF;

        StartIndexCell[i][nvalid] = Evt->DataGroup[0].StartIndexCell;

        Evt742[i][nvalid] = Evt;

        nvalid++;
      }

      EVTSNUM[i] = nvalid;
      Evt742[i].resize(nvalid);
      TRGTTAG[i].resize(nvalid);
      TRGTTAG1[i].resize(nvalid);
      StartIndexCell[i].resize(nvalid);
    } else {
      cerr << "Unsupported digitizer model for board " << i << ": " << BoardName[i] << endl << flush;
      EVTSNUM[i] = 0;
    }

  }

  /*
    THIRD LOOP:
    Loop over the decoded events and write the data to the output buffer.
    Free the event memory after writing.
  */
  for(int i=0; i<nboard; i++){
    if(IsV1720E(BoardName[i])){

      for(unsigned int iev=0; iev<Evt16[i].size(); iev++){
        CAEN_DGTZ_UINT16_EVENT_t *Evt = Evt16[i][iev];
        if(Evt == NULL) continue;

        // Loop over channels and samples
        for(int j=0;j<NCHDGTZ[i];j++){
          for (uint32_t k=0; k<ndgtz[i]; ++k) {
            uint16_t temp = (uint16_t)(Evt->DataChannel[j][k]);
            *pdata16++ = temp;
          }
        }
        
        CAEN_DGTZ_FreeEvent(gDGTZ[i],(void**)&Evt);
        Evt16[i][iev] = NULL;
      }
    } else if(IsV1742(BoardName[i])){

      for(unsigned int iev=0; iev<Evt742[i].size(); iev++){
        CAEN_DGTZ_X742_EVENT_t *Evt = Evt742[i][iev];
        if(Evt == NULL) continue;

        // Loop over channels and samples
        for(int j=0;j<NCHDGTZ[i];j++){
          uint32_t ig = j/8;
          uint32_t ich = j%8;

          for (uint32_t k=0; k<ndgtz[i]; ++k) {
            uint16_t temp = (uint16_t)(Evt->DataGroup[ig].DataChannel[ich][k]);
            *pdata16++ = temp;
          }
        }

        CAEN_DGTZ_FreeEvent(gDGTZ[i],(void**)&Evt);
        Evt742[i][iev] = NULL;
      }
    }
  }
  
  //cerr<<"Closing DIG0 bank..."<<endl<<flush;
  bk_close(pevent, pdata16);
  //cerr<<"DIG0 bank closed"<<endl<<flush;
    
  uint32_t* hdata = NULL;
  uint32_t header_data = 0;
  //TIME_STAMP(pevent) = (std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch())).count();
  bk_create(pevent, "DGH0", TID_DWORD, (void **)&hdata);

  uint32_t DAQ_version = 1000;
  *hdata++ = DAQ_version;

  header_data = nboard;
  *hdata++ = header_data;

  for(int i=0;i<nboard;i++){
    
    header_data = atoi(&BoardName[i][1]);
    *hdata++ = header_data;
    
    header_data = ndgtz[i];
    *hdata++ = header_data;
    
    header_data = NCHDGTZ[i];
    *hdata++ = header_data;
    
    header_data = EVTSNUM[i];
    *hdata++ = header_data;

    header_data = ADCMAX[i];
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
    
    if(IsV1742(BoardName[i])) {
    	for(unsigned int j=0; j<EVTSNUM[i]; j++) {
      		*hdata++ = (uint32_t)StartIndexCell[i][j];
    	}
    }
  }//end for on boards for header
  
  bk_close(pevent, hdata);

  return 0;
}
#endif

#ifdef HAVE_CAMERA
INT read_camera(char *pevent, int icam, bool crop_image, int crop_size, int crop_origin_x, int crop_origin_y)
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
  if(failed(err)) {
    cerr << "read_camera: unable to get captransferinfo." << endl;
    return FE_ERR_HW;//throw runtime_error("read_camera: unable to get captransferinfo.\n");
  }

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
      //*pdata++ = tmpData;
      
      if(crop_image) {
        if(x>crop_origin_x && x<= crop_origin_x+crop_size && y>crop_origin_y && y<= crop_origin_y+crop_size) {
          *pdata++ = tmpData;
        }
      } else {
        *pdata++ = tmpData;
      }

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
