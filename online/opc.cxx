/********************************************************************\

  Name:         opc.cxx
  Created by:   Francesco Renga

  Contents:     Device Driver for generic OPC server

  $Id: mscbdev.c 3428 2006-12-06 08:49:38Z ritt $

\********************************************************************/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <regex>
#include "midas.h"
#include "msystem.h"
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_subscriptions.h>
#include <open62541/plugin/log_stdout.h>


#ifndef _MSC_VER
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#endif

#include <vector>

using namespace std;

/*---- globals -----------------------------------------------------*/

typedef struct {
  char Name[32];                // System Name (duplication)
  char ip[32];                  // IP# for network access
  int nsIndex;
  char TagsGuid[64];
} OPC_SETTINGS;

#define OPC_SETTINGS_STR "\
System Name = STRING : [32] gassys\n\
IP = STRING : [32] 192.168.0.105:4870\n\
Namespace Index = INT : 3\n\
Tags Guid = STRING : [64] ecef81d5-c834-4379-ab79-c8fa3133a311\n\
"

typedef struct {
  OPC_SETTINGS opc_settings;
  UA_Client* client;
  int num_channels;
  vector<string> channel_name;
  vector<int> channel_type;
  vector<int> channel_size;  
} OPC_INFO;

/*---- device driver routines --------------------------------------*/

