/********************************************************************\

Name:         iseg_hps.c
Created by:   Pierre-Andre Amaudruz

Contents:     ISEG HPS Device driver. 

$Id: iseg_hps.c 2780 2005-10-19 13:20:29Z ritt $

\********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include "mfe.h"
#include "msystem.h"
#include <iostream>
#define ALARM XALARM
#include "midas.h"
#undef ALARM

/*---- globals -----------------------------------------------------*/

#define DEFAULT_TIMEOUT 10000	/* 10 sec. */

/* Store any parameters the device driver needs in following 
structure.  Edit the ISEGHPS_SETTINGS_STR accordingly. This 
contains  usually the address of the device. For a CAMAC device
this  could be crate and station for example. */

typedef struct
{
  char name[32];		// System name (iseg_hps)
  char ip[32];			// IP# for network access
  INT port;
} ISEGHPS_SETTINGS;

#define ISEGHPS_SETTINGS_STR "\
System Name = STRING : [32] iseg_hps\n\
IP = STRING : [32] 0.0.0.0\n\
Port = INT : 10001\n\
"

/* following structure contains private variables to the device
driver. It  is necessary to store it here in case the device
driver  is used for more than one device in one frontend. If it
would be  stored in a global variable, one device could over-
write the other device's  variables. */

typedef struct
{
  int handle;
  ISEGHPS_SETTINGS iseg_hps_settings;
  float *array;
  INT num_channels;		// Total # of channels
  INT (*bd) (INT cmd, ...);	/* bus driver entry function */
  void *bd_info;		/* private info of bus driver */
  INT sock;
  HNDLE hkey;			/* ODB key for bus driver info */
} ISEGHPS_INFO;

#define MAX_NAME 32

std::map<INT, char*> iseg_hps_label;

void get_slot (ISEGHPS_INFO * info, WORD channel, WORD * chan, WORD * slot);
INT iseg_hps_lParam_set (ISEGHPS_INFO * info, WORD nchannel, WORD , char const *ParName, DWORD * lvalue);
INT iseg_hps_lParam_get (ISEGHPS_INFO * info, WORD nchannel, WORD , char const  *ParName, DWORD * lvalue);
INT iseg_hps_fParam_set (ISEGHPS_INFO * info, WORD nchannel, WORD , char const  *ParName, float *fvalue);
INT iseg_hps_fParam_get (ISEGHPS_INFO * info, WORD nchannel, WORD , char const *ParName, float *fvalue);
INT iseg_hps_fBoard_set (ISEGHPS_INFO * info, WORD nchannel, WORD , char const *ParName, float *fvalue);
INT iseg_hps_fBoard_get (ISEGHPS_INFO * info, WORD nchannel, WORD , char const *ParName, float *fvalue);

/*---- device driver routines --------------------------------------*/
/* the init function creates a ODB record which contains the
settings  and initialized it variables as well as the bus driver */

static INT tcp_connect(char *host, int port, int *sock)
{

  int status;
  struct sockaddr_in  sockaddr_in;
  char str[256];
  
  *sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  memset(&sockaddr_in, 0, sizeof(sockaddr_in));
  sockaddr_in.sin_family = AF_INET; // UDP, TCP
  sockaddr_in.sin_port = htons((short) port); // remote Port
  //inet_pton(AF_INET, host, &(sockaddr_in.sin_addr));
  sockaddr_in.sin_addr.s_addr = inet_addr(host);

  // connect to server (three way handshake)
  do {

    status = connect(*sock, (const struct sockaddr *)&sockaddr_in, sizeof(sockaddr_in));
    
    /* don't return if an alarm signal was cought */
  } while (status == -1 && errno == EINTR);
  
  if (status != 0) {
    closesocket(*sock);
    *sock = -1;
    sprintf(str, "Cannot connect to host %s", host);
    mfe_error(str);
    return FE_ERR_HW;
  }

  return FE_SUCCESS;
  
}

