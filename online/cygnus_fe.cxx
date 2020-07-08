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

using namespace std;

#define HAVE_CAEN_BRD
#define HAVE_CAMERA

#ifdef HAVE_CAEN_BRD
#define HAVE_V895
#define HAVE_V1190
#define HAVE_V1761
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
#ifdef HAVE_V1761
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
INT max_event_size = 300000000;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 5 * 1024 * 1024;

/* buffer size to hold events */
INT event_buffer_size = 600000000;

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

/*-- Equipment list ------------------------------------------------*/

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
#endif

#ifdef HAVE_V1761
int gDGTZ;
char *buffer_dgtz = NULL;
int NCHDGTZ = 32;
double DGTZ_OFFSET[32] = {0.};
int ndgtz = 100000;
int posttrg = 70;
#endif

#ifdef HAVE_CAMERA
HDCAM gCam = 0;
HDCAMWAIT hwait = 0;
#endif

int gDisBase   = 0xEE000000;
int gTdcBase   = 0x33330000;
int gDigBase   = 0x22220000;

/*-- Frontend Init -------------------------------------------------*/

INT frontend_init()
{
  /* put any hardware initialization here */
#ifdef HAVE_V1761
  CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB,0,0,gDigBase,&gDGTZ);
  if(ret != CAEN_DGTZ_Success) {
    printf("Can't open digitizer -- Error %d\n",ret);
  }
#endif

#ifdef HAVE_CAEN_BRD

  CaenBrdgSetPar(cvV1718,0);

  int status = mvme_open(&gVme,1);

  printf("gVme %d status %d\n",gVme,status);

  mvme_set_am(gVme, MVME_AM_A32/*MVME_AM_A24_ND*/);

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
  
  return SUCCESS;

}

/*-- Begin of Run --------------------------------------------------*/

