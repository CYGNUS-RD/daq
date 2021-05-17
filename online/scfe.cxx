/********************************************************************\

  Name:         scfe.c
  Created by:   Francesco Renga

  Contentes: Slow Control Frontend program for CYGNUS_RD

  $Id$

\********************************************************************/

#include <stdio.h>
#include "midas.h"
#include "mfe.h"
#include "class/hv.h"
#include "gem_hv.h"
#include "gem_dd_sy4527.h"
#include "iseg_hps.h"
#include "bus/null.h"

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
const char *frontend_name = "SC Frontend";
/* The frontend file name, don't change it */
const char *frontend_file_name = __FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = FALSE;

/* a frontend status page is displayed with this frequency in ms    */
INT display_period = 1000;

/* maximum event size produced by this frontend */
INT max_event_size = 10000;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 5 * 1024 * 1024;

/* buffer size to hold events */
INT event_buffer_size = 10 * 10000;

/*-- Equipment list ------------------------------------------------*/

/* device driver list */
DEVICE_DRIVER sy4527_driver[] = {
  {"sy4527", dd_sy4527, 64, null, DF_PRIO_DEVICE|DF_REPORT_CHSTATE|DF_REPORT_STATUS|DF_HW_RAMP},
  {""}
};
/*
DEVICE_DRIVER iseg_hps_driver[] = {
  {"iseg_hps", iseg_hps, 1, null, DF_PRIO_DEVICE|DF_REPORT_CHSTATE|DF_REPORT_STATUS|DF_HW_RAMP},
  {""}
};
*/
EQUIPMENT equipment[] = {

   {"HV",                       /* equipment name */
    {3, 0,                       /* event ID, trigger mask */
     "SYSTEM",                  /* event buffer */
     EQ_SLOW,                   /* equipment type */
     0,                         /* event source */
     "FIXED",                   /* format */
     TRUE,                      /* enabled */
     RO_RUNNING | RO_TRANSITIONS,        /* read when running and on transitions */
     60000,                     /* read every 60 sec */
     0,                         /* stop run after this event limit */
     0,                         /* number of sub events */
     10000,                         /* log history every event */
     "", "", ""} ,
    cd_gem_hv_read,                 /* readout routine */
    cd_gem_hv,                      /* class driver main routine */
    sy4527_driver,                  /* device driver list */
    NULL,                       /* init string */
    },

   // {"CATHODE",                       /* equipment name */
   //  {3, 0,                       /* event ID, trigger mask */
   //   "SYSTEM",                  /* event buffer */
   //   EQ_SLOW,                   /* equipment type */
   //   0,                         /* event source */
   //   "FIXED",                   /* format */
   //   TRUE,                      /* enabled */
   //   RO_RUNNING | RO_TRANSITIONS,        /* read when running and on transitions */
   //   60000,                     /* read every 60 sec */
   //   0,                         /* stop run after this event limit */
   //   0,                         /* number of sub events */
   //   10000,                         /* log history every event */
   //   "", "", ""} ,
   //  cd_hv_read,                 /* readout routine */
   //  cd_hv,                      /* class driver main routine */
   //  iseg_hps_driver,                  /* device driver list */
   //  NULL,                       /* init string */
   //  },

   {""}
   
};


/*-- Dummy routines ------------------------------------------------*/

INT poll_event(INT source, INT count, BOOL test)
{
   return 1;
};
INT interrupt_configure(INT cmd, INT source, POINTER_T adr)
{
   return 1;
};

/*-- Frontend Init -------------------------------------------------*/

INT frontend_init()
{
   return CM_SUCCESS;
}

/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit()
{
   return CM_SUCCESS;
}

/*-- Frontend Loop -------------------------------------------------*/

INT frontend_loop()
{
   return CM_SUCCESS;
}

/*-- Begin of Run --------------------------------------------------*/

INT begin_of_run(INT run_number, char *error)
{
   return CM_SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/

INT end_of_run(INT run_number, char *error)
{
   return CM_SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/

INT pause_run(INT run_number, char *error)
{
   return CM_SUCCESS;
}

/*-- Resume Run ----------------------------------------------------*/

INT resume_run(INT run_number, char *error)
{
   return CM_SUCCESS;
}

/*------------------------------------------------------------------*/