/*----------------------------------------------------------------------------*/
INT iseg_hps_read (ISEGHPS_INFO * info, char *cmd, char *ans){

  int retcode;
  char answ[255];
  char buf[255];
  memset(buf,'\0',255);
  memset(answ,'\0',255);
  char *crlf;

  int tot_ret = 0;
  
  //tcp_connect(info->iseg_hps_settings.ip,info->iseg_hps_settings.port,&info->sock);

  send(info->sock, cmd, strlen(cmd), 0);

  do {

    retcode = recv(info->sock, buf, sizeof(answ), 0);

    if (retcode > 0) {
      buf[retcode] = 0;
      strcat(answ, buf);
      tot_ret += retcode;
    }
    
    crlf = strstr(answ, "\r\n");

  } while ( (retcode > 0) && (crlf == 0) );
  
  if (crlf > 0) {
    *crlf = 0;
  }

  strcpy(ans,answ);
  
  //closesocket(info->sock);

  return (tot_ret > 0) ? FE_SUCCESS : 0;
  
}

INT iseg_hps_write (ISEGHPS_INFO * info, char *cmd){

  //tcp_connect(info->iseg_hps_settings.ip,info->iseg_hps_settings.port,&info->sock);

  int stat = send(info->sock, cmd, strlen(cmd), 0);

  //closesocket(info->sock);

  if(stat > 0) return FE_SUCCESS;

  return 0;
  
}

INT iseg_hps_init (HNDLE hkey, void **pinfo, WORD channels,
                INT (*bd) (INT cmd, ...))
{

  int status, size;
  HNDLE hDB, hkeydd;
  ISEGHPS_INFO *info;

  /*  allocate info structure */
  info = (ISEGHPS_INFO *) calloc (1, sizeof (ISEGHPS_INFO));
  *pinfo = info;

  cm_get_experiment_database (&hDB, NULL);

  /*  create ISEGHPS settings record */
  status = db_create_record (hDB, hkey, "DD", ISEGHPS_SETTINGS_STR);
  if (status != DB_SUCCESS)
    return FE_ERR_ODB;

  db_find_key (hDB, hkey, "DD", &hkeydd);
  size = sizeof (info->iseg_hps_settings);
  db_get_record (hDB, hkeydd, &info->iseg_hps_settings, &size, 0);
  
  //  Connect to device
  status = tcp_connect(info->iseg_hps_settings.ip,info->iseg_hps_settings.port,&info->sock);
  status = FE_SUCCESS;
  if(status == FE_SUCCESS)
    cm_msg (MINFO, "iseg_hps", "Connected to IP: %s", info->iseg_hps_settings.ip);
  else return 0;

  info->num_channels = 1;
  info->array = (float *) calloc (channels, sizeof (float));
  info->hkey = hkey;

  iseg_hps_label[info->handle] = new char[8];
  sprintf(iseg_hps_label[info->handle],"ISEG_HPS");

  //KILL ENABLE MODE
  //char scmd[255] = ":CONF:KILL 1";

  //int ret = iseg_hps_write(info,scmd);
  
  return FE_SUCCESS;
  
}

/*----------------------------------------------------------------------------*/
INT iseg_hps_exit (ISEGHPS_INFO * info)
{
  /*  free local variables */
  if (info->array)
    free (info->array);

  free (info);

  closesocket(info->sock);
  
  return FE_SUCCESS;
}

/*----------------------------------------------------------------------------*/
INT iseg_hps_Label_set (ISEGHPS_INFO * info, WORD channel, char *label)
{

  if (strlen (label) < MAX_NAME)
  {

    strcpy(iseg_hps_label[info->handle],label);

    return FE_SUCCESS;

  }
  else
    return 0;
}

/*----------------------------------------------------------------------------*/
INT iseg_hps_Label_get (ISEGHPS_INFO * info, WORD channel, char *label)
{

  strcpy(label,iseg_hps_label[info->handle]);  

  return FE_SUCCESS;
  
}

/*----------------------------------------------------------------------------*/
/**  Get voltage
*/
INT iseg_hps_get (ISEGHPS_INFO * info, WORD channel, float *pvalue)
{

  char ans[255];  
  char cmd[255] = ":MEAS:VOLT?\r\n";

  int ret = iseg_hps_read(info,cmd,ans);
  if(ret == FE_SUCCESS){
    char *val = strtok(ans,"E");
    *pvalue = atof(val)*1.e3;
    return FE_SUCCESS;
  }

  return 0;
  
}

