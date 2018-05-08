/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGAnalyzer_H
#define CYGAnalyzer_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGAnalyzer                                                                //
//                                                                            //
// Basic class for the CYGAnalyzer. This class creates and manages all        //
// Folders, Tasks and Trees.                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include "ROMEAnalyzer.h"
#include <RConfig.h>
#include <TClonesArray.h>
#include <TH1.h>
#include <TTree.h>
#include <TFile.h>
class CYGGlobalSteering;
class CYGWindow;
class CYGMidasDAQ;
class CYGRomeDAQ;
class ROMEDataBaseDAQ;
class CYGODB;
class CYGEvent;
class CYGHit;
class CYGLightCluster;
class CYGAnalyzerConfigParameters;

class CYGTReadData;
class CYGTReadData_Base;
class CYGTWriteData;
class CYGTWriteData_Base;

class CYGEventLoop;
class CYGConfig;
class CYGDBAccess;
class CYGConfigToForm;


class CYGAnalyzer : public ROMEAnalyzer
{
friend class CYGWindow;
friend class CYGEventLoop;
friend class CYGConfig;
friend class CYGGlobalSteering;
protected:
   // Folder Fields
   CYGODB* fODBFolder; // Handle to CYGODB Folder
   CYGODB* fODBFolderStorage; // Handle to CYGODB Folder Storage
   CYGEvent* fEventFolder; // Handle to CYGEvent Folder
   CYGEvent* fEventFolderStorage; // Handle to CYGEvent Folder Storage
   TClonesArray* fHitFolders; // Handle to CYGHit Folders
   TClonesArray* fHitFoldersStorage; // Handle to CYGHit Folders Storage
   TClonesArray* fLightClusterFolders; // Handle to CYGLightCluster Folders
   TClonesArray* fLightClusterFoldersStorage; // Handle to CYGLightCluster Folders Storage
   CYGAnalyzerConfigParameters* fAnalyzerConfigParametersFolder; // Handle to CYGAnalyzerConfigParameters Folder
   CYGAnalyzerConfigParameters* fAnalyzerConfigParametersFolderStorage; // Handle to CYGAnalyzerConfigParameters Folder Storage

   // Steering Parameter Fields
   CYGGlobalSteering* fGlobalSteeringParameters; // Handle to the GlobalSteering Class

   // DAQ Handle
   CYGMidasDAQ* fMidasDAQ; // Handle to the Midas DAQ Class
   CYGRomeDAQ* fRomeDAQ; // Handle to the Rome DAQ Class
   ROMEDataBaseDAQ* fDataBaseDAQ; // Handle to the DataBase DAQ Class

   CYGDBAccess* fDBAccess; // Handle to the DBAccess Class

private:
   CYGAnalyzer(const CYGAnalyzer &c); // not implemented
   CYGAnalyzer &operator=(const CYGAnalyzer &c); // not implemented

public:
   CYGAnalyzer(ROMERint *app = dynamic_cast<ROMERint*>(gApplication), Bool_t batch = kFALSE,
               Bool_t daemon = kFALSE, Bool_t nographics = kFALSE, Int_t mode = 0);
   virtual ~CYGAnalyzer();
   // Folders
   CYGODB* GetODB();
   CYGODB** GetODBAddress();
   CYGODB* GetODBFolderStorage() const { return fODBFolderStorage; } 
   void   UpdateODB();
   void SetODB(CYGODB* pointer);
   CYGEvent* GetEvent();
   CYGEvent** GetEventAddress();
   CYGEvent* GetEventFolderStorage() const { return fEventFolderStorage; } 
   void   UpdateEvent();
   void SetEvent(CYGEvent* pointer);
   CYGHit* GetHitAt(Int_t indx);
   TClonesArray* GetHits();
   TClonesArray** GetHitAddress();
   TClonesArray* GetHitFoldersStorage() const { return fHitFoldersStorage; }
   void   UpdateHit();
   void SetHits(TClonesArray* pointer);
   CYGLightCluster* GetLightClusterAt(Int_t indx);
   TClonesArray* GetLightClusters();
   TClonesArray** GetLightClusterAddress();
   TClonesArray* GetLightClusterFoldersStorage() const { return fLightClusterFoldersStorage; }
   void   UpdateLightCluster();
   void SetLightClusters(TClonesArray* pointer);
   CYGAnalyzerConfigParameters* GetAnalyzerConfigParameters();
   CYGAnalyzerConfigParameters** GetAnalyzerConfigParametersAddress();
   CYGAnalyzerConfigParameters* GetAnalyzerConfigParametersFolderStorage() const { return fAnalyzerConfigParametersFolderStorage; } 
   void   UpdateAnalyzerConfigParameters();
   void SetAnalyzerConfigParameters(CYGAnalyzerConfigParameters* pointer);

   void FolderArrayOutOfBouds(Int_t index,const char* folder,const char* arraySize) const;
   Bool_t RegisterFolder(const char* object);
   Bool_t UnRegisterFolder(const char* object);

   // Config Parameter Folder
   void FillConfigParametersFolder();
   void SaveConfigParametersFolder() const;
   // Storage
   void   FillObjectStorage();
   // Check dependence
   Bool_t CheckDependences() const;
   void   UpdateConfigParameters();
   // Check tree names
   Bool_t CheckTreeFileNames() const;
   void   SetHitSize(Int_t number);
   Int_t  GetHitSize() const { return fHitFolders->GetEntriesFast(); }
   void   SetLightClusterSize(Int_t number);
   Int_t  GetLightClusterSize() const { return fLightClusterFolders->GetEntriesFast(); }


   TH1* GetHisto(const char* pathToHisto) const;
   bool ResetAllHistos();


