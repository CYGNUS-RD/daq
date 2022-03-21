/********************************************************************\

  Name:         opc.cxx
  Created by:   Francesco Renga

  Contents:     Device Driver for generic OPC server

  $Id: mscbdev.c 3428 2006-12-06 08:49:38Z ritt $

\********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
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
IP = STRING : [32] 172.17.19.70:4870\n\
Namespace Index = INT : 3\n\
Tags Guid = STRING : [64] ecef81d5-c834-4379-ab79-c8fa3133a311\n\
"

typedef struct {
  OPC_SETTINGS opc_settings;
  UA_Client* client;
  int num_channels;
  vector<string> channel_name;
  vector<int> channel_type;
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

   printf("---- OPC variables ----\n");
   
   for(size_t i = 0; i < bResp.resultsSize; ++i) {

     for(size_t j = 0; j < bResp.results[i].referencesSize; ++j) {

       UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
       if ((ref->nodeClass == UA_NODECLASS_OBJECT || ref->nodeClass == UA_NODECLASS_VARIABLE||ref->nodeClass == UA_NODECLASS_METHOD)) {

	 if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
	   UA_NodeId dataType;
	   UA_Client_readDataTypeAttribute(info->client, UA_NODEID_STRING(info->opc_settings.nsIndex, ref->nodeId.nodeId.identifier.string.data), &dataType);
	   (info->channel_type).push_back(dataType.identifier.numeric-1);
	   (info->channel_name).push_back((char*)(ref->nodeId.nodeId.identifier.string.data));
	   printf("[%d] %s\n",(info->channel_name).size()-1,ref->nodeId.nodeId.identifier.string.data);
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
	     UA_Client_readDataTypeAttribute(info->client, UA_NODEID_STRING(info->opc_settings.nsIndex, ref->nodeId.nodeId.identifier.string.data), &dataType);
	     (info->channel_type).push_back(dataType.identifier.numeric-1);
	     (info->channel_name).push_back((char*)(ref->nodeId.nodeId.identifier.string.data));
	     printf("[%d] %s\n",(info->channel_name).size()-1,ref->nodeId.nodeId.identifier.string.data);
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

  ////Read value
  UA_Variant *val = UA_Variant_new();
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
      strncpy(name, ((OPC_INFO*)info)->channel_name[channel].c_str(),20);      
      status = FE_SUCCESS;
      break;

   default:
      break;
   }

   va_end(argptr);

   return status;
}

/*------------------------------------------------------------------*/