/*----------------------------------------------------------------------------*/
INT iseg_hps_demand_get (ISEGHPS_INFO * info, WORD channel, float *pvalue)
{

  char ans[255];  
  char cmd[255] = ":READ:VOLT?\r\n";

  int ret = iseg_hps_read(info,cmd,ans);
  if(ret == FE_SUCCESS){
    char *val = strtok(ans,"E");
    *pvalue = atof(val)*1.e3;
    return FE_SUCCESS;
  }

  return 0;

}

/*----------------------------------------------------------------------------*/
INT iseg_hps_current_get (ISEGHPS_INFO * info, WORD channel, float *pvalue)
{

  char ans[255];  
  char cmd[255] = ":MEAS:CURR?\r\n";

  int ret = iseg_hps_read(info,cmd,ans);

  if(ret == FE_SUCCESS){
    char *val = strtok(ans,"E");
    *pvalue = atof(val)*1.e3;
    return FE_SUCCESS;
  }
  
  return 0;

}

/*----------------------------------------------------------------------------*/
INT iseg_hps_current_limit_get (ISEGHPS_INFO * info, WORD channel, float *pvalue)
{

  char ans[255];  
  char cmd[255] = ":READ:CURR?\r\n";

  int ret = iseg_hps_read(info,cmd,ans);
  if(ret == FE_SUCCESS){
    char *val = strtok(ans,"E");
    *pvalue = atof(val)*1.e3;
    return FE_SUCCESS;
  }

  return 0;

}

/*---------------------------------------------------------------------------*/
/** Set demand voltage */
INT iseg_hps_set (ISEGHPS_INFO * info, WORD channel, float value)
{

  char cmd[255];

  sprintf(cmd,":VOLT %f\r\n",value);

  return iseg_hps_write(info,cmd);

}

/*----------------------------------------------------------------------------*/
INT iseg_hps_current_limit_set (ISEGHPS_INFO * info, WORD channel, float *pvalue)
{

  char cmd[255];

  sprintf(cmd,":CURR %f\r\n",(*pvalue)*1.e-6); //current in A

  return iseg_hps_write(info,cmd);

}

/*----------------------------------------------------------------------------*/
INT iseg_hps_chState_set (ISEGHPS_INFO * info, WORD channel, DWORD *pvalue)
{

  char cmd[255];
  if(*pvalue)
    sprintf(cmd,":VOLT ON\r\n");
  else
    sprintf(cmd,":VOLT OFF\r\n");

  return iseg_hps_write(info,cmd);

}
  
/*----------------------------------------------------------------------------*/
INT iseg_hps_chState_get (ISEGHPS_INFO * info, WORD channel, DWORD *pvalue)
{

  char ans[255];  
  char cmd[255] = ":READ:CHAN:STAT?\r\n";

  int ret = iseg_hps_read(info,cmd,ans);
  if(ret == FE_SUCCESS){
    int stat = atoi(ans);
    *pvalue = ((stat>>3) & 1);   
    return FE_SUCCESS;
  }

  return 0;

}

/*----------------------------------------------------------------------------*/

INT iseg_hps_chStatus_get (ISEGHPS_INFO * info, WORD channel, DWORD *pvalue)
{
 
  int ret;
  char ans[255];  
  char cmd[255] = ":READ:CHAN:STAT?\r\n";
  int mask = 0b1110001000100110;
  
  ret = iseg_hps_read(info,cmd,ans);
  if(ret == FE_SUCCESS){
    int stat = atoi(ans);
    *pvalue = (stat & mask);
    return FE_SUCCESS;
  }

  *pvalue = 1;
  return 0;

}

/*----------------------------------------------------------------------------*/

INT iseg_hps_temperature_get (ISEGHPS_INFO * info, WORD channel, float *pvalue)
{

  char ans[255];  
  char cmd[255] = ":READ:MOD:TEMP?\r\n";

  int ret = iseg_hps_read(info,cmd,ans);
  if(ret == FE_SUCCESS){
    char *val = strtok(ans,"C");
    *pvalue = atof(val);
    return FE_SUCCESS;
  }

  return 0;

}
    
/*----------------------------------------------------------------------------*/

