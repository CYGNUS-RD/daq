#include <complex>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>   
#include <ctype.h>
#include <string.h>
#include "VICPClient.h"
#include "lecroy_lib.h"
using namespace std;

#ifndef BOOL
#define BOOL int
#endif

#define MAX_S 256
#define FALSE 0
#define TRUE 1

float ETH_SCOPE_get(CVICPClient *dso_pntr){
float ret;
int i;
bool found=FALSE;
char instr[MAX_S];

	memset(instr,0,MAX_S);
	dso_pntr[0].readDataFromDevice(instr,MAX_S);
	//printf("Received:%s\n",instr);
	i=-1;
	do{
		i++;
		if(isdigit(instr[i])||instr[i]=='-'){
			found=TRUE;
			}
		if((found==TRUE)&&(i!=0)){
			if(instr[i-1]=='_'){
				found=FALSE;
				}
			if(instr[i-1]=='C'){
				found=FALSE;
				}
			if(instr[i-1]=='M'){
				found=FALSE;
				}
			if(instr[i-1]=='T'){
				found=FALSE;
				}
			}
		}while((found==FALSE)&&(i<MAX_S));

	if(i>=MAX_S){
		printf("eth_scope_main.cc:i>MAX_S apparently, returning 0\n");
		return 0;
		}
		//printf("this is what idigit got\n");
		//puts(instr+i);
	ret=atof(instr+i);
	return ret;
	}

void ETH_SCOPE_channel_str(char chan, char *channel){
	switch (chan) {
		case 'A' :
		case 'a' : strcpy(channel,"TA:");
			break;
		case 'B' :
		case 'b' : strcpy(channel,"TB:");
			break;
		case 'C' :
		case 'c' : strcpy(channel,"TC:");
			break;
		case 'D' :
		case 'd' : strcpy(channel,"TD:");
			break;
		case '1' : strcpy(channel,"C1:");
			break;
		case '2' : strcpy(channel,"C2:");
			break;
		case '3' : strcpy(channel,"C3:");
			break;
		case '4' : strcpy(channel,"C4:");
			break;
		default :
			cout<<"eth_scope_main:ETH_SCOPE_channel_str: unknown channel \""<<chan<<"\"\n";
			cout<<"returning channel \"Z\" \n";
			strcpy(channel,"TZ:");
			break;
	}
}

void ETH_SCOPE_ASET_FIND(CVICPClient *dso_pntr,char chnnl){
        char outstr[MAX_S];
	char outstr2[]="ASET FIND";
	
	ETH_SCOPE_channel_str(chnnl,outstr);
	strcat (outstr,outstr2);
        dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
        }

void ETH_SCOPE_set_no_averages(CVICPClient *dso_pntr,char chnnl,int source,int no){
	char channel[5];
	char outstr[MAX_S];
	char outstr2[]="%sDEF EQN,'AVGS(C%d)',SWEEPS,%d";
	char outstr3[]="TA:DEF?";
	ETH_SCOPE_channel_str(chnnl,channel);
	sprintf(outstr,outstr2,channel,source,no);
	cout<<outstr<<"\n";
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);

	/* (sds, 2/5/03: not sure why these two lines are here, still,
	    better leave them I reckon... */

	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr3),true);
	dso_pntr[0].readDataFromDevice(outstr,MAX_S);
	}

int ETH_SCOPE_get_wf_size(CVICPClient *dso_pntr){
	char outstr[]="MSIZ?";
	float ret;
	
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	ret=ETH_SCOPE_get(dso_pntr);
	return ret;
	}


float ETH_SCOPE_get_time_per_div(CVICPClient *dso_pntr){
	char outstr[]="TDIV?";
	float ret;
	
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	ret=ETH_SCOPE_get(dso_pntr);
	return ret;
	}

int ETH_SCOPE_get_no_points(CVICPClient *dso_pntr,char chnnl){
	char outstr[MAX_S];
	char outstr2[]="INSP? 'WAVE_ARRAY_1'";
	int ret;
        	
        ETH_SCOPE_channel_str(chnnl,outstr);
	strcat (outstr,outstr2);
        dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
        ret=(int)ETH_SCOPE_get(dso_pntr);  
	return ret;
	}