INT opc_init(HNDLE hKey, void **pinfo, INT channels, INT(*bd) (INT cmd, ...))
{
  
   int status, size;
   OPC_INFO *info;
   HNDLE hDB;
   
   /* allocate info structure */
   info = (OPC_INFO *)calloc(1, sizeof(OPC_INFO));
   *pinfo = info;
   
   cm_get_experiment_database(&hDB, NULL);

   /* create PSI_BEAMBLOCKER settings record */
   status = db_create_record(hDB, hKey, "", OPC_SETTINGS_STR);
   if (status != DB_SUCCESS)
      return FE_ERR_ODB;

   size = sizeof(info->opc_settings);
   db_get_record(hDB, hKey, &info->opc_settings, &size, 0);

   info->client = UA_Client_new();
   UA_ClientConfig_setDefault(UA_Client_getConfig(info->client));

   char address[32];
   sprintf(address,"opc.tcp://%s",info->opc_settings.ip);
   printf("Connecting to %s...\n",address);
   UA_StatusCode retval = UA_Client_connect(info->client, address);
   if(retval != UA_STATUSCODE_GOOD) {
     UA_Client_delete(info->client);
     return FE_ERR_HW;
   }

   ////Scan of variables
   UA_BrowseRequest bReq;
   UA_BrowseRequest_init(&bReq);
   bReq.requestedMaxReferencesPerNode = 0;
   bReq.nodesToBrowse = UA_BrowseDescription_new();
   bReq.nodesToBrowseSize = 1;
   UA_Guid guid1;
   UA_Guid_parse(&guid1, UA_STRING(info->opc_settings.TagsGuid));
   bReq.nodesToBrowse[0].nodeId = UA_NODEID_GUID(info->opc_settings.nsIndex, guid1);
   bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; /* return everything */
   
   UA_BrowseResponse bResp = UA_Client_Service_browse(info->client, bReq);

   printf("---- OPC variables ---- %d\n",bResp.resultsSize);
   
   for(size_t i = 0; i < bResp.resultsSize; ++i) {

     for(size_t j = 0; j < bResp.results[i].referencesSize; ++j) {

       UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
       if ((ref->nodeClass == UA_NODECLASS_OBJECT || ref->nodeClass == UA_NODECLASS_VARIABLE||ref->nodeClass == UA_NODECLASS_METHOD)) {

	 if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {

	   UA_NodeId dataType;

	   char name[256];
	   strcpy(name,ref->nodeId.nodeId.identifier.string.data);
	   
	   UA_StatusCode retval = UA_Client_readDataTypeAttribute(info->client, UA_NODEID_STRING(info->opc_settings.nsIndex, name), &dataType);

	   if (retval != UA_STATUSCODE_GOOD){ // Try to remove the last character, that sometimes is wrongly added

	     char newname[256];
	     strcpy(newname,name);
	     newname[strlen(name)-1] = '\0';	     
	     retval = UA_Client_readDataTypeAttribute(info->client, UA_NODEID_STRING(info->opc_settings.nsIndex, newname), &dataType);
	     
	     if(retval == UA_STATUSCODE_GOOD) strcpy(name,newname);

	   }

	   UA_Variant *val = UA_Variant_new();
	   retval = UA_Client_readValueAttribute(info->client, UA_NODEID_STRING(info->opc_settings.nsIndex,name), val);

	   if(UA_Variant_isScalar(val)){
	     (info->channel_type).push_back(dataType.identifier.numeric-1);
	     (info->channel_name).push_back((char*)(name));
	     (info->channel_size).push_back(1);
	     printf("[%d] %s\n",(info->channel_name).size()-1,name);
	   }
	   else{
	     for(int ia=0;ia<val->arrayLength;ia++){
	       (info->channel_type).push_back(dataType.identifier.numeric-1);
	       char newname[256];
	       sprintf(newname,"a%d_%s",ia,(char*)(name));
	       (info->channel_name).push_back(newname);
	       (info->channel_size).push_back(val->arrayLength);
	       printf("[%d] %s\n",(info->channel_name).size()-1,ref->nodeId.nodeId.identifier.string.data);
	     }
	   }

	   UA_Variant_delete(val);

	 }

       }

     }
     
   }
   
   UA_BrowseNextRequest bNextReq;
   UA_BrowseNextResponse bNextResp;				    
   UA_BrowseNextRequest_init(&bNextReq);
   
   bNextReq.releaseContinuationPoints = UA_FALSE;
   bNextReq.continuationPoints = &bResp.results[0].continuationPoint;
   bNextReq.continuationPointsSize = 1;
   
   bool hasRef;
   
   do {
     
     hasRef = false;
     
     bNextResp = UA_Client_Service_browseNext(info->client, bNextReq);
     
     for (size_t i = 0; i < bNextResp.resultsSize; i++) {

       for (size_t j = 0; j < bNextResp.results[i].referencesSize; j++) {
	     
	 hasRef = true;
	 
	 UA_ReferenceDescription *ref = &(bNextResp.results[i].references[j]);
	 if ((ref->nodeClass == UA_NODECLASS_OBJECT || ref->nodeClass == UA_NODECLASS_VARIABLE||ref->nodeClass == UA_NODECLASS_METHOD)) {
	   
	   if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
	     UA_NodeId dataType;

	     char name[256];
	     strcpy(name,ref->nodeId.nodeId.identifier.string.data);

	     UA_StatusCode retval = UA_Client_readDataTypeAttribute(info->client, UA_NODEID_STRING(info->opc_settings.nsIndex, name), &dataType);
	     
	     if (retval != UA_STATUSCODE_GOOD){ // Try to remove the last character, that sometimes is wrongly added
	       
	       char newname[256];
	       strcpy(newname,name);
	       newname[strlen(name)-1] = '\0';	     
	       retval = UA_Client_readDataTypeAttribute(info->client, UA_NODEID_STRING(info->opc_settings.nsIndex, newname), &dataType);
	       
	       if(retval == UA_STATUSCODE_GOOD) strcpy(name,newname);
	       
	     }
	     
	     UA_Variant *val = UA_Variant_new();
	     retval = UA_Client_readValueAttribute(info->client, UA_NODEID_STRING(info->opc_settings.nsIndex,name), val);
	     
	     if(UA_Variant_isScalar(val)){
	       (info->channel_type).push_back(dataType.identifier.numeric-1);
	       (info->channel_name).push_back((char*)(name));
	       (info->channel_size).push_back(1);
	       printf("[%d] %s\n",(info->channel_name).size()-1,name);
	     }
	     else{
	       for(int ia=0;ia<val->arrayLength;ia++){
		 (info->channel_type).push_back(dataType.identifier.numeric-1);
		 char newname[256];
		 sprintf(newname,"a%d_%s",ia,(char*)(name));
		 (info->channel_name).push_back(newname);
		 (info->channel_size).push_back(val->arrayLength);
		 printf("[%d] %s\n",(info->channel_name).size()-1,newname);
	       }
	     }
	     
	     UA_Variant_delete(val);
	     
	   }
	   
	 }
	 
       }

     }
       
     bNextReq.continuationPoints = &bNextResp.results[0].continuationPoint;
     bNextReq.continuationPointsSize = 1;
     
   } while(hasRef);

   printf("-----------------------\n");

   info->num_channels = info->channel_name.size();
   
   UA_BrowseRequest_deleteMembers(&bReq);
   UA_BrowseResponse_deleteMembers(&bResp);
   
   UA_BrowseRequest_clear(&bReq);
   UA_BrowseResponse_clear(&bResp);

   return FE_SUCCESS;

}

