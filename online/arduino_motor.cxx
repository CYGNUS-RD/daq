/********************************************************************\

  Name:         arduino_motor.h
  Created by:   Francesco Renga

  Contents:     Device driver to control motor through Arduino

  $Id$

\********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>   /* UNIX standard function definitions */
#include <fcntl.h>    /* File control definitions */
#include <errno.h>    /* Error number definitions */
#include <termios.h>  /* POSIX terminal control definitions */
#include <sys/ioctl.h>
#include <math.h>

#include "midas.h"

/*---- globals -----------------------------------------------------*/

#define DEFAULT_TIMEOUT 10000   /* 10 sec. */

/* Store any parameters the device driver needs in following 
   structure. Edit the ARDUINO_SETTINGS_STR accordingly. This 
   contains usually the address of the device. For a CAMAC device
   this could be crate and station for example. */

typedef struct {
  char port[256];
  int baud;
} ARDUINO_SETTINGS;

#define ARDUINO_SETTINGS_STR "\
Port = STRING : [256] /dev/ttyACM0\n\
Baud = INT : 9600\n\
"

/* following structure contains private variables to the device
   driver. It is necessary to store it here in case the device
   driver is used for more than one device in one frontend. If it
   would be stored in a global variable, one device could over-
   write the other device's variables. */

typedef struct {
  ARDUINO_SETTINGS arduino_settings;
  int fd;
  HNDLE hkey;                  /* ODB key for driver info */
} ARDUINO_INFO;

/*---- device driver routines --------------------------------------*/

typedef INT(func_t) (INT cmd, ...);

/* the init function creates a ODB record which contains the
   settings and initialized it variables as well as the bus driver */

INT arduino_motor_init(HNDLE hkey, ARDUINO_INFO **pinfo)
{
   int status, size;
   HNDLE hDB, hkeydd;
   ARDUINO_INFO *info;
   struct termios toptions;

   /* allocate info structure */
   info = (ARDUINO_INFO*) calloc(1, sizeof(ARDUINO_INFO));
   *pinfo = info;

   cm_get_experiment_database(&hDB, NULL);

   /* create ARDUINO settings record */
   status = db_create_record(hDB, hkey, "DD", ARDUINO_SETTINGS_STR);
   if (status != DB_SUCCESS)
      return FE_ERR_ODB;

   db_find_key(hDB, hkey, "DD", &hkeydd);
   size = sizeof(info->arduino_settings);
   db_get_record(hDB, hkeydd, &info->arduino_settings, &size, 0);

   info->hkey = hkey;

   if (status != SUCCESS)
      return status;

   /* initialization of device, something like ... */
   printf("\nConnecting to %s...\n",info->arduino_settings.port);
   info->fd = open(info->arduino_settings.port, O_RDWR | O_NOCTTY);;
   
   if(info->fd == -1){
     printf("Connection failed\n");
     return FE_ERR_HW;
   }
   
   if (tcgetattr(info->fd, &toptions) < 0) {
     printf("Couldn't get term attributes\n");
     return FE_ERR_HW;
   }

   cfsetispeed(&toptions, info->arduino_settings.baud);
   cfsetospeed(&toptions, info->arduino_settings.baud);

   toptions.c_cflag &= ~PARENB;
   toptions.c_cflag &= ~CSTOPB;
   toptions.c_cflag &= ~CSIZE;
   toptions.c_cflag |= CS8;
   toptions.c_cflag &= ~CRTSCTS;
   toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
   toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl
   toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
   toptions.c_oflag &= ~OPOST; // make raw
   toptions.c_cc[VMIN]  = 0;
   toptions.c_cc[VTIME] = 20;

   if(tcsetattr(info->fd, TCSANOW, &toptions) < 0) {
     printf("Couldn't set term attributes\n");
     return FE_ERR_HW;
   }

   printf("connection done\n");
   
   return FE_SUCCESS;

}

/*----------------------------------------------------------------------------*/