float ETH_SCOPE_get_vgain(CVICPClient *dso_pntr,char chnnl){
	char outstr[MAX_S];
	char outstr2[]="INSP? 'VERTICAL_GAIN'";
	float ret;
 
        ETH_SCOPE_channel_str(chnnl,outstr);
	strcat (outstr,outstr2);
        dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
        ret=ETH_SCOPE_get(dso_pntr);  
	return ret;
	}

float ETH_SCOPE_get_voffset(CVICPClient *dso_pntr,char chnnl){
	char outstr[MAX_S];
 	char outstr2[]="INSP? 'VERTICAL_OFFSET'";
	float ret;

        ETH_SCOPE_channel_str(chnnl,outstr);
	strcat (outstr,outstr2);
        dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
        ret=ETH_SCOPE_get(dso_pntr); 
	return ret; 
	}

float ETH_SCOPE_get_hinterval(CVICPClient *dso_pntr,char chnnl){
	char outstr[MAX_S];
 	char outstr2[]="INSP? 'HORIZ_INTERVAL'";
	float ret;
 
        ETH_SCOPE_channel_str(chnnl,outstr);
	strcat (outstr,outstr2);
        dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
        ret=ETH_SCOPE_get(dso_pntr);  
	return ret;
	}

float ETH_SCOPE_get_hoffset(CVICPClient *dso_pntr,char chnnl){
	char outstr[MAX_S];
	char outstr2[]="INSP? 'HORIZ_OFFSET'";
	float ret;
 
        ETH_SCOPE_channel_str(chnnl,outstr);
	strcat (outstr,outstr2);
        dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
        ret=ETH_SCOPE_get(dso_pntr);  
	return ret;
	}

float ETH_SCOPE_get_angle(CVICPClient *dso_pntr){
	float ret;
	int i;
	BOOL found=FALSE;
	char instr[MAX_S];
	char outstr[]="XYCV? HABS_ANGLE";
 
	for(i=0;i<MAX_S;i++){
		instr[i]='\0';
		}

        dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	dso_pntr[0].readDataFromDevice(instr,MAX_S);
	i=-1;
	do{
		i++;
		if(isdigit(instr[i])||instr[i]=='-'){
			found=TRUE;
			}
		if((found==TRUE)&&(i!=0)){
			if(instr[i-1]=='_'){
				found=FALSE;
				}
			}
		}while((found==FALSE)&&(i<MAX_S));

	if(i>=MAX_S){
		if(strstr(instr,"UN")!=NULL) return 1000;

		printf("eth_scope_main.cc:i>MAX_S apparently, returning 0\n");
		return 0;
		}
	ret=atof(instr+i);
	return ret;

	}

float ETH_SCOPE_get_x_value(CVICPClient *dso_pntr){
	float ret;
	int i;
	BOOL found=FALSE;
	char instr[MAX_S];
	char outstr[]="XYCV? HABS_X";
 
	for(i=0;i<MAX_S;i++){
		instr[i]='\0';
		}

        dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	dso_pntr[0].readDataFromDevice(instr,MAX_S);

	i=-1;
	do{
		i++;
		if(isdigit(instr[i])||instr[i]=='-'){
			found=TRUE;
			}
		if((found==TRUE)&&(i!=0)){
			if(instr[i-1]=='_'){
				found=FALSE;
				}
			}
		}while((found==FALSE)&&(i<MAX_S));

	if(i>=MAX_S){
		if(strstr(instr,"UN")!=NULL) return 1000;

		printf("eth_scope_main.cc:i>MAX_S apparently, returning 0\n");
		return 0;
		}
	ret=atof(instr+i);
	return ret;

	}

float ETH_SCOPE_get_y_value(CVICPClient *dso_pntr){
	float ret;
	int i;
	BOOL found=FALSE;
	char instr[MAX_S];
	char outstr[]="XYCV? HABS_Y";
 
	for(i=0;i<MAX_S;i++){
		instr[i]='\0';
		}

        dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	dso_pntr[0].readDataFromDevice(instr,MAX_S);
	i=-1;
	do{
		i++;
		if(isdigit(instr[i])||instr[i]=='-'){
			found=TRUE;
			}
		if((found==TRUE)&&(i!=0)){
			if(instr[i-1]=='_'){
				found=FALSE;
				}
			}
		}while((found==FALSE)&&(i<MAX_S));

	if(i>=MAX_S){
		if(strstr(instr,"UN")!=NULL) return 1000;

		printf("eth_scope_main.cc:i>MAX_S apparently, returning 0\n");
		return 0;
		}
	ret=atof(instr+i);
	return ret;

	}