INT iseg_hps_ramp_set (ISEGHPS_INFO * info, INT cmd, WORD channel, float *pvalue)
{

  char scmd[255];

  if (cmd == CMD_SET_RAMPUP)
    sprintf(scmd,":CONF:RAMP:VOLT %f\r\n",*pvalue); 
  else if (cmd == CMD_SET_RAMPDOWN)
    sprintf(scmd,":CONF:RAMP:VOLT %f\r\n",*pvalue); 

  return iseg_hps_write(info,scmd);

}

/*----------------------------------------------------------------------------*/
INT iseg_hps_ramp_get (ISEGHPS_INFO * info, INT cmd, WORD channel, float *pvalue)
{

  char ans[255];  
  char scmd[255];

  if (cmd == CMD_GET_RAMPUP)
    sprintf(scmd,":READ:RAMP:VOLT?\r\n"); 
  else if (cmd == CMD_GET_RAMPDOWN)
    sprintf(scmd,":READ:RAMP:VOLT?\r\n"); 

  int ret = iseg_hps_read(info,scmd,ans);
  if(ret == FE_SUCCESS){
    char *val = strtok(ans,"E");
    printf(".....%s\n",val);
    *pvalue = atof(val)*1.e3;
    return FE_SUCCESS;
  }

  return 0;

}

/*----------------------------------------------------------------------------*/
INT iseg_hps_voltage_limit_set (ISEGHPS_INFO * info, WORD channel, float *pvalue)
{

  char cmd[255];

  sprintf(cmd,":VOLT:LIM %f\r\n",*pvalue); //current in A

  return iseg_hps_write(info,cmd);

}

/*----------------------------------------------------------------------------*/
INT iseg_hps_voltage_limit_get (ISEGHPS_INFO * info, WORD channel, float *pvalue)
{

  char ans[255];  
  char cmd[255] = ":READ:VOLT:LIM?\r\n";

  int ret = iseg_hps_read(info,cmd,ans);

  if(ret == FE_SUCCESS){
    char *val = strtok(ans,"E");
    *pvalue = atof(val)*1.e3;
    return FE_SUCCESS;
  }

  return 0;

}

/*----------------------------------------------------------------------------*/
INT iseg_hps_trip_time_set (ISEGHPS_INFO * info, WORD channel, float *pvalue)
{

    return FE_SUCCESS;

}

/*----------------------------------------------------------------------------*/
INT iseg_hps_trip_time_get (ISEGHPS_INFO * info, WORD channel, float *pvalue)
{
  
  *pvalue = 0;

  return FE_SUCCESS;

}

