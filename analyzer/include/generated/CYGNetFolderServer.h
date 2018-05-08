/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGNetFolderServer_H
#define CYGNetFolderServer_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGNetFolderServer                                                         //
//                                                                            //
// This class implements the NetFolderServer for CYGAnalyzer.                 //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include "ROMENetFolderServer.h"
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <ROMETGraph.h>
#include <ROMETGraphErrors.h>
#include <TGraph2D.h>
#include <ROMETCutG.h>
class CYGODB;
class CYGEvent;
class CYGHit;
class CYGLightCluster;
class CYGAnalyzerConfigParameters;


class CYGNetFolderServer : public ROMENetFolderServer
{
protected:
   // Folder Fields
   Bool_t fFolderActive[kMaxSocketClients][5]; //! Flag if folder is active
   TObjArray* fFolder[kMaxSocketClients]; //! Handle to Folders

public:
   void              StartServer(TApplication *app,Int_t port,const char* serverName);
   static Int_t      ResponseFunction(TSocket *socket);

private:
   CYGNetFolderServer(const CYGNetFolderServer &c); // not implemented
   CYGNetFolderServer &operator=(const CYGNetFolderServer &c); // not implemented

public:
   CYGNetFolderServer():ROMENetFolderServer(){}
   virtual ~CYGNetFolderServer(){}

   Bool_t GetODBFolderActive(Int_t i) const { return fFolderActive[i][0]; }
   Bool_t GetEventFolderActive(Int_t i) const { return fFolderActive[i][1]; }
   Bool_t GetHitFolderActive(Int_t i) const { return fFolderActive[i][2]; }
   Bool_t GetLightClusterFolderActive(Int_t i) const { return fFolderActive[i][3]; }
   Bool_t GetAnalyzerConfigParametersFolderActive(Int_t i) const { return fFolderActive[i][4]; }

   Bool_t UpdateObjects();
   void   ConstructObjects(TSocket* socket);
   void   DestructObjects(TSocket* socket);

protected:
   static Int_t      CheckCommand(TSocket *socket,char *str);
   static THREADTYPE Server(void *arg);
   static THREADTYPE ServerLoop(void *arg);

   ClassDef(CYGNetFolderServer, 0)
};

#endif   // CYGNetFolderServer_H
