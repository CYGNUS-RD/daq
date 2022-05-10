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

using namespace std;

#define HAVE_CAEN_BRD
#define HAVE_CAMERA

#ifdef HAVE_CAEN_BRD
//#define HAVE_V895
//#define HAVE_V1190
#define HAVE_CAEN_DGTZ
#ifdef HAVE_CAEN_DGTZ
#define HAVE_V1761 
#define HAVE_V1742
#endif
#endif

#ifdef HAVE_CAEN_BRD
#include "vme/caenBridge.h"
#endif
#ifdef HAVE_V895
#include "vme/v895.h"
#endif
#ifdef HAVE_V1190
#include "vme/v1190.h"
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
INT max_event_size = 1000000000;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 5 * 1024 * 1024;

/* buffer size to hold events */
INT event_buffer_size = 2000000000;

/*-- Function declarations -----------------------------------------*/

INT frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();

INT read_event(char *pevent, INT off);

INT poll_event(INT source, INT count, BOOL test);
INT interrupt_configure(INT cmd, INT source, POINTER_T adr);

/* Custom Routines */

INT init_vme_modules();
INT ConfigBridge();
INT ConfigDgtz();
INT ConfigDisc();
INT ConfigCamera();
INT disable_trigger();
INT enable_trigger();
INT ClearDevice();
INT read_tdc(char *pevent);
INT read_dgtz(char *pevent);
INT read_camera(char *pevent);
void ReadDgtzConfig();
void Free_arrays();

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
HDCAM gCam = 0;
HDCAMWAIT hwait = 0;
#endif

int rec_ev = 0;

/*-- Frontend Init -------------------------------------------------*/

INT frontend_init()
{

  /* put any hardware initialization here */
#ifdef HAVE_CAEN_DGTZ
  ReadDgtzConfig();
#endif

#ifdef HAVE_CAEN_BRD

  CaenBrdgSetPar(cvV1718,0);

  int status = mvme_open(&gVme,1);

  //mvme_set_am(gVme, MVME_AM_A32/*MVME_AM_A24_ND*/);
  mvme_set_am(gVme, MVME_AM_A24_ND);

  printf("gVme %d status %d\n",gVme,status);

  init_vme_modules();

#endif

#ifdef HAVE_CAMERA

  DCAMERR err;
  
  gCam = dcamcon_init_open();
  if(gCam == NULL) {
    cout << "CAMERA NOT FOUND" << endl;
    exit(0);
  }
  
  dcamcon_show_dcamdev_info(gCam);

  //External trigger with positive polarity
  /*
  err = dcamprop_setvalue( gCam, DCAM_IDPROP_TRIGGERSOURCE, DCAMPROP_TRIGGERSOURCE__EXTERNAL ); 
  if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGERSOURCE" << endl;

  err = dcamprop_setvalue( gCam, DCAM_IDPROP_TRIGGERPOLARITY, DCAMPROP_TRIGGERPOLARITY__POSITIVE );
  if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGERPOLARITY" << endl;
  */

  //Software trigger
  err = dcamprop_setvalue( gCam, DCAM_IDPROP_TRIGGERSOURCE, DCAMPROP_TRIGGERSOURCE__SOFTWARE ); 
  if(failed(err)) cout << "ERROR IN DCAM_IDPROP_TRIGGERSOURCE" << endl;
  
  //Global exposure as output signal with positive polarity
  err = dcamprop_setvalue( gCam, DCAM_IDPROP_OUTPUTTRIGGER_KIND, DCAMPROP_OUTPUTTRIGGER_KIND__EXPOSURE );
  if(failed(err)) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_KIND" << endl;

  err = dcamprop_setvalue( gCam, DCAM_IDPROP_OUTPUTTRIGGER_POLARITY, DCAMPROP_OUTPUTTRIGGER_POLARITY__POSITIVE );  
  if(failed(err)) cout << "ERROR IN DCAM_IDPROP_OUTPUTTRIGGER_POLARITY" << endl;

  ConfigCamera();
  
#endif
  
  disable_trigger();

  /* print message and return FE_ERR_HW if frontend should not be started */

  return SUCCESS;
}