int ETH_SCOPE_set_wf_size(CVICPClient *dso_pntr,int points){
  	char outstr[MAX_S];
	int ret;
	
	sprintf(outstr,"MSIZ %d",points);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	ret=ETH_SCOPE_get_wf_size(dso_pntr);
	return ret;
}

float ETH_SCOPE_set_time_per_div(CVICPClient *dso_pntr,float time){
	// This sets the timebase and also returns
	// the actual value used by the scope as this may be altered
	// the value of the delay is in seconds (although this may fuck up
	// depending upon the scope and may have to use ms us and ns post fixes)
	char outstr[MAX_S];
	float ret;
	
	sprintf(outstr,"TDIV %e",time);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	ret=ETH_SCOPE_get_time_per_div(dso_pntr);
	return ret;
	}

float ETH_SCOPE_get_trigger_delay(CVICPClient *dso_pntr){
	char outstr[]="TRDL?";
	float ret;
	
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	ret=ETH_SCOPE_get(dso_pntr);
	return ret;
	}

int ETH_SCOPE_get_TRIG_MODE(CVICPClient *dso_pntr){
	char outstr[]="TRMD?";
	char instr[MAX_S];
	
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
        dso_pntr[0].readDataFromDevice(instr,MAX_S);
	printf("string is %s\n", instr);

	if(!strcmp(instr,"TRMD SINGLE")) return 1;
	return 0;

	}

float ETH_SCOPE_set_trigger_delay(CVICPClient *dso_pntr,float delay){
	// This sets the trigger delay and also returns
	// the actual value used by the scope as this may be altered
	// the value of the delay is in seconds *SEE BELOW* (although this may fuck up
	// depending upon the scope and may have to use ms us and ns post fixes)

	// Note added by Steve 30.6.98:
	// even if you send a float, the scope only accepts an integer, and so
	// the NS fix has been added: hence, send this function the required delay
	// in nanoseconds.

	char outstr[MAX_S];
	float ret;
	
	sprintf(outstr,"TRDL -%fNS",delay);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	ret=(float)ETH_SCOPE_get_trigger_delay(dso_pntr);
	return ret;
	}

float ETH_SCOPE_get_pk(CVICPClient *dso_pntr,char chnnl){
	char outstr[MAX_S];
	char outstr2[]="PAVA? PKPK";
	float ret;
	
	ETH_SCOPE_channel_str(chnnl,outstr);
	strcat(outstr,outstr2);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	ret=(float)ETH_SCOPE_get(dso_pntr);
	return ret;
	}

float ETH_SCOPE_get_rms(CVICPClient *dso_pntr,char chnnl){
	char outstr[MAX_S];
	char outstr2[]="PAVA? RMS";
	float ret;
	
	ETH_SCOPE_channel_str(chnnl,outstr);
	strcat(outstr,outstr2);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	ret=(float)ETH_SCOPE_get(dso_pntr);
	return ret;
	}

float ETH_SCOPE_get_freq(CVICPClient *dso_pntr,char chnnl){
	char outstr[MAX_S];
	char outstr2[]="PAVA? FREQ";
	float ret;
	
	ETH_SCOPE_channel_str(chnnl,outstr);
	strcat(outstr,outstr2);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	ret=(float)ETH_SCOPE_get(dso_pntr);
	return ret;
	}

float ETH_SCOPE_get_average(CVICPClient *dso_pntr,char chnnl){
	char outstr[MAX_S];
	char outstr2[]="PAVA? MEAN";
	float ret;
	
	ETH_SCOPE_channel_str(chnnl,outstr);
	strcat(outstr,outstr2);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	ret=(float)ETH_SCOPE_get(dso_pntr);
	return ret;
	}

float ETH_SCOPE_get_ATTENUATION(CVICPClient *dso_pntr,char chnnl){
	char outstr[MAX_S];
	char outstr2[]="ATTN?";
	float ret;
	
	ETH_SCOPE_channel_str(chnnl,outstr);
	strcat(outstr,outstr2);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	ret=(float)ETH_SCOPE_get(dso_pntr);
	return ret;
	}