INT arduino_motor_exit(ARDUINO_INFO * info)
{

  //Close the communication with Arduino
  close(info->fd);
  
  free(info);
  
  return FE_SUCCESS;
}

/*----------------------------------------------------------------------------*/

INT arduino_motor_set(ARDUINO_INFO * info, INT channel, float value)
{

  printf("arduino set\n");

  char cmd[80];

  if(channel == 0) {
    if(roundf(value) > -0.5 && roundf(value) < 9.5) sprintf(cmd, "R%d", (int)roundf(value));
    else return FE_SUCCESS;
  }
  else if(channel == 1){
    if(roundf(value) == 0) sprintf(cmd, "F");
    else if(roundf(value) == 1) sprintf(cmd, "B");
    else return FE_SUCCESS;
  }

  int len = strlen(cmd);
  int n = write(info->fd, cmd, len);

  usleep(1000);
  tcflush(info->fd,TCIOFLUSH);

  if(n!=len) return FE_ERR_HW;
  
  return FE_SUCCESS;
  
}

/*----------------------------------------------------------------------------*/

INT arduino_motor_get(ARDUINO_INFO * info, INT channel, float *pvalue)
{

  tcflush(info->fd,TCIOFLUSH);
  //sleep(2);
  
  char cmd[80];

  /* read value from channel, something like ... */
  if(channel == 0) sprintf(cmd, "A");
  else if(channel < 11) sprintf(cmd, "P%d", channel-1);
  else {
    *pvalue = (float)ss_nan();
    return FE_SUCCESS;
  }

  int len = strlen(cmd);
  int n = write(info->fd, cmd, len);

  if(n!=len) return FE_ERR_HW;

  tcdrain(info->fd);

  usleep(10000);
  
  char buf[80];
  char b[1];
  int i = 0;

  DWORD t0 = ss_time();
  DWORD timeout = 10;

  while(1) { 

    n = read(info->fd, b, 1);  // read a char at a time

    if( n==-1) {
      i = 0;
      break;    // couldn't read
    }
    if( n==0 ) {
      printf("sleep\n");
      usleep( 100 * 1000 ); // wait 10 msec try again
      continue;
    }

    if(b[0] == '\n') break;
    if(ss_time() > t0 + timeout) {
      printf("timeout\n");
      i=0;
      break;
    }
    
    buf[i] = b[0];
    i++;

  }
  
  printf("%s %s\n",cmd,buf);
  tcflush(info->fd,TCIOFLUSH);

  if(i>0){
    *pvalue = atof(buf);
    return FE_SUCCESS;
  }
  else{
    *pvalue = (float)ss_nan();
    return FE_ERR_HW;
  }

}

/*---- device driver entry point -----------------------------------*/

INT arduino_motor(INT cmd, ...)
{
   va_list argptr;
   HNDLE hKey;
   INT channel, status;
   float value, *pvalue;
   ARDUINO_INFO *info;

   va_start(argptr, cmd);
   status = FE_SUCCESS;

   switch (cmd) {
   case CMD_INIT: {
      hKey = va_arg(argptr, HNDLE);
      ARDUINO_INFO** pinfo = va_arg(argptr, ARDUINO_INFO **);
      status = arduino_motor_init(hKey, pinfo);
      break;
   }
   case CMD_EXIT:
      info = va_arg(argptr, ARDUINO_INFO *);
      status = arduino_motor_exit(info);
      break;

   case CMD_SET:
      info = va_arg(argptr, ARDUINO_INFO *);
      channel = va_arg(argptr, INT);
      value = (float) va_arg(argptr, double);   // floats are passed as double
      status = arduino_motor_set(info, channel, value);
      break;

   case CMD_GET_DIRECT:
   case CMD_GET:
      info = va_arg(argptr, ARDUINO_INFO *);
      channel = va_arg(argptr, INT);
      pvalue = va_arg(argptr, float *);
      status = arduino_motor_get(info, channel, pvalue);
      break;

   default:
      break;
   }

   va_end(argptr);

   return status;
}

/*------------------------------------------------------------------*/