INT begin_of_run(INT run_number, char *error)
{
  /* put here clear scalers etc. */
#ifdef HAVE_CAEN_BRD

  ConfigBridge();
  CAENVME_StartPulser(gVme->handle,cvPulserA);

#ifdef HAVE_V1761
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
  int size = sizeof(int);
  HNDLE hDB;

  cm_get_experiment_database(&hDB, NULL);

  db_get_value(hDB, 0, "/Configurations/MaxEvents",&maxevents,&size,TID_INT,TRUE);

  double events;
  size = sizeof(double);
  db_get_value(hDB, 0, "/Equipment/Trigger/Statistics/Events sent",&events,&size,TID_DOUBLE,TRUE);

  if(events >= maxevents) return 0;
  
  int i;
  DWORD flag;
  
  int lamTDC = 1;
  int lamDGTZ = 1;
  int lamCAM = 1;

  count = 1;

  if (count > 100) count = 100;
  
  for (i = 0; i < count; i++) {

    /* poll hardware and set flag to TRUE if new event is available */
#ifdef HAVE_CAMERA
    
    //wait for frame ready
    
    double exposure;
    dcamprop_getvalue( gCam, DCAM_IDPROP_EXPOSURETIME, &exposure);
    
    DCAMWAIT_START waitstart;
    memset( &waitstart, 0, sizeof(waitstart) );
    waitstart.size = sizeof(waitstart);
    waitstart.timeout = (int)(2.*exposure*1000) + 30; //in ms --> max wait = exposure + USB transfer time 
    
    //send a trigger to the camera
    dcamcap_firetrigger(gCam,0);

    waitstart.eventmask = DCAMWAIT_CAPEVENT_EXPOSUREEND;
    DCAMERR err1 = dcamwait_start( hwait, &waitstart );
    
    if(failed(err1) || err1 == DCAMERR_TIMEOUT) lamCAM = 0;

#endif

#ifdef HAVE_V1190
    lamTDC = v1190_DataReady(gVme,gTdcBase);
#endif
    
#ifdef HAVE_V1761
    uint32_t status;
    CAEN_DGTZ_ReadRegister(gDGTZ,CAEN_DGTZ_ACQ_STATUS_ADD,&status); /* read status register */
    lamDGTZ = (status & 0x8); /* 4th bit is data ready */
#endif
    
    flag = (lamTDC && lamDGTZ && lamCAM);
    
    if (flag){
      if (!test){

#ifdef HAVE_CAEN_BRD	
	//SET OUT_1 to 0 (busy)
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
    DCAMBUF_FRAME bufframe;
    waitstart.eventmask = DCAMWAIT_CAPEVENT_FRAMEREADY;
    DCAMERR err2 = dcamwait_start( hwait, &waitstart );
    //dcambuf_lockframe( gCam, &bufframe );
    //dcambuf_release( gCam );
#endif
        
#ifdef HAVE_CAEN_BRD
    
#ifdef HAVE_V1190
    if(lamTDC) {
      v1190_SoftClear(gVme,gTdcBase);
    }
#endif
    
#ifdef HAVE_V1761
    if(lamDGTZ) {
      CAEN_DGTZ_ClearData(gDGTZ);
    }
#endif
    
    //SET OUT_1 to 1 (not busy)
    CAENVME_SetOutputRegister(gVme->handle,cvOut1Bit);
    
    //Reset GATE (pulser B)
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

  /* init bank structure */
  bk_init32(pevent);
  INT defaultEvSize = bk_size(pevent);

  //////READ SYSTEMS

#ifdef HAVE_CAEN_BRD
  
#ifdef HAVE_V1190
  read_tdc(pevent);
#endif
  
#ifdef HAVE_V1761
  read_dgtz(pevent);
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

#ifdef HAVE_CAEN_BRD

INT init_vme_modules(){

  /* BRIDGE INITIALIZATION */
  unsigned int data;

  //SET POSITIVE POLARITIES OF LEDS AND SIGNALS
  data = 0x7F;
  CAENVME_WriteRegister(gVme->handle,cvLedPolRegClear,data);
  data = 0x3E0;
  CAENVME_WriteRegister(gVme->handle,cvOutMuxRegClear,data);

  //USE OUT_1 as VME VETO
  data = 0xC;
  CAENVME_WriteRegister(gVme->handle,cvOutMuxRegSet,data);

  //Set OUT_0 as pulser A for periodic trigger to the camera
  CAENVME_SetOutputConf(gVme->handle,cvOutput0,cvDirect,cvActiveHigh,cvMiscSignals);
  
  //Set OUT_2 as pulser B for a single gate
  CAENVME_SetOutputConf(gVme->handle,cvOutput2,cvInverted,cvActiveHigh,cvMiscSignals);
  
  ConfigBridge();

  CAENVME_StopPulser(gVme->handle,cvPulserA);
  CAENVME_StopPulser(gVme->handle,cvPulserB);
  
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
  v1190_OffsetSet_ns(gVme,gTdcBase,-400); //-400
  v1190_WidthSet_ns(gVme,gTdcBase,1200); //1200
  v1190_SubtractTriggerTimeON(gVme,gTdcBase);
  v1190_TriggerMatchingSet(gVme,gTdcBase);
  v1190_SetMaxNOfHitsPerEve(gVme,gTdcBase,V1190_HE_32);
  v1190_EnableERRORMark(gVme,gTdcBase);
  v1190_EnableHeaderAndTrailer(gVme,gTdcBase);
  v1190_Status(gVme,gTdcBase,V1190B);

#endif

#ifdef HAVE_V1761
  
  /* DIGITIZER INITIALIZATION */
  
  CAEN_DGTZ_BoardInfo_t BoardInfo;
  CAEN_DGTZ_ErrorCode ret;
  
  ret = CAEN_DGTZ_GetInfo(gDGTZ, &BoardInfo);
  printf("\nConnected to CAEN Digitizer Model %s\n", BoardInfo.ModelName);
  
  ////Reset the board
  ret |= CAEN_DGTZ_Reset(gDGTZ);                                               /* Reset Digitizer */
  ret |= CAEN_DGTZ_ClearData(gDGTZ);
  ret |= CAEN_DGTZ_GetInfo(gDGTZ, &BoardInfo);                                 /* Get Board Info */
  
  ////Waveform Setup
  //ret |= CAEN_DGTZ_SetChannelEnableMask(gDGTZ,3);                              /* Enable channel 0 and 1*/
  //ret |= CAEN_DGTZ_SetRecordLength(gDGTZ,4096);                                /* Set the lenght of each waveform (in samples) */
  //ret |= CAEN_DGTZ_SetPostTriggerSize(gDGTZ,32);                               /* Trigger position */
  
  ret |= CAEN_DGTZ_SetSWTriggerMode(gDGTZ,CAEN_DGTZ_TRGMODE_DISABLED);
  ret |= CAEN_DGTZ_SetChannelSelfTrigger(gDGTZ,CAEN_DGTZ_TRGMODE_DISABLED,3);
  ret |= CAEN_DGTZ_SetExtTriggerInputMode(gDGTZ,CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT);
  ret |= CAEN_DGTZ_SetMaxNumEventsBLT(gDGTZ,16);                                /* Set the max number of events to transfer in a sigle readout */
  
  //Acquisition mode
  ret |= CAEN_DGTZ_SetAcquisitionMode(gDGTZ,CAEN_DGTZ_SW_CONTROLLED);          /* Set the acquisition mode */

  if(ret != CAEN_DGTZ_Success) {  
    printf("Errors during Digitizer Initialization.\n");
  }
  
  ConfigDgtz();
  
#endif  

  return 0;
  
}
  
#endif

#ifdef HAVE_CAEN_BRD
INT ConfigBridge(){

  //Configure pulser A for camera trigger
  //---Start and reset from SW
  CVTimeUnits unit = cvUnit410us;
  DWORD period = 100000/410; //in units of 410 us
  DWORD width = 10; //in units of 410 us
  CAENVME_SetPulserConf(gVme->handle,cvPulserA,period,width,unit,0,cvManualSW,cvManualSW);

  //Configure pulser B for a single gate
  //---Start from IN_0, reset from SW, infinite length
  unit = cvUnit25ns;
  period = 100; //in units of 104 ms 
  width = 255; //in units of 104 ms
  CAENVME_SetPulserConf(gVme->handle,cvPulserB,period,width,unit,0,cvInputSrc0,cvManualSW);

  return 0;
  
}
#endif

#ifdef HAVE_V1761
INT ConfigDgtz(){

  CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;

  int size = sizeof(int);

  HNDLE hDB;

  cm_get_experiment_database(&hDB, NULL);
  
  db_get_value(hDB, 0, "/Configurations/DigitizerChannels",&NCHDGTZ,&size,TID_INT,TRUE);
  db_get_value(hDB, 0, "/Configurations/DigitizerSamples",&ndgtz,&size,TID_INT,TRUE);
  db_get_value(hDB, 0, "/Configurations/DigitizerPostTrg",&posttrg,&size,TID_INT,TRUE);

  if(NCHDGTZ > 32) NCHDGTZ = 32;                                           /* max 5 bit to store the number of channels */
  if(ndgtz > 8388608) ndgtz = 8388608;                                         /* max 23 bit to allocate the number of samples */
  ret |= CAEN_DGTZ_SetChannelEnableMask(gDGTZ,0xFFFFFFFF);                              /* Enable all channels */
  ret |= CAEN_DGTZ_SetRecordLength(gDGTZ,ndgtz);                                /* Set the lenght of each waveform (in samples) */
  ret |= CAEN_DGTZ_SetPostTriggerSize(gDGTZ,posttrg);                               /* Trigger position */
  
  size = sizeof(double);
  
  for(int ich=0;ich<NCHDGTZ;ich++){
    
    char name[64];
    sprintf(name,"/Configurations/DigitizerOffset[%d]",ich);
    
    db_get_value(hDB,0,name,&DGTZ_OFFSET[ich],&size,TID_DOUBLE,TRUE);
    
    if(DGTZ_OFFSET[ich] > 0.5) DGTZ_OFFSET[ich] = 0.5;
    else if(DGTZ_OFFSET[ich] < -0.5) DGTZ_OFFSET[ich] = -0.5;
    
    ret |= CAEN_DGTZ_SetChannelDCOffset(gDGTZ,0,(int)(DGTZ_OFFSET[ich]*65536 + 32768));

  }
  
  //Calibration
  ret |= CAEN_DGTZ_Calibrate(gDGTZ);
  
  //Buffer allocation
  uint32_t bsize;
  ret = CAEN_DGTZ_MallocReadoutBuffer(gDGTZ,&buffer_dgtz,&bsize);

  if(ret != CAEN_DGTZ_Success) {
    printf("Errors during Digitizer Configuration.\n");
  }

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

  //SET OUT_1 to 0 (busy)
  CAENVME_ClearOutputRegister(gVme->handle,cvOut1Bit);  

#ifdef HAVE_V1761
  CAEN_DGTZ_SWStopAcquisition(gDGTZ);
#endif

#endif 

  return 0;
  
}
 
INT enable_trigger()
{
  
  ClearDevice();

#ifdef HAVE_V1761
  CAEN_DGTZ_SWStartAcquisition(gDGTZ);
#endif

  return 0;

}

INT ClearDevice()
{

#ifdef HAVE_CAEN_BRD

#ifdef HAVE_V1190
  v1190_SoftClear(gVme,gTdcBase);
#endif
#ifdef HAVE_V1761
  CAEN_DGTZ_ClearData(gDGTZ);
#endif

  //SET OUT_1 to 1 (not busy)
  CAENVME_SetOutputRegister(gVme->handle,cvOut1Bit);  

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

#ifdef HAVE_V1761
int read_dgtz(char* pevent){

  uint32_t bsize;
  char * evtptr = NULL;
  uint32_t NumEvents;
  CAEN_DGTZ_EventInfo_t eventInfo;
  CAEN_DGTZ_UINT16_EVENT_t *Evt = NULL;

  CAEN_DGTZ_ReadData(gDGTZ,CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,buffer_dgtz,&bsize);
  CAEN_DGTZ_GetNumEvents(gDGTZ,buffer_dgtz,bsize,&NumEvents);

  //if(NumEvents != 1) cout << "---------- ERROR!!!!! DGTZ > 1 event!!! ----------" << endl;
  cout << "NumEvents = " << NumEvents << endl;

  DWORD* hdata = NULL;
  bk_create(pevent, "DIGH", TID_DWORD, &hdata);

  DWORD header_data = 0;
  header_data |= (ndgtz & 0x7FFFFF); //23 bits for the number of samples per event
  header_data |= ((NCHDGTZ & 0x1F) << 23); //5 bits for the number of channels
  header_data |= ((NumEvents & 0xF) << 28); //4 bits for the number of events
  *hdata++ = header_data;

  for(int j=0;j<NCHDGTZ;j++){
    header_data = (uint32_t)(DGTZ_OFFSET[j]*65536 + 32768);
    *hdata++ = header_data;
  }

  bk_close(pevent, header_data);

  WORD* pdata16 = NULL;
  bk_create(pevent, "DIG0", TID_WORD, &pdata16);

  for(int j=0;j<NCHDGTZ;j++){

    for(int iev=0;iev<NumEvents;iev++){

      CAEN_DGTZ_GetEventInfo(gDGTZ,buffer_dgtz,bsize,iev,&eventInfo,&evtptr);
      CAEN_DGTZ_DecodeEvent(gDGTZ,evtptr,&Evt);

      for (uint32_t i=0; i<ndgtz; ++i) {

	uint16_t temp = Evt->DataChannel[j][i];
	*pdata16++ = temp;

      }

      CAEN_DGTZ_FreeEvent(gDGTZ,&Evt);

    }

    bk_close(pevent, pdata16);

  }

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

  /*
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
  */

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
  
  bk_close(pevent, pdata);

  //dcambuf_release( gCam );

  return 1;

}
#endif