/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit()
{

  disable_trigger();

#ifdef HAVE_CAMERA
  dcamdev_close( gCam );
  dcamapi_uninit();
#endif
  
#ifdef HAVE_CAEN_DGTZ
  Free_arrays();
#endif
  
  return SUCCESS;

}

/*-- Begin of Run --------------------------------------------------*/

INT begin_of_run(INT run_number, char *error)
{

  rec_ev = 0;
  
  /* put here clear scalers etc. */
#ifdef HAVE_CAEN_BRD

  ConfigBridge();
  //TO BE CHECKED FOR V3718
  CAENVME_StartPulser(gVme->handle,cvPulserA);

  //WRONG FOR V3718
  ////Set LED off
  CAENVME_ClearOutputRegister(gVme->handle,cvOut3Bit);  

#ifdef HAVE_CAEN_DGTZ
  ConfigDgtz();
#endif
   
#ifdef HAVE_V895
  ConfigDisc();
#endif

#endif
  
#ifdef HAVE_CAMERA

  ConfigCamera();
  dcambuf_alloc( gCam, 1 );
  dcamcap_start( gCam, DCAMCAP_START_SEQUENCE );

  DCAMWAIT_OPEN waitopen;
  memset( &waitopen, 0, sizeof(waitopen) );
  waitopen.size = sizeof(waitopen);
  waitopen.hdcam = gCam;
  
  dcamwait_open( &waitopen );
  
  hwait = waitopen.hwait;
  
#endif
  
  enable_trigger();

  return SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/

INT end_of_run(INT run_number, char *error)
{

  disable_trigger();

#ifdef HAVE_CAEN_BRD
  //WRONG FOR V3718
  CAENVME_StopPulser(gVme->handle,cvPulserA);
#endif

#ifdef HAVE_CAMERA
  dcamcap_stop( gCam );
  dcambuf_release( gCam );
  dcamwait_close( hwait );
#endif
   
 return SUCCESS;

}

/*-- Pause Run -----------------------------------------------------*/

INT pause_run(INT run_number, char *error)
{

  disable_trigger();
  
  return SUCCESS;
}

/*-- Resume Run ----------------------------------------------------*/

INT resume_run(INT run_number, char *error)
{
  
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

  int maxevents;
  bool freerun;
  int size = sizeof(int);
  HNDLE hDB;

  cm_get_experiment_database(&hDB, NULL);

  db_get_value(hDB, 0, "/Configurations/MaxEvents",&maxevents,&size,TID_INT,TRUE);

  double events;
  size = sizeof(double);
  db_get_value(hDB, 0, "/Equipment/Trigger/Statistics/Events sent",&events,&size,TID_DOUBLE,TRUE);

  int mode;
  size = sizeof(int);
  db_get_value(hDB, 0, "/Configurations/TriggerMode",&mode,&size,TID_INT,TRUE);
  
  if(maxevents > 0 && events >= maxevents) return 0;

  size = 4*sizeof(bool);
  db_get_value(hDB, 0, "/Configurations/FreeRunning",&freerun,&size,TID_BOOL,TRUE);

  int i;
  DWORD flag;
  
  int lamTDC = 1;
  int lamDGTZ = 1;
  int lamCAM = 1;

  count = 1;

  if (count > 100) count = 100;
  
  for (i = 0; i < count; i++) {

    /*
#ifdef HAVE_CAEN_BRD    
    //////Sync test
    if(rec_ev == 4){
      //Switch LED on through OUT_3
      //WRONG FOR V3718
      CAENVME_SetOutputRegister(gVme->handle,cvOut3Bit|cvOut1Bit);  
      sleep(1);
    }
    else if(rec_ev == 5){
      //Switch LED off through OUT_3
      //WRONG FOR V3718
      CAENVME_ClearOutputRegister(gVme->handle,cvOut3Bit);
      sleep(1);
    }
#endif
    */
    
    /* poll hardware and set flag to TRUE if new event is available */
#ifdef HAVE_CAMERA

    //wait for frame ready
    double exposure;
    dcamprop_getvalue( gCam, DCAM_IDPROP_EXPOSURETIME, &exposure);

    int delay = 180;
    if(mode==1 || mode==2) delay = 360.;
    
    DCAMWAIT_START waitstart;
    memset( &waitstart, 0, sizeof(waitstart) );
    waitstart.size = sizeof(waitstart);
    waitstart.timeout = (int)(exposure*1000) + 30 + delay; //in ms --> max wait = 2*exposure + USB transfer time 
    
    //ofstream outfile;
    //if(!test){
    //  outfile.open("time.dat",ios_base::app);
    //  time_t result = time(nullptr);
    //  outfile << result << "  " ;
    //}

    int pics = 1;
    if(rec_ev==0) pics = 2;
    for(int i=0;i<pics;i++){

      if(pics ==2 && i==0) CAENVME_ClearOutputRegister(gVme->handle,cvOut1Bit);

      //send a trigger to the camera
      dcamcap_firetrigger(gCam,0);
      
      waitstart.eventmask = DCAMWAIT_CAPEVENT_EXPOSUREEND;
      DCAMERR err1 = dcamwait_start( hwait, &waitstart );

      //if(pics==2)
      //	sleep(1);
      
      //if(!test){
      //  time_t result = time(nullptr);
      //  outfile << result << endl ;
      //  outfile.close();
      //}    
      //if(failed(err1) || err1 == DCAMERR_TIMEOUT) lamCAM = 0;

      if(pics ==2 && i==0) CAENVME_SetOutputRegister(gVme->handle,cvOut1Bit);

      lamCAM = 1;

    }
    
#endif

#ifdef HAVE_V1190
    if(!freerun){
      lamTDC = v1190_DataReady(gVme,gTdcBase);
    }
#endif
    
#ifdef HAVE_CAEN_DGTZ
    if(!freerun){
      uint32_t status;
      for(int i=0;i<nboard;i++){
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_ReadRegister(gDGTZ[i],CAEN_DGTZ_ACQ_STATUS_ADD,&status); /* read status register */
	lamDGTZ &= ((status & 0x8)>>3); /* 4th bit is data ready */
      }
    }
#endif

    flag = (lamTDC && lamDGTZ && lamCAM);

    if (flag){
      if (!test){

#ifdef HAVE_CAEN_BRD	
	//SET OUT_1 to 0 (busy)
	//WRONG FOR V3718
	CAENVME_ClearOutputRegister(gVme->handle,cvOut1Bit);
#endif
	
#ifdef HAVE_CAMERA	
	waitstart.eventmask = DCAMWAIT_CAPEVENT_FRAMEREADY;
	DCAMERR err2 = dcamwait_start( hwait, &waitstart );
	if(failed(err2) || err2 == DCAMERR_TIMEOUT) break;
#endif

	return TRUE;
	
      }

    }

#ifdef HAVE_CAMERA
    //DCAMBUF_FRAME bufframe;
    //waitstart.eventmask = DCAMWAIT_CAPEVENT_FRAMEREADY;
    //DCAMERR err2 = dcamwait_start( hwait, &waitstart );
    //dcambuf_lockframe( gCam, &bufframe );
    //dcambuf_release( gCam );
#endif
        
#ifdef HAVE_CAEN_BRD
    
#ifdef HAVE_V1190
    if(lamTDC) {
      v1190_SoftClear(gVme,gTdcBase);
    }
#endif
    
#ifdef HAVE_CAEN_DGTZ
    if(lamDGTZ) {
      for(int i=0;i<nboard;i++){
	CAEN_DGTZ_ClearData(gDGTZ[i]);
      }
    }
#endif
    
    //Reset GATE (pulser B)
    //WRONG FOR V3718
    CAENVME_StopPulser(gVme->handle,cvPulserB);
    
#endif
    
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

INT read_event(char *pevent, INT off)
{

  rec_ev++;
  
  /* init bank structure */
  bk_init32(pevent);
  INT defaultEvSize = bk_size(pevent);

  //////READ SYSTEMS

#ifdef HAVE_CAEN_BRD
  
#ifdef HAVE_V1190
  read_tdc(pevent);
#endif

#ifdef HAVE_CAEN_DGTZ
  HNDLE hDB;

  cm_get_experiment_database(&hDB, NULL);

  bool freerun;
  int size = 4*sizeof(bool);

  db_get_value(hDB, 0, "/Configurations/FreeRunning",&freerun,&size,TID_BOOL,TRUE);
  if(!freerun) read_dgtz(pevent);

#endif

#endif
  
#ifdef HAVE_CAMERA
  read_camera(pevent);
#endif

  //////////////////
  
  ClearDevice();

  if (bk_size(pevent)==defaultEvSize ) { return 0; }
  return bk_size(pevent);

}

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
  data = 0x7F;
  CAENVME_WriteRegister(gVme->handle,cvLedPolRegClear,data);
  //WRONG FOR V3718
  data = 0x3E0;
  CAENVME_WriteRegister(gVme->handle,cvOutMuxRegClear,data);

  //USE OUT_1 as VME VETO and OUT_3 as LED driver
  //data = 0xCC;
  //CAENVME_WriteRegister(gVme->handle,cvOutMuxRegSet,data);

  //TO BE CHECKED FOR V3718
  //Set OUT_0 as pulser A for periodic trigger to the camera
  CAENVME_SetOutputConf(gVme->handle,cvOutput0,cvDirect,cvActiveHigh,cvMiscSignals);
  
  //TO BE CHECKED FOR V3718
  //Set OUT_2 as pulser B for a single gate
  CAENVME_SetOutputConf(gVme->handle,cvOutput2,cvInverted,cvActiveHigh,cvMiscSignals);

  //WRONG FOR V3718
  //USE OUT_1 as VME VETO
  data = 0xC;
  CAENVME_WriteRegister(gVme->handle,cvOutMuxRegSet,data);
  data = 0xC0;
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
      ret |= CAEN_DGTZ_SetMaxNumEventsBLT(gDGTZ[i],128);                                /* Set the max number of events to transfer in a sigle readout */
  
      //Acquisition mode
      ret |= CAEN_DGTZ_SetAcquisitionMode(gDGTZ[i],CAEN_DGTZ_SW_CONTROLLED);          /* Set the acquisition mode */

      if(ret != CAEN_DGTZ_Success) {  
	printf("Errors during Digitizer Initialization, board number %d.\n",i);
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
  DWORD period = 100000/410; //in units of 410 us
  DWORD width = 10; //in units of 410 us
  
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
	
    }
    
    //Uploading and enabling the automatic correction for the 1742 digitizer

    if(strcmp(BoardName[i],"V1742")==0)      //da decidere
      {
	if ((ret = CAEN_DGTZ_LoadDRS4CorrectionData(gDGTZ[i],DRS4Frequency)) != CAEN_DGTZ_Success) 
          { 
	    cerr<<"Error in LoadDRS4Correction"<<endl;
	    exit(EXIT_FAILURE);
          } 
	if ((ret = CAEN_DGTZ_EnableDRS4Correction(gDGTZ[i])) != CAEN_DGTZ_Success)
          { 
	    cerr<<"Error in EnableDRS4Correction"<<endl;
	    exit(EXIT_FAILURE);
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
INT ConfigCamera()
{
  
  HNDLE hDB;

  cm_get_experiment_database(&hDB, NULL);

  //Set exposure time in seconds
  DCAMERR err;

  double exposure;
  int size = sizeof(double);
  db_get_value(hDB, 0, "/Configurations/Exposure",&exposure,&size,TID_DOUBLE,TRUE);

  err = dcamprop_setvalue( gCam, DCAM_IDPROP_EXPOSURETIME, exposure);
  if(failed(err)) cout << "ERROR IN DCAM_IDPROP_EXPOSURETIME" << endl;

  int mode;
  size = sizeof(int);
  db_get_value(hDB, 0, "/Configurations/TriggerMode",&mode,&size,TID_INT,TRUE);
  if(mode==1) err = dcamprop_setvalue( gCam, DCAM_IDPROP_TRIGGER_GLOBALEXPOSURE, DCAMPROP_TRIGGER_GLOBALEXPOSURE__GLOBALRESET); 
  else if(mode==2) err = dcamprop_setvalue( gCam, DCAM_IDPROP_TRIGGER_GLOBALEXPOSURE, DCAMPROP_TRIGGER_GLOBALEXPOSURE__EMULATE); 
  else err = dcamprop_setvalue( gCam, DCAM_IDPROP_TRIGGER_GLOBALEXPOSURE, DCAMPROP_TRIGGER_GLOBALEXPOSURE__DELAYED);

  dcamprop_setvalue( gCam, DCAM_IDPROP_SENSORMODE, DCAMPROP_SENSORMODE__AREA);
  dcamprop_setvalue( gCam, DCAM_IDPROP_READOUTSPEED, DCAMPROP_READOUTSPEED__SLOWEST);
  dcamprop_setvalue( gCam, DCAM_IDPROP_INTERNAL_FRAMEINTERVAL, 0.033326);
  dcamprop_setvalue( gCam, DCAM_IDPROP_BITSPERCHANNEL, 16);
  dcamprop_setvalue( gCam, DCAM_IDPROP_BINNING, 1);
  dcamprop_setvalue( gCam, DCAM_IDPROP_FRAMEBUNDLE_MODE,DCAMPROP_MODE__OFF);
  dcamprop_setvalue( gCam, DCAM_IDPROP_DEFECTCORRECT_MODE, DCAMPROP_DEFECTCORRECT_MODE__OFF);
  dcamprop_setvalue( gCam, DCAM_IDPROP_SPOTNOISEREDUCER, DCAMPROP_MODE__OFF);
  
  return 0;
  
}
#endif

INT disable_trigger()
{

#ifdef HAVE_CAEN_BRD

  //WRONG FOR V3718
  //SET OUT_1 to 0 (busy)
  CAENVME_ClearOutputRegister(gVme->handle,cvOut1Bit);  

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
  
  ClearDevice();

#ifdef HAVE_CAEN_DGTZ
  for(int i=0;i<nboard;i++){
    CAEN_DGTZ_SWStartAcquisition(gDGTZ[i]);
  }
#endif

  return 0;

}

INT ClearDevice()
{

#ifdef HAVE_CAEN_BRD

#ifdef HAVE_V1190
  v1190_SoftClear(gVme,gTdcBase);
#endif
#ifdef HAVE_CAEN_DGTZ
  for(int i=0;i<nboard;i++){
    CAEN_DGTZ_ClearData(gDGTZ[i]);
  }
#endif

  //WRONG FOR V3718
  //SET OUT_1 to 1 (not busy)
  CAENVME_SetOutputRegister(gVme->handle,cvOut1Bit);  

  //TO BE CHECKD FOR V3718
  //Reset GATE (pulser B)
  CAENVME_StopPulser(gVme->handle,cvPulserB);

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

  uint32_t bsize;
  char * evtptr = NULL;
  uint32_t NumEvents;
  CAEN_DGTZ_EventInfo_t eventInfo;

  WORD* pdata16 = NULL;
  bk_create(pevent, "DIG0", TID_WORD, &pdata16);

  for(int i=0;i<nboard;i++){
    
    CAEN_DGTZ_ReadData(gDGTZ[i],CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,buffer_dgtz[i],&bsize);
    CAEN_DGTZ_GetNumEvents(gDGTZ[i],buffer_dgtz[i],bsize,&NumEvents);    

    //if(NumEvents != 1) cout << "---------- ERROR!!!!! DGTZ > 1 event!!! ----------" << endl;
    //cout << "NumEvents = " << NumEvents << endl;
    
    //#ifdef HAVE_V1761
    if(strcmp(BoardName[i],"V1761")==0 || strcmp(BoardName[i],"V1720E")==0){
      
      CAEN_DGTZ_UINT16_EVENT_t *Evt = NULL;
    
      for(int iev=0;iev<NumEvents;iev++){
      
	CAEN_DGTZ_AllocateEvent(gDGTZ[i], (void**)&Evt);
      
	CAEN_DGTZ_GetEventInfo(gDGTZ[i],buffer_dgtz[i],bsize,iev,&eventInfo,&evtptr);
	CAEN_DGTZ_DecodeEvent(gDGTZ[i],evtptr,(void**)&Evt);
      
	for(int j=0;j<NCHDGTZ[i];j++){
	
	  for (uint32_t k=0; k<ndgtz[i]; ++k) {
	  
	    uint16_t temp = (uint16_t)(Evt->DataChannel[j][k]);
	    *pdata16++ = temp;
	  
	  }
	
	}
      
	CAEN_DGTZ_FreeEvent(gDGTZ[i],&Evt);

      }

    }

    //#ifdef HAVE_V1742
    else if(strcmp(BoardName[i],"V1742")==0){

      
      CAEN_DGTZ_X742_EVENT_t *Evt = NULL;

      for(int iev=0;iev<NumEvents;iev++){

	CAEN_DGTZ_AllocateEvent(gDGTZ[i], (void**)&Evt);					

	CAEN_DGTZ_GetEventInfo(gDGTZ[i],buffer_dgtz[i],bsize,iev,&eventInfo,&evtptr);

	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_DecodeEvent(gDGTZ[i],evtptr,(void**)&Evt);

	for(int j=0;j<NCHDGTZ[i];j++){

	  uint32_t ig = j/8;
	  uint32_t ich = j%8;
	
	  for (uint32_t k=0; k<ndgtz[i]; ++k) {
	  
	    uint16_t temp = (uint16_t)(Evt->DataGroup[ig].DataChannel[ich][k]);
	    *pdata16++ = temp;
	  
	  }
	
	}
      
	CAEN_DGTZ_FreeEvent(gDGTZ[i],&Evt);
      
      }
    } 

      //#endif  
    
    bk_close(pevent, pdata16);
    
  }//end for on boards for reading data

  uint32_t* hdata = NULL;
  uint32_t header_data = 0;
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
    
    header_data = NumEvents;
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
    
    bk_close(pevent, hdata);

  }//end for on boards for header
  
  return 0;

}
#endif

#ifdef HAVE_CAMERA
INT read_camera(char *pevent)
{

  DCAMBUF_FRAME bufframe;
  memset( &bufframe, 0, sizeof(bufframe) );
  bufframe.size= sizeof(bufframe);
  bufframe.iFrame = -1;

  //////Read the data
  dcambuf_lockframe( gCam, &bufframe );

  //////Create the bank
  WORD* pdata = NULL;
  bk_create(pevent, "CAM0", TID_WORD, &pdata);

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

  const char* pSrc = (const char*)bufframe.buf;

  //////FULL MATRIX
  for(int y = 0; y < bufframe.height; y++ ){

    //Copy one row
    const unsigned short* pDst = (const unsigned short*)pSrc;

    //go through the row
    for(int x = 0; x < bufframe.width; x++ ){

      WORD tmpData = *pDst++; 

      *pdata++ = tmpData;

    }

    pSrc += bufframe.rowbytes;

  }
  
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
  
  bk_close(pevent, pdata);

  //dcambuf_release( gCam );

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
