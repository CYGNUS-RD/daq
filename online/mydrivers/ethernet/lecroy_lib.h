#ifndef __LECROY_LIB__
#define __LECROY_LIB__

#undef max
#undef min

#include <complex>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>   
#include <ctype.h>
#include <string.h>
using namespace std;
#ifndef BOOL
#define BOOL int
#endif

//#ifdef __cplusplus
//extern "C" {
//#endif

// function that take some sort of measurement
float	ETH_SCOPE_get_pk(CVICPClient *dso_pntr,char);
float	ETH_SCOPE_get_rms(CVICPClient *dso_pntr,char);
float	ETH_SCOPE_get_freq(CVICPClient *dso_pntr,char);
float	ETH_SCOPE_get_average(CVICPClient *dso_pntr,char);
float   ETH_SCOPE_get_ATTENUATION(CVICPClient *dso_pntr,char chnnl);
float	ETH_SCOPE_get_volts_per_div(CVICPClient *dso_pntr,char);
complex<float> ETH_SCOPE_ft_trace(CVICPClient *dso_pntr,char,float,int);
complex<float> ETH_SCOPE_fast_ft_trace(CVICPClient *dso_pntr,char,float,int,long,float,float);
void 	ETH_SCOPE_fts_trace(CVICPClient *dso_pntr,char chnnl,float *,complex <float>* ,int,int);  

// functions that set some scope property
void    ETH_SCOPE_ASET_FIND(CVICPClient *dso_pntr,char chnnl);
void 	ETH_SCOPE_set_TRIG_delay(CVICPClient *dso_pntr,float);
void    ETH_SCOPE_set_DISPLAY(CVICPClient *dso_pntr,char *mode);
void    ETH_SCOPE_set_AUTO_CALIBRATE(CVICPClient *dso_pntr,char *mode);
void 	ETH_SCOPE_set_WAIT(CVICPClient *dso_pntr,float);
float	ETH_SCOPE_set_trigger_delay(CVICPClient *dso_pntr,float);
void    ETH_SCOPE_set_TRIG_MODE(CVICPClient *dso_pntr,char *mode);
void    ETH_SCOPE_set_TRIG_TRG(CVICPClient *dso_pntr);
void    ETH_SCOPE_set_ARM(CVICPClient *dso_pntr);
void	ETH_SCOPE_set_no_averages(CVICPClient *dso_pntr,char,int,int);
float	ETH_SCOPE_set_time_per_div(CVICPClient *dso_pntr,float);
float	ETH_SCOPE_set_volts_per_div(CVICPClient *dso_pntr,float);
void	ETH_SCOPE_set_8bit_transfer(CVICPClient *dso_pntr);
void	ETH_SCOPE_set_16bit_transfer(CVICPClient *dso_pntr);
void	SER_SCOPE_set_8bit_transfer(CVICPClient *dso_pntr);
void	SER_SCOPE_set_16bit_transfer(CVICPClient *dso_pntr);
void 	ETH_SCOPE_set_TRIG_AC_COUPLING(CVICPClient *dso_pntr,char chnnl);
void 	ETH_SCOPE_set_TRIG_DC_COUPLING(CVICPClient *dso_pntr,char chnnl);
void 	ETH_SCOPE_set_AC1M_COUPLING(CVICPClient *dso_pntr,char chnnl);
void 	ETH_SCOPE_set_DC1M_COUPLING(CVICPClient *dso_pntr,char chnnl);
void 	ETH_SCOPE_set_DC50_COUPLING(CVICPClient *dso_pntr,char chnnl);
void 	ETH_SCOPE_set_TRIG_LEVEL(CVICPClient *dso_pntr,char chnnl,float level);
void 	ETH_SCOPE_set_voffset(CVICPClient *dso_pntr,char chnnl,float level);
void 	ETH_SCOPE_set_volts_per_div(CVICPClient *dso_pntr,char chnnl,float level);
int     ETH_SCOPE_set_wf_size(CVICPClient *dso_pntr,int points);

// functions that return some sort of property
int 	ETH_SCOPE_get_no_points(CVICPClient *dso_pntr,char); 
float	ETH_SCOPE_get_time_per_div(CVICPClient *dso_pntr);
float	ETH_SCOPE_get_trigger_delay(CVICPClient *dso_pntr);
int    ETH_SCOPE_get_TRIG_MODE(CVICPClient *dso_pntr);
void	SER_SCOPE_get_wave(CVICPClient *dso_pntr,char,char *,int);
void	ETH_SCOPE_get_wave(CVICPClient *dso_pntr,char,char *,int);
void	ETH_SCOPE_get_wave_12bit(CVICPClient *dso_pntr,char,unsigned int *,int);
void	ETH_SCOPE_get_wave(CVICPClient *dso_pntr,char,char *);
float	ETH_SCOPE_get_vgain(CVICPClient *dso_pntr,char);
float	ETH_SCOPE_get_voffset(CVICPClient *dso_pntr,char);
float	ETH_SCOPE_get_hinterval(CVICPClient *dso_pntr,char);
float	ETH_SCOPE_get_hoffset(CVICPClient *dso_pntr,char);
float	ETH_SCOPE_get_angle(CVICPClient *dso_pntr);
float	ETH_SCOPE_get_x_value(CVICPClient *dso_pntr);
float	ETH_SCOPE_get_y_value(CVICPClient *dso_pntr);
int    ETH_SCOPE_poll(CVICPClient *dso_pntr);

// functions that send/receive messages
float	ETH_SCOPE_get(CVICPClient *dso_pntr);

// open / close the scope into known states
void	ETH_SCOPE_close(CVICPClient *dso_pntr);


// misc. functions
void 	ETH_SCOPE_clear_sweeps(CVICPClient *dso_pntr);
void 	ETH_SCOPE_wait(CVICPClient *dso_pntr);
int	ETH_SCOPE_average_finished(CVICPClient *dso_pntr,char);
int	ETH_SCOPE_all_average_finished(CVICPClient *dso_pntr);
int	ETH_SCOPE_all_average_finished(CVICPClient *dso_pntr,BOOL,BOOL,BOOL,BOOL);
int 	ETH_SCOPE_get_stat(CVICPClient *dso_pntr);  
void	ETH_SCOPE_channel_str(char,char *);

  //#ifdef __cplusplus
  //}
  //#endif

#endif