   TH1* GetGraph(const char* pathToGraph) const;
   bool ResetAllGraphs();

   // Tasks
   void ConstructHistoFolders(TFolder *f, Bool_t addToArray);
   void ConstructHistoDirectories(TDirectory *d, TObjArray *cratedDir);
   void InitTasks();

   CYGTReadData* GetReadDataTask() const { return reinterpret_cast<CYGTReadData*>(GetTaskObjectAt(0)); }
   CYGTReadData_Base* GetReadDataTaskBase() const { return reinterpret_cast<CYGTReadData_Base*>(GetTaskObjectAt(0)); }
   CYGTWriteData* GetWriteDataTask() const { return reinterpret_cast<CYGTWriteData*>(GetTaskObjectAt(1)); }
   CYGTWriteData_Base* GetWriteDataTaskBase() const { return reinterpret_cast<CYGTWriteData_Base*>(GetTaskObjectAt(1)); }

   // Trees
   TTree* GetDataTreeTree() const { return static_cast<ROMETree*>(fTreeObjects->At(0))->GetTree(); }
   TFile* GetDataTreeFile() const { return static_cast<ROMETree*>(fTreeObjects->At(0))->GetFile(); }

   // Database
   ROMEDataBase* GetXMLDataBase() const { return GetDataBase("XML"); }
   ROMEDataBase* GetTextDataBase() const { return GetDataBase("TEXT"); }
   ROMEDataBase* GetODBDataBase() const { return GetDataBase("ODB"); }

   // DataBase Methodes
   Bool_t ReadSingleDataBaseFolders();
   Bool_t ReadArrayDataBaseFolders();
   Bool_t GetDBPathWriteFlag(const char* path) const;

   CYGDBAccess* GetDBAccess() const { return fDBAccess; }

   // Object Interpreter
   Int_t       GetObjectInterpreterCode(const char* objectPath) const;
   Int_t       GetObjectInterpreterIntValue(Int_t code,Int_t defaultValue) const;
   Double_t    GetObjectInterpreterDoubleValue(Int_t code,Double_t defaultValue) const;
   ROMEString& GetObjectInterpreterCharValue(Int_t code,ROMEString& defaultValue,ROMEString& buffer) const;

   // Steering Parameter Methodes
   CYGGlobalSteering* GetGSP() const { return fGlobalSteeringParameters; }

   // Deprecated DAQ Access Methods
   CYGMidasDAQ* GetMidas() const { return GetMidasDAQ(); }
   CYGRomeDAQ* GetRome() const { return GetRomeDAQ(); }
   // Midas DAQ Access Methods
   Bool_t IsMidasDAQ() const { return fMidasDAQ != 0; }
   CYGMidasDAQ* GetMidasDAQ() const;
   void     SetMidasDAQ(CYGMidasDAQ* handle) { fMidasDAQ = handle; }
   // Rome DAQ Access Methods
   Bool_t IsRomeDAQ() const { return fRomeDAQ != 0; }
   CYGRomeDAQ* GetRomeDAQ() const;
   void     SetRomeDAQ(CYGRomeDAQ* handle) { fRomeDAQ = handle; }
   // DataBase DAQ Access Methods
   Bool_t IsDataBaseDAQ() const { return fDataBaseDAQ != 0; }
   ROMEDataBaseDAQ* GetDataBaseDAQ() const;
   void     SetDataBaseDAQ(ROMEDataBaseDAQ* handle) { fDataBaseDAQ = handle; }

   Bool_t   DumpFolders(const char* filename, Bool_t only_database = kFALSE) const;
   Bool_t   LoadFolders(const char* filename, Bool_t only_database = kFALSE);

   Bool_t   ShowConfigurationFile();
   Bool_t   SaveHisto(CYGConfigToForm *dialog,const char* path,ROMEHisto* histo,ROMEConfigHisto* configHisto,Int_t histoDimension);
   Bool_t   SaveModes(CYGConfigToForm *dialog);
   Bool_t   SaveOffline(CYGConfigToForm *dialog);
   Bool_t   SaveOnline(CYGConfigToForm *dialog);
   Bool_t   SavePaths(CYGConfigToForm *dialog);
   Bool_t   SaveCommon(CYGConfigToForm *dialog);
   Bool_t   SaveSettings(CYGConfigToForm *dialog);
   Bool_t   SaveFloatingPointExceptionTrap(CYGConfigToForm *dialog);
   Bool_t   SaveSocketServer(CYGConfigToForm *dialog);
   Bool_t   SaveDataBase(CYGConfigToForm *dialog);
   Bool_t   SaveFolders(CYGConfigToForm *dialog);
   Bool_t   SaveODB(CYGConfigToForm *dialog);
   Bool_t   SaveTrees(CYGConfigToForm *dialog);
   Bool_t   SaveGlobalSteeringParameters(CYGConfigToForm *dialog);
   Bool_t   SaveAnalyzer(CYGConfigToForm *dialog);
   Bool_t   SaveMonitor(CYGConfigToForm *dialog);
   Bool_t   SaveMidas(CYGConfigToForm *dialog);

   CYGWindow *GetWindow() const { return reinterpret_cast<CYGWindow*>(fWindow); }
private:
   Bool_t ReadUserParameter(const char* opt, const char *value, Int_t& i);
   void   UserParameterUsage() const;
   void   startSplashScreen();
   void   consoleStartScreen();
   Bool_t ConnectSocketClientNetFolder(TSocket *sock);
   void   StartNetFolderServer();

   ClassDef(CYGAnalyzer, 0);
};

extern CYGAnalyzer *gAnalyzer;  // global Analyzer Handle

#endif   // CYGAnalyzer_H