/*----------------------------------------------------------------------------*/

INT opc_exit(OPC_INFO * info)
{

   UA_Client_disconnect(info->client);
   UA_Client_delete(info->client);

   if (info)
      free(info);

   return FE_SUCCESS;
   
}

/*----------------------------------------------------------------------------*/

INT opc_get(OPC_INFO * info, INT channel, float *pvalue)
{
 

  UA_Variant *val = UA_Variant_new();
  string newname;
 
  ////Read value
  if(info->channel_size[channel] == 1){

    UA_StatusCode retval = UA_Client_readValueAttribute(info->client, UA_NODEID_STRING(info->opc_settings.nsIndex, info->channel_name[channel].c_str()), val);

    if (retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(val) &&
	val->type == &UA_TYPES[info->channel_type[channel]]
	) {
      
      if(info->channel_type[channel] == UA_TYPES_FLOAT){
	*pvalue = *(UA_Float*)val->data;
      }
      
      else if(info->channel_type[channel] == UA_TYPES_DOUBLE){
	*pvalue = *(UA_Double*)val->data;
      }
      
      else if(info->channel_type[channel] == UA_TYPES_INT16){
	*pvalue = *(UA_Int16*)val->data;
      }
      
      else if(info->channel_type[channel] == UA_TYPES_INT32){
	*pvalue = *(UA_Int32*)val->data;
      }
      
      else if(info->channel_type[channel] == UA_TYPES_INT64){
	*pvalue = *(UA_Int64*)val->data;
      }
      
      else if(info->channel_type[channel] == UA_TYPES_BOOLEAN){
	*pvalue = (*(UA_Boolean*)val->data) ? 1.0 : 0.0;
      }
      
      else if(info->channel_type[channel] == UA_TYPES_BYTE){
	*pvalue = *(UA_Byte*)val->data;
      }
      
      else {
	*pvalue = 0.;
      }

    }

  }

  else{

    char name[256];
    strcpy(name,info->channel_name[channel].c_str());
    char *ptr = strchr(name,'_');
    
    //printf("name: %s\n",&ptr[1]);

    UA_StatusCode retval = UA_Client_readValueAttribute(info->client, UA_NODEID_STRING(info->opc_settings.nsIndex, &ptr[1]), val);

    //ptr[0] = '\0';

    //printf("index %s:",name);
    
    int ia = atof(&name[1]);
        
    if(info->channel_type[channel] == UA_TYPES_UINT16){
    
      *pvalue = ((UA_UInt16*)(val->data))[ia];
    }

  }

  UA_Variant_delete(val);
  
  return FE_SUCCESS;

}