float ETH_SCOPE_get_volts_per_div(CVICPClient *dso_pntr,char chnnl){
	char outstr[MAX_S];
	char outstr2[]="Volt_DIV?";
	float ret;
	
	ETH_SCOPE_channel_str(chnnl,outstr);
	sprintf(outstr,outstr2);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	ret=(float)ETH_SCOPE_get(dso_pntr);
	return ret;
	}

void ETH_SCOPE_clear_sweeps(CVICPClient *dso_pntr){
	char outstr1[]="*CLS";
	char outstr2[]="CLSW";
	// clear the status registers...
	dso_pntr[0].sendDataToDevice(outstr1,strlen(outstr1),true);
	//clear the sweeps
	dso_pntr[0].sendDataToDevice(outstr2,strlen(outstr2),true);
	}

void ETH_SCOPE_set_TRIG_delay(CVICPClient *dso_pntr,float delay){
	char outstr[MAX_S];
	sprintf(outstr,"TRDL %f",delay);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

void ETH_SCOPE_set_TRIG_LEVEL(CVICPClient *dso_pntr,char chnnl,float level){
	char outstr[MAX_S];

	ETH_SCOPE_channel_str(chnnl,outstr);
	sprintf(outstr+strlen(outstr),"TRLV %f",level);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

void ETH_SCOPE_set_voffset(CVICPClient *dso_pntr,char chnnl,float level){
	char outstr[MAX_S];

	ETH_SCOPE_channel_str(chnnl,outstr);
	sprintf(outstr+strlen(outstr),"OFST %f",level);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

void ETH_SCOPE_set_volts_per_div(CVICPClient *dso_pntr,char chnnl,float level){
	char outstr[MAX_S];

	ETH_SCOPE_channel_str(chnnl,outstr);
	sprintf(outstr+strlen(outstr),"VDIV %f",level);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

void ETH_SCOPE_set_WAIT(CVICPClient *dso_pntr,float timeout){
	char outstr[MAX_S];
	sprintf(outstr,"WAIT %f",timeout);
	//printf("holding on ETH_SCOPE_set_WAIT\n");
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

void ETH_SCOPE_set_DISPLAY(CVICPClient *dso_pntr,char *mode){
	char outstr[MAX_S];
	sprintf(outstr,"DISP %s",mode);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

void ETH_SCOPE_set_AUTO_CALIBRATE(CVICPClient *dso_pntr,char *mode){
	char outstr[MAX_S];
	sprintf(outstr,"ACAL %s",mode);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

void ETH_SCOPE_set_TRIG_MODE(CVICPClient *dso_pntr,char *mode){
	char outstr[MAX_S];
	sprintf(outstr,"TRMD %s",mode);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

void ETH_SCOPE_set_TRIG_TRG(CVICPClient *dso_pntr){
	char outstr[]="*TRG";
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

void ETH_SCOPE_set_TRIG_AC_COUPLING(CVICPClient *dso_pntr,char chnnl){
	char outstr[MAX_S];
	char outstr2[]="TRCP AC";
	
	ETH_SCOPE_channel_str(chnnl,outstr);
	strcat(outstr,outstr2);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

void ETH_SCOPE_set_TRIG_DC_COUPLING(CVICPClient *dso_pntr,char chnnl){
	char outstr[MAX_S];
	char outstr2[]="TRCP DC";
	
	ETH_SCOPE_channel_str(chnnl,outstr);
	strcat(outstr,outstr2);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

void ETH_SCOPE_set_AC1M_COUPLING(CVICPClient *dso_pntr,char chnnl){
	char outstr[MAX_S];
	char outstr2[]="CPL A1M";
	
	ETH_SCOPE_channel_str(chnnl,outstr);
	strcat(outstr,outstr2);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

void ETH_SCOPE_set_DC1M_COUPLING(CVICPClient *dso_pntr,char chnnl){
	char outstr[MAX_S];
	char outstr2[]="CPL D1M";
	
	ETH_SCOPE_channel_str(chnnl,outstr);
	strcat(outstr,outstr2);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

void ETH_SCOPE_set_DC50M_COUPLING(CVICPClient *dso_pntr,char chnnl){
	char outstr[MAX_S];
	char outstr2[]="CPL D50";
	
	ETH_SCOPE_channel_str(chnnl,outstr);
	strcat(outstr,outstr2);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

void ETH_SCOPE_set_ARM(CVICPClient *dso_pntr){
	char outstr[]="ARM";
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

BOOL ETH_SCOPE_average_finished(CVICPClient *dso_pntr,char channel){
	int mask=0,temp;
	char outstr[]="INR?";
	channel=toupper(channel);
	switch(channel){
		case 'A':mask=256;break;
		case 'B':mask=512;break;
		case 'C':mask=1024;break;
		case 'D':mask=2048;break;
		default:
		printf("ERROR: average finished, unknown channel %c\n",channel);
		mask=0;
		}
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
//	usleep(1000);
	temp=(int)ETH_SCOPE_get(dso_pntr);
//	printf("mask:%d     received:%d    mask&received:%d\n",mask,outstr,mask&outstr);
	if((mask&temp)==0){
		return FALSE;
		}
	else{
		return TRUE;
		}
	}

BOOL ETH_SCOPE_all_average_finished(CVICPClient *dso_pntr){
	int mask=2048+1024+512+256;// set for all the channels
	char outstr[]="INR?";
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
        if((mask&(int)ETH_SCOPE_get(dso_pntr))!=mask){
                return FALSE;
                }
        else{
                return TRUE;
                }
                 return TRUE;
       }               

BOOL ETH_SCOPE_poll(CVICPClient *dso_pntr){
        int mask=1;// set for all the channels
	char outstr[]="INR?";
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
        if((mask&(int)ETH_SCOPE_get(dso_pntr))!=mask){
	  return FALSE;
	}
	return TRUE;
}               

BOOL ETH_SCOPE_all_average_finished(CVICPClient *dso_pntr, BOOL a,BOOL b,BOOL c,BOOL d){
	int mask=0;
	int i;
	int test=0;
	int temp=0;
	int oldtemp=0;
	char outstr[]="INR?";

	//set for those channels set to TRUE
	mask=256*((a==TRUE)?1:0)+512*((b==TRUE)?1:0)+1024*((c==TRUE)?1:0)+2048*((d==TRUE)?1:0);

	while(test==0)
	{
                i=0;
	        dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
		temp=(int)ETH_SCOPE_get(dso_pntr);
                oldtemp=oldtemp|temp;
                if((oldtemp&mask)==mask){
		  test=1;
		  return TRUE;}
                else {
		test=0;
		return FALSE;}
	}
                return TRUE;
}  

int ETH_SCOPE_get_stat(CVICPClient *dso_pntr){
	char outstr[]="INR?";
	int ret;
	
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	ret=(int)ETH_SCOPE_get(dso_pntr);
	return ret;
	}

void ETH_SCOPE_set_8bit_transfer(CVICPClient *dso_pntr){
	char outstr[]="CFMT DEF9,BYTE,BIN";
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

void SER_SCOPE_set_8bit_transfer(CVICPClient *dso_pntr){
	char instr[256];
	char outstr[]="CFMT DEF9,BYTE,HEX";
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	dso_pntr[0].readDataFromDevice(instr,255);
	}

void ETH_SCOPE_set_16bit_transfer(CVICPClient *dso_pntr){
	char outstr[]="CFMT DEF9,WORD,BIN";
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	}

void SER_SCOPE_set_16bit_transfer(CVICPClient *dso_pntr){
	char instr[256];
	char outstr[]="CFMT DEF9,WORD,HEX";
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	dso_pntr[0].readDataFromDevice(instr,255);
	}

/* THIS FN IS OVERLOADED, BE SURE TO DUPLICATE CHANGES IN THE OTHER ONE */
void ETH_SCOPE_get_wave(CVICPClient *dso_pntr, char chnnl, char * data){
	char outstr[MAX_S];
	char outstr2[]="WF? DAT1";
	char *waveform;
	int no_points;
	int necessary_buffer_size;

	no_points=ETH_SCOPE_get_no_points(dso_pntr,chnnl);
	necessary_buffer_size=no_points+86;

// "86!?!?! Where does that random number come from?" I hear you cry! well....
// What actually happens is this. You send the scope a request for the waveform data,
// which is something along the lines of C1:WF? DAT1
// The scope responds in three chunks (not including the TCP header consisting of:

// First chunk:    C1:WF DAT1,#9000000404
// i.e. response to question, a #9, then the number of points in the waveform
// so, we're up to 22 bytes so far.

// Second chunk:   <the data> which will be (in this case) 404 bytes long (no_points)

// Third "chunk":  one byte of data. Don't know what it is, but you have to read it,
// otherwise the scope's response now lags by one after each question.
// "But why 86? Why not 23?" I hear you cry. Well...
// You have to read a minimum of LECROY_TCP_MINIMUM_PACKET_SIZE bytes, where
// LECROY_TCP_MINIMUM_PACKET_SIZE is #define'd in lecroy_tcp.h 22+64=86.

// So, given all that palava, I've decided to create a new array each time this
// function is called, so that you don't have to rememeber to save the waveform
// starting from the 23rd received byte. Hence the memcpy stuff at the end.
//                                --- Steve 6/5/03

	waveform=new char[necessary_buffer_size];

	ETH_SCOPE_channel_str(chnnl,outstr);
	strcat(outstr,outstr2);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);

	if(dso_pntr[0].readDataFromDevice(waveform,necessary_buffer_size)<0){
		printf("ETH_SCOPE_get_wave:receive error,\n");
		printf(" need ip address to reset trying again, bombing out early\n");
		}

	memcpy(data,waveform+22,no_points);
	delete[] waveform;

	}


/* OVERLOADED FN: this one also accepts the "no_points" value, since it is
   possible you already know this already (for making your array "data" big enough)
   It saves on asking the scope how many points it's going to send, that's all,
   but if the value is wrong, then the data may not be read properly. */

void ETH_SCOPE_get_wave(CVICPClient *dso_pntr, char chnnl, char * data, int no_points){

  char outstr[MAX_S];
  char outstr2[]="WF? DAT1";
  char *waveform;
  int necessary_buffer_size=no_points+86;
  
// "86!?!?! Where does that random number come from?" I hear you cry! see overloaded fn above
  
  waveform=new char[necessary_buffer_size];
  
  ETH_SCOPE_channel_str(chnnl,outstr);
  strcat(outstr,outstr2);
  dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
  if(dso_pntr[0].readDataFromDevice(waveform,necessary_buffer_size)<0){
    printf("ETH_SCOPE_get_wave:receive error,\n");
		printf(" need ip address to reset trying again, bombing out early\n");
  }
  
  memcpy(data,waveform+22,no_points);
  //printf("here data is %d,%d\n",data[0],waveform[22]);
  delete[] waveform;

}

void ETH_SCOPE_get_wave_12bit(CVICPClient *dso_pntr, char chnnl, unsigned int *data, int no_points){

  char *data_low = new char[no_points];
  char *data_high = new char[no_points];

  ETH_SCOPE_get_wave(dso_pntr,chnnl,data_low,no_points);

  printf("--- data_low %d\n",(int)(*data_low)+127);

  char outstr[MAX_S];
  char outstr2[]="WF? DAT2";
  char *waveform;
  int necessary_buffer_size=no_points+86;
  
// "86!?!?! Where does that random number come from?" I hear you cry! see overloaded fn above
  
  waveform=new char[necessary_buffer_size];
  
  ETH_SCOPE_channel_str(chnnl,outstr);
  strcat(outstr,outstr2);
  dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
  if(dso_pntr[0].readDataFromDevice(waveform,necessary_buffer_size)<0){
    printf("ETH_SCOPE_get_wave:receive error,\n");
		printf(" need ip address to reset trying again, bombing out early\n");
  }
  
  memcpy(data_high,waveform+22,no_points);

  printf("###### data_high %d\n",(int)(*data_high)+127);

  *data = 0;
  *data |= (unsigned int)(*data_low);
  *data |= (unsigned int)(*data_high) << 8;

  //printf("here data is %d,%d\n",data[0],waveform[22]);
  delete[] waveform;
  delete[] data_low;
  delete[] data_high;

}

void SER_SCOPE_get_wave(CVICPClient *dso_pntr, char chnnl, char *data, int no_points){
	char outstr[MAX_S];
	char outstr2[]="WF? DAT1";
	char *waveform;
	unsigned char *temp;
	int i,j;

	int necessary_buffer_size=2*no_points+26+1;

	temp=new unsigned char[necessary_buffer_size];
	waveform=new char[necessary_buffer_size];
	/*if((outstr=(unsigned char*)calloc(necessary_buffer_size,sizeof(unsigned char)))==NULL){
 	  printf("Memory allocation for outstr unsuccessful\n");
  	  exit(1);}
 	if((waveform=(char*)calloc(necessary_buffer_size,sizeof(char)))==NULL){
 	  printf("Memory allocation for waveform unsuccessful\n");
 	  exit(1);}*/

	ETH_SCOPE_channel_str(chnnl,outstr);
	strcat(outstr,outstr2);
	dso_pntr[0].sendDataToDevice(outstr,strlen(outstr),true);
	if(dso_pntr[0].readDataFromDevice(waveform,necessary_buffer_size)<0){
		printf("ETH_SCOPE_get_wave:receive error,\n");
		printf(" need ip address to reset trying again, bombing out early\n");
		}

	memmove(temp,waveform+26,2*no_points);
	   for(i=0;i<no_points*2;i++){
	       if ((temp[i] >= '0') && (temp[i] <= '9')) temp[i]=temp[i]-'0';
	       if ((temp[i] >= 'a') && (temp[i] <= 'f')) temp[i]=temp[i]-'a'+10;
	       if ((temp[i] >= 'A') && (temp[i] <= 'F')) temp[i]=temp[i]-'A'+10;
	     }
		j=0;
		for(i=0;i<2*no_points;i+=2) {
			if(temp[i+1] <= 127) {
				data[j]  = (16*temp[i] + (temp[i+1] )); 
				
				}
			else  {
				temp[i]=(~temp[i])+1;
				temp[i+1]=~temp[i+1];
				data[j] = -(16*temp[i] + (temp[i+1] ));  
				
				}
			j++;
			}
	     
	delete[] temp;
	delete[] waveform;
	}


complex<float> ETH_SCOPE_fast_ft_trace(CVICPClient *dso_pntr,char chnnl,float frequency,int resolution,long no_points,float t_div,float vertical_gain){
complex<float>	complex_amplitude;
	char	*charbuffer;
	short	*shortbuffer;
	double  p,w;
	int 	i;

//	no_points=ETH_SCOPE_get_no_points(dso_pntr,chnnl);
//	t_div=ETH_SCOPE_get_hinterval(dso_pntr,chnnl);
//	vertical_gain=ETH_SCOPE_get_vgain(dso_pntr,chnnl);

	charbuffer=new char[no_points];
	if(charbuffer==NULL){
		cout<<"unable to allocate buffer memory of "<<no_points<<" bytes\n";
		return complex<float>(0,0);
		}

		shortbuffer=new short[no_points/2];
		if(shortbuffer==NULL){
			cout<<"ETH_SCOPE_ft_trace: unable to allocate short buffer memory of "<<no_points<<" bytes\n";
			return complex<float>(0,0);
			}
	ETH_SCOPE_get_wave(dso_pntr, chnnl, charbuffer, no_points);

	if (resolution==2) {
		memcpy(shortbuffer,charbuffer,no_points);
		}

	w=2*M_PI*frequency*t_div;
	complex_amplitude=complex<float>(0.0,0.0);

	if(resolution==1){
		for(i=0,p=0;i<no_points;i++,p+=w){
			complex_amplitude+=complex<float>(cos(p)*(float)charbuffer[i],sin(p)*(float)charbuffer[i]);
			}
		}
	else{
		for(i=0,p=0;i<(no_points/2);i++,p+=w){
			complex_amplitude+=complex<float>(cos(p)*(float)shortbuffer[i],sin(p)*(float)shortbuffer[i]);
			}
		}
	complex_amplitude=complex_amplitude*(float)vertical_gain/(float)(no_points/resolution);
	delete[] charbuffer;
	if(resolution==2) delete[] shortbuffer;
	return complex_amplitude;
	}



complex<float> ETH_SCOPE_ft_trace(CVICPClient *dso_pntr,char chnnl,float frequency,int resolution){
complex<float>	complex_amplitude;
	float	t_div,vertical_gain;
	char	*charbuffer;
	short	*shortbuffer;
	long	no_points;
	double  p,w;
	int 	i;

	no_points=ETH_SCOPE_get_no_points(dso_pntr,chnnl);
	t_div=ETH_SCOPE_get_hinterval(dso_pntr,chnnl);
	vertical_gain=ETH_SCOPE_get_vgain(dso_pntr,chnnl);

	charbuffer=new char[no_points];
	if(charbuffer==NULL){
		cout<<"unable to allocate buffer memory of "<<no_points<<" bytes\n";
		return complex<float>(0,0);
		}

		shortbuffer=new short[no_points/2];
		if(shortbuffer==NULL){
			cout<<"ETH_SCOPE_ft_trace: unable to allocate short buffer memory of "<<no_points<<" bytes\n";
			return complex<float>(0,0);
			}
	ETH_SCOPE_get_wave(dso_pntr, chnnl, charbuffer, no_points);

	if (resolution==2) {
		memcpy(shortbuffer,charbuffer,no_points);
		}

	w=2*M_PI*frequency*t_div;
	complex_amplitude=complex<float>(0.0,0.0);

	if(resolution==1){
		for(i=0,p=0;i<no_points;i++,p+=w){
			complex_amplitude+=complex<float>(cos(p)*(float)charbuffer[i],sin(p)*(float)charbuffer[i]);
			}
		}
	else{
		for(i=0,p=0;i<(no_points/2);i++,p+=w){
			complex_amplitude+=complex<float>(cos(p)*(float)shortbuffer[i],sin(p)*(float)shortbuffer[i]);
			}
		}
	complex_amplitude=complex_amplitude*(float)vertical_gain/(float)(no_points/resolution);
	delete[] charbuffer;
	if(resolution==2) delete[] shortbuffer;
	return complex_amplitude;
	}



void ETH_SCOPE_fts_trace(CVICPClient *dso_pntr,char chnnl,float *frequency,complex <float>* amps,int N,int resolution){
complex<float>  complex_amplitude;
	float   t_div,vertical_gain;
	char    *charbuffer;
	short	*shortbuffer;
	long    no_points;
	double  p,w;
	int     i,j;
 
	no_points=ETH_SCOPE_get_no_points(dso_pntr,chnnl);
	t_div=ETH_SCOPE_get_hinterval(dso_pntr,chnnl);
	vertical_gain=ETH_SCOPE_get_vgain(dso_pntr,chnnl);

	charbuffer=new char[no_points];
	if(charbuffer==NULL){
		cout<<"ETH_SCOPE_fts_trace:unable to allocate charbuffer memory of "<<no_points<<" bytes\n";
		return;
		}
		shortbuffer=new short[no_points/2];
		if(shortbuffer==NULL){
			cout<<"ETH_SCOPE_fts_trace: unable to allocate shortbuffer memory of "<<no_points<<" bytes\n";
			return;
			}

	ETH_SCOPE_get_wave(dso_pntr, chnnl, charbuffer, no_points);

	if (resolution==2) {
		memcpy(shortbuffer,charbuffer,no_points);
		}

	if(resolution==1){
		for(j=0;j<N;j++){	
	        	w=2*M_PI*frequency[j]*t_div;
	        	complex_amplitude=complex<float>(0.0,0.0);
	        	for(i=0,p=0;i<no_points;i++,p+=w){
	                	complex_amplitude+=complex<float>(cos(p)*(float)charbuffer[i],sin(p)*(float)charbuffer[i]);
	                	}
	        	amps[j]==complex_amplitude*(float)vertical_gain/(float)(no_points/resolution);
			}
		}
	else{
		for(j=0;j<N;j++){	
	        	w=2*M_PI*frequency[j]*t_div;
	        	complex_amplitude=complex<float>(0.0,0.0);
	        	for(i=0,p=0;i<(no_points/2);i++,p+=w){
	                	complex_amplitude+=complex<float>(cos(p)*(float)shortbuffer[i],sin(p)*(float)shortbuffer[i]);
	                	}
	        	amps[j]==complex_amplitude*(float)vertical_gain/(float)no_points;
			}
		}

	delete[] charbuffer;
	if(resolution==2) delete[] shortbuffer;
        }  