/*---- device driver entry point -----------------------------------*/
INT iseg_hps (INT cmd, ...)
{
  va_list argptr;
  HNDLE hKey;
  WORD channel, status, icmd;
  DWORD flags;
  DWORD state, *pstate;
  char *label;
  float value, *pvalue;
  void *info;
  INT (*bd) (INT cmd, ...);

  va_start (argptr, cmd);
  status = FE_SUCCESS;

  switch (cmd)
  {
  case CMD_INIT:
    hKey = va_arg (argptr, HNDLE);
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg (argptr, INT);
    flags = va_arg (argptr, DWORD);
    bd =  va_arg (argptr, INT (*)(INT, ...));
    status = iseg_hps_init (hKey, (void **)info, channel, bd);
    printf("Status %d\n",status);
    break;

  case CMD_EXIT:
    info = va_arg (argptr, void *);
    status = iseg_hps_exit ((ISEGHPS_INFO *) info);
    break;

  case CMD_GET_LABEL:  // name
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg (argptr, INT);
    label = (char *) va_arg (argptr, char *);
    status = iseg_hps_Label_get ((ISEGHPS_INFO *) info, channel, label);
    break;

  case CMD_SET_LABEL:  // name
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg (argptr, INT);
    label = (char *) va_arg (argptr, char *);
    status = iseg_hps_Label_set ((ISEGHPS_INFO *) info, channel, label);
    break;

  case CMD_GET_DEMAND:  // set voltage read back
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg (argptr, INT);
    pvalue = va_arg (argptr, float *);
    status = iseg_hps_demand_get ((ISEGHPS_INFO *) info, channel, pvalue);
    break;

  case CMD_SET:  // voltage
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg (argptr, INT);
    value = (float) va_arg (argptr, double);	// floats are passed as double
    status = iseg_hps_set ((ISEGHPS_INFO *) info, channel, value);
    break;

  case CMD_GET:  //voltage
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg (argptr, INT);
    pvalue = va_arg(argptr, float *);
    status = iseg_hps_get ((ISEGHPS_INFO *) info, channel, pvalue);
    break;

  case CMD_GET_CURRENT:
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg (argptr, INT);
    pvalue = va_arg(argptr, float *);
    status = iseg_hps_current_get ((ISEGHPS_INFO *) info, channel, pvalue);
    break;

  case CMD_SET_CHSTATE:
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg(argptr, INT);
    state = (DWORD)va_arg(argptr, double);
    status = iseg_hps_chState_set ((ISEGHPS_INFO *) info, channel, &state);
    break;

  case CMD_GET_CHSTATE:
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg (argptr, INT);
    pstate = (DWORD *) va_arg (argptr, DWORD *);
    status = iseg_hps_chState_get ((ISEGHPS_INFO *) info, channel, pstate);
    break;

  case CMD_GET_STATUS:
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg (argptr, INT);
    pstate = (DWORD *) va_arg (argptr, DWORD *);
    status = iseg_hps_chStatus_get ((ISEGHPS_INFO *) info, channel, pstate);
    break;

  case CMD_GET_TEMPERATURE:
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg (argptr, INT);
    pvalue = (float *)va_arg(argptr, float *);
    status = iseg_hps_temperature_get ((ISEGHPS_INFO *) info, channel, pvalue);
    break;

  case CMD_SET_RAMPUP:
  case CMD_SET_RAMPDOWN:
    info = va_arg (argptr, void *);
    icmd = cmd;
    channel = (WORD) va_arg (argptr, INT);
    value = (float) va_arg(argptr, double);	// floats are passed as double
    status = iseg_hps_ramp_set ((ISEGHPS_INFO *) info, icmd, channel, &value);
    break;

  case CMD_GET_RAMPUP:
  case CMD_GET_RAMPDOWN:
    info = va_arg (argptr, void *);
    icmd = cmd;
    channel = (WORD) va_arg (argptr, INT);
    pvalue = va_arg (argptr, float *);
    status = iseg_hps_ramp_get ((ISEGHPS_INFO *) info, icmd, channel, pvalue);
    break;

  case CMD_SET_CURRENT_LIMIT:
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg(argptr, INT);
    value = (float)va_arg(argptr, double);	// floats are passed as double
    status = iseg_hps_current_limit_set ((ISEGHPS_INFO *)info, channel, &value);
    break;

  case CMD_GET_CURRENT_LIMIT:
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg (argptr, INT);
    pvalue = (float *) va_arg (argptr, float *);
    status = iseg_hps_current_limit_get ((ISEGHPS_INFO *)info, channel, pvalue);
    break;

  case CMD_SET_VOLTAGE_LIMIT:
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg (argptr, INT);
    value = (float) va_arg (argptr, double);
    status = iseg_hps_voltage_limit_set ((ISEGHPS_INFO *)info, channel, &value);
    break;

  case CMD_GET_VOLTAGE_LIMIT:
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg (argptr, INT);
    pvalue = va_arg (argptr, float *);
    status = iseg_hps_voltage_limit_get ((ISEGHPS_INFO *)info, channel, pvalue);
    break;

  case CMD_SET_TRIP_TIME:
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg (argptr, INT);
    value = (float) va_arg (argptr, double);
    status = iseg_hps_trip_time_set ((ISEGHPS_INFO *)info, channel, &value);
    break;

  case CMD_GET_TRIP_TIME:
    info = va_arg (argptr, void *);
    channel = (WORD) va_arg (argptr, INT);
    pvalue = va_arg (argptr, float *);
    status = iseg_hps_trip_time_get ((ISEGHPS_INFO *)info, channel, pvalue);
    break;

  default:
    break;
  }

  va_end (argptr);

  return status;
}

/*------------------------------------------------------------------*/

/* emacs
 * Local Variables:
 * mode:C
 * mode:font-lock
 * tab-width: 8
 * c-basic-offset: 2
 * End:
 */