INT opc_set(OPC_INFO * info, INT channel, float value)
{

  ////Write value
  UA_Variant *myVariant = UA_Variant_new();

  if(info->channel_type[channel] == UA_TYPES_FLOAT){
    UA_Float typevalue = value;
    UA_Variant_setScalarCopy(myVariant, &typevalue, &UA_TYPES[info->channel_type[channel]]);
  }

  else if(info->channel_type[channel] == UA_TYPES_DOUBLE){
    UA_Double typevalue = value;
    UA_Variant_setScalarCopy(myVariant, &typevalue, &UA_TYPES[info->channel_type[channel]]);
  }

  else if(info->channel_type[channel] == UA_TYPES_INT16){
    UA_Int16 typevalue = value;
    UA_Variant_setScalarCopy(myVariant, &typevalue, &UA_TYPES[info->channel_type[channel]]);
  }

  else if(info->channel_type[channel] == UA_TYPES_INT32){
    UA_Int32 typevalue = value;
    UA_Variant_setScalarCopy(myVariant, &typevalue, &UA_TYPES[info->channel_type[channel]]);
  }

  else if(info->channel_type[channel] == UA_TYPES_INT64){
    UA_Int64 typevalue = value;
    UA_Variant_setScalarCopy(myVariant, &typevalue, &UA_TYPES[info->channel_type[channel]]);
  }

  else if(info->channel_type[channel] == UA_TYPES_BOOLEAN){
    UA_Boolean typevalue = (value != 0) ? true : false;
    UA_Variant_setScalarCopy(myVariant, &typevalue, &UA_TYPES[info->channel_type[channel]]);
  }

  else if(info->channel_type[channel] == UA_TYPES_BYTE){
    UA_Byte typevalue = value;
    UA_Variant_setScalarCopy(myVariant, &typevalue, &UA_TYPES[info->channel_type[channel]]);
  }
  
  else return FE_SUCCESS;
  
  UA_Client_writeValueAttribute(info->client, UA_NODEID_STRING(info->opc_settings.nsIndex, info->channel_name[channel].c_str()), myVariant);
  UA_Variant_delete(myVariant);
  
  return FE_SUCCESS;

}

/*---- device driver entry point -----------------------------------*/

INT opc(INT cmd, ...)
{
   va_list argptr;
   HNDLE hKey;
   INT channel, status;
   DWORD flags;
   float value, *pvalue;
   void *info, *bd;
   char *name;
   string full_name;
   regex e;
   
   va_start(argptr, cmd);
   status = FE_SUCCESS;

   switch (cmd) {
   case CMD_INIT:
      hKey = va_arg(argptr, HNDLE);
      info = va_arg(argptr, void *);
      channel = va_arg(argptr, INT);
      flags = va_arg(argptr, DWORD);
      bd = va_arg(argptr, void *);
      status = opc_init(hKey, info, channel, bd);
      break;

   case CMD_EXIT:
      info = va_arg(argptr, void *);
      status = opc_exit(info);
      break;

   case CMD_GET:
   case CMD_GET_DEMAND:
      info = va_arg(argptr, void *);
      channel = va_arg(argptr, INT);
      pvalue = va_arg(argptr, float *);
      status = opc_get(info, channel, pvalue);
      break;

   case CMD_SET:
      info = va_arg(argptr, void *);
      channel = va_arg(argptr, INT);
      value = (float)va_arg(argptr, double);
      status = opc_set(info, channel, value);
      break;

   case CMD_GET_LABEL:
      info = va_arg(argptr, void *);
      channel = va_arg(argptr, INT);
      name = va_arg(argptr, char *);
      full_name = ((OPC_INFO*)info)->channel_name[channel]; 
      e = regex("DB_");   
      full_name = regex_replace (full_name,e,"");   
      e = regex("VIS_");   
      full_name = regex_replace (full_name,e,"");   
      e = regex("Pressure");   
      full_name = regex_replace (full_name,e,"Press");   
      e = regex("Flowrate");   
      full_name = regex_replace (full_name,e,"Flow");   
      e = regex("Ricircolo");   
      full_name = regex_replace (full_name,e,"Rec");   
      e = regex("Purificazione");   
      full_name = regex_replace (full_name,e,"Pur");   
      e = regex("Analisi");   
      full_name = regex_replace (full_name,e,"Ana");   
      e = regex("Miscelazione");   
      full_name = regex_replace (full_name,e,"Mix");   
      strncpy(name, full_name.c_str(),20);
      status = FE_SUCCESS;
      break;

   default:
      break;
   }

   va_end(argptr);

   return status;
}

/*------------------------------------------------------------------*/
