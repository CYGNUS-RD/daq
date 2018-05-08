/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGConfig_H
#define CYGConfig_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGConfig                                                                  //
//                                                                            //
// This class handles the framework configuration file for CYGAnalyzer.       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include <TArrayL64.h>
#include "ROMEString.h"
#include "ROMEStrArray.h"
#include "ROMEConfigHisto.h"
#include "ROMEConfigGraph.h"
#include "ROMEConfig.h"
class ROMEXML;

class CYGConfig : public ROMEConfig
{
#if !defined(__CINT__)
public:

   // ConfigData;
   class ConfigData {
   public:
      Int_t        fLastRunNumberIndex;
      TArrayL64    fRunNumberArray;
      Int_t        fLastInputFileNameIndex;
      ROMEStrArray fInputFileNameArray;

      // Modes;
      class Modes {
      public:
         ROMEString   fAnalyzingMode;
         Bool_t       fAnalyzingModeModified;
         ROMEString   fDAQSystem;
         Bool_t       fDAQSystemModified;
      private:
         Modes(const Modes &c); // not implemented
         Modes &operator=(const Modes &c); // not implemented
      public:
         Modes()
         :fAnalyzingMode("")
         ,fAnalyzingModeModified(kFALSE)
         ,fDAQSystem("")
         ,fDAQSystemModified(kFALSE)
         {
         }
         ~Modes() {
         }
      };
      Modes* fModes;
      Bool_t fModesModified;

      // Offline;
      class Offline {
      public:
         ROMEString   fRunNumbers;
         Bool_t       fRunNumbersModified;
         ROMEString   fEventNumbers;
         Bool_t       fEventNumbersModified;
         ROMEString   fEventStep;
         Bool_t       fEventStepModified;
         ROMEString   fInputFileNames;
         Bool_t       fInputFileNamesModified;
      private:
         Offline(const Offline &c); // not implemented
         Offline &operator=(const Offline &c); // not implemented
      public:
         Offline()
         :fRunNumbers("")
         ,fRunNumbersModified(kFALSE)
         ,fEventNumbers("")
         ,fEventNumbersModified(kFALSE)
         ,fEventStep("")
         ,fEventStepModified(kFALSE)
         ,fInputFileNames("")
         ,fInputFileNamesModified(kFALSE)
         {
         }
         ~Offline() {
         }
      };
      Offline* fOffline;
      Bool_t fOfflineModified;

      // Online;
      class Online {
      public:
         ROMEString   fHost;
         Bool_t       fHostModified;
         ROMEString   fExperiment;
         Bool_t       fExperimentModified;
         ROMEString   fAnalyzerName;
         Bool_t       fAnalyzerNameModified;
         ROMEString   fMemoryBuffer;
         Bool_t       fMemoryBufferModified;
         ROMEString   fReadConfigFromODB;
         Bool_t       fReadConfigFromODBModified;
      private:
         Online(const Online &c); // not implemented
         Online &operator=(const Online &c); // not implemented
      public:
         Online()
         :fHost("")
         ,fHostModified(kFALSE)
         ,fExperiment("")
         ,fExperimentModified(kFALSE)
         ,fAnalyzerName("")
         ,fAnalyzerNameModified(kFALSE)
         ,fMemoryBuffer("")
         ,fMemoryBufferModified(kFALSE)
         ,fReadConfigFromODB("")
         ,fReadConfigFromODBModified(kFALSE)
         {
         }
         ~Online() {
         }
      };
      Online* fOnline;
      Bool_t fOnlineModified;

      // Paths;
      class Paths {
      public:
         ROMEString   fInputFilePath;
         Bool_t       fInputFilePathModified;
         ROMEString   fOutputFilePath;
         Bool_t       fOutputFilePathModified;
         ROMEString   fOutputFileOption;
         Bool_t       fOutputFileOptionModified;
         ROMEString   fMakeOutputDirectory;
         Bool_t       fMakeOutputDirectoryModified;
      private:
         Paths(const Paths &c); // not implemented
         Paths &operator=(const Paths &c); // not implemented
      public:
         Paths()
         :fInputFilePath("")
         ,fInputFilePathModified(kFALSE)
         ,fOutputFilePath("")
         ,fOutputFilePathModified(kFALSE)
         ,fOutputFileOption("")
         ,fOutputFileOptionModified(kFALSE)
         ,fMakeOutputDirectory("")
         ,fMakeOutputDirectoryModified(kFALSE)
         {
         }
         ~Paths() {
         }
      };
      Paths* fPaths;
      Bool_t fPathsModified;

      // Common;
      class Common {
      public:

         // Settings;
         class Settings {
         public:
            ROMEString   fVerboseLevel;
            Bool_t       fVerboseLevelModified;
            ROMEString   fReportMaxCount;
            Bool_t       fReportMaxCountModified;
            ROMEString   fReportSummaryFileName;
            Bool_t       fReportSummaryFileNameModified;
            ROMEString   fReportSummaryFileLevel;
            Bool_t       fReportSummaryFileLevelModified;
            ROMEString   fReportSummaryLineLength;
            Bool_t       fReportSummaryLineLengthModified;
            ROMEString   fConfigCommentLevel;
            Bool_t       fConfigCommentLevelModified;
            ROMEString   fQuitMode;
            Bool_t       fQuitModeModified;
            ROMEString   fShowSplashScreen;
            Bool_t       fShowSplashScreenModified;
            ROMEString   fGraphicalConfigEdit;
            Bool_t       fGraphicalConfigEditModified;
            ROMEString   fPreserveConfig;
            Bool_t       fPreserveConfigModified;
         private:
            Settings(const Settings &c); // not implemented
            Settings &operator=(const Settings &c); // not implemented
         public:
            Settings()
            :fVerboseLevel("")
            ,fVerboseLevelModified(kFALSE)
            ,fReportMaxCount("")
            ,fReportMaxCountModified(kFALSE)
            ,fReportSummaryFileName("")
            ,fReportSummaryFileNameModified(kFALSE)
            ,fReportSummaryFileLevel("")
            ,fReportSummaryFileLevelModified(kFALSE)
            ,fReportSummaryLineLength("")
            ,fReportSummaryLineLengthModified(kFALSE)
            ,fConfigCommentLevel("")
            ,fConfigCommentLevelModified(kFALSE)
            ,fQuitMode("")
            ,fQuitModeModified(kFALSE)
            ,fShowSplashScreen("")
            ,fShowSplashScreenModified(kFALSE)
            ,fGraphicalConfigEdit("")
            ,fGraphicalConfigEditModified(kFALSE)
            ,fPreserveConfig("")
            ,fPreserveConfigModified(kFALSE)
            {
            }
            ~Settings() {
            }
         };
         Settings* fSettings;
         Bool_t fSettingsModified;

         // FloatingPointExceptionTrap;
         class FloatingPointExceptionTrap {
         public:
            ROMEString   fInvalid;
            Bool_t       fInvalidModified;
            ROMEString   fDivByZero;
            Bool_t       fDivByZeroModified;
            ROMEString   fOverflow;
            Bool_t       fOverflowModified;
            ROMEString   fUnderflow;
            Bool_t       fUnderflowModified;
            ROMEString   fInexact;
            Bool_t       fInexactModified;
         private:
            FloatingPointExceptionTrap(const FloatingPointExceptionTrap &c); // not implemented
            FloatingPointExceptionTrap &operator=(const FloatingPointExceptionTrap &c); // not implemented
         public:
            FloatingPointExceptionTrap()
            :fInvalid("")
            ,fInvalidModified(kFALSE)
            ,fDivByZero("")
            ,fDivByZeroModified(kFALSE)
            ,fOverflow("")
            ,fOverflowModified(kFALSE)
            ,fUnderflow("")
            ,fUnderflowModified(kFALSE)
            ,fInexact("")
            ,fInexactModified(kFALSE)
            {
            }
            ~FloatingPointExceptionTrap() {
            }
         };
         FloatingPointExceptionTrap* fFloatingPointExceptionTrap;
         Bool_t fFloatingPointExceptionTrapModified;

         // SocketServer;
         class SocketServer {
         public:
            ROMEString   fActive;
            Bool_t       fActiveModified;
            ROMEString   fPortNumber;
            Bool_t       fPortNumberModified;
         private:
            SocketServer(const SocketServer &c); // not implemented
            SocketServer &operator=(const SocketServer &c); // not implemented
         public:
            SocketServer()
            :fActive("")
            ,fActiveModified(kFALSE)
            ,fPortNumber("")
            ,fPortNumberModified(kFALSE)
            {
            }
            ~SocketServer() {
            }
         };
         SocketServer* fSocketServer;
         Bool_t fSocketServerModified;

         // DataBase;
         class DataBase {
         public:
            ROMEString   fName;
            Bool_t       fNameModified;
            ROMEString   fConnection;
            Bool_t       fConnectionModified;
            ROMEString   fType;
            Bool_t       fTypeModified;
            ROMEString   fEventBased;
            Bool_t       fEventBasedModified;
         private:
            DataBase(const DataBase &c); // not implemented
            DataBase &operator=(const DataBase &c); // not implemented
         public:
            DataBase()
            :fName("")
            ,fNameModified(kFALSE)
            ,fConnection("")
            ,fConnectionModified(kFALSE)
            ,fType("")
            ,fTypeModified(kFALSE)
            ,fEventBased("")
            ,fEventBasedModified(kFALSE)
            {
            }
            ~DataBase() {
            }
         };
         DataBase** fDataBase;
         Bool_t *fDataBaseModified;
         Bool_t fDataBaseArrayModified;
         Int_t  fDataBaseArraySize;

         // Folders;
         class Folders {
         public:

            // ODB;
            class ODB {
            public:

               // RunNumber;
               class RunNumber {
               public:
                  ROMEString   fDataBaseName;
                  Bool_t       fDataBaseNameModified;
                  ROMEString   fDataBasePath;
                  Bool_t       fDataBasePathModified;
               private:
                  RunNumber(const RunNumber &c); // not implemented
                  RunNumber &operator=(const RunNumber &c); // not implemented
               public:
                  RunNumber()
                  :fDataBaseName("")
                  ,fDataBaseNameModified(kFALSE)
                  ,fDataBasePath("")
                  ,fDataBasePathModified(kFALSE)
                  {
                  }
                  ~RunNumber() {
                  }
               };
               RunNumber* fRunNumber;
               Bool_t fRunNumberModified;

               // StartTime;
               class StartTime {
               public:
                  ROMEString   fDataBaseName;
                  Bool_t       fDataBaseNameModified;
                  ROMEString   fDataBasePath;
                  Bool_t       fDataBasePathModified;
               private:
                  StartTime(const StartTime &c); // not implemented
                  StartTime &operator=(const StartTime &c); // not implemented
               public:
                  StartTime()
                  :fDataBaseName("")
                  ,fDataBaseNameModified(kFALSE)
                  ,fDataBasePath("")
                  ,fDataBasePathModified(kFALSE)
                  {
                  }
                  ~StartTime() {
                  }
               };
               StartTime* fStartTime;
               Bool_t fStartTimeModified;
            private:
               ODB(const ODB &c); // not implemented
               ODB &operator=(const ODB &c); // not implemented
            public:
               ODB()
               :fRunNumber(new RunNumber())
               ,fRunNumberModified(kFALSE)
               ,fStartTime(new StartTime())
               ,fStartTimeModified(kFALSE)
               {
               }
               ~ODB() {
                  SafeDelete(fRunNumber);
                  SafeDelete(fStartTime);
               }
            };
            ODB* fODB;
            Bool_t fODBModified;
         private:
            Folders(const Folders &c); // not implemented
            Folders &operator=(const Folders &c); // not implemented
         public:
            Folders()
            :fODB(new ODB())
            ,fODBModified(kFALSE)
            {
            }
            ~Folders() {
               SafeDelete(fODB);
            }
         };
         Folders* fFolders;
         Bool_t fFoldersModified;

         // Trees;
         class Trees {
         public:
            ROMEString   fAccumulate;
            Bool_t       fAccumulateModified;
            ROMEString   fMaxMemory;
            Bool_t       fMaxMemoryModified;

            // DataTree;
            class DataTree {
            public:
               ROMEString   fRead;
               Bool_t       fReadModified;
               ROMEString   fWrite;
               Bool_t       fWriteModified;
               ROMEString   fFill;
               Bool_t       fFillModified;
               ROMEString   fCompressionLevel;
               Bool_t       fCompressionLevelModified;
               ROMEString   fCompressionAlgorithm;
               Bool_t       fCompressionAlgorithmModified;
               ROMEString   fAutoSaveSize;
               Bool_t       fAutoSaveSizeModified;
               ROMEString   fAutoFlushSize;
               Bool_t       fAutoFlushSizeModified;
               ROMEString   fCacheSize;
               Bool_t       fCacheSizeModified;
               ROMEString   fMaxNumberOfEntries;
               Bool_t       fMaxNumberOfEntriesModified;
               ROMEString   fTreeInputFileName;
               Bool_t       fTreeInputFileNameModified;
               ROMEString   fTreeOutputFileName;
               Bool_t       fTreeOutputFileNameModified;
               ROMEString   fSaveConfiguration;
               Bool_t       fSaveConfigurationModified;

               // lightclusters;
               class lightclusters {
               public:
                  ROMEString   fActive;
                  Bool_t       fActiveModified;
                  ROMEString   fRead;
                  Bool_t       fReadModified;
               private:
                  lightclusters(const lightclusters &c); // not implemented
                  lightclusters &operator=(const lightclusters &c); // not implemented
               public:
                  lightclusters()
                  :fActive("")
                  ,fActiveModified(kFALSE)
                  ,fRead("")
                  ,fReadModified(kFALSE)
                  {
                  }
                  ~lightclusters() {
                  }
               };
               lightclusters* flightclusters;
               Bool_t flightclustersModified;
            private:
               DataTree(const DataTree &c); // not implemented
               DataTree &operator=(const DataTree &c); // not implemented
            public:
               DataTree()
               :fRead("")
               ,fReadModified(kFALSE)
               ,fWrite("")
               ,fWriteModified(kFALSE)
               ,fFill("")
               ,fFillModified(kFALSE)
               ,fCompressionLevel("")
               ,fCompressionLevelModified(kFALSE)
               ,fCompressionAlgorithm("")
               ,fCompressionAlgorithmModified(kFALSE)
               ,fAutoSaveSize("")
               ,fAutoSaveSizeModified(kFALSE)
               ,fAutoFlushSize("")
               ,fAutoFlushSizeModified(kFALSE)
               ,fCacheSize("")
               ,fCacheSizeModified(kFALSE)
               ,fMaxNumberOfEntries("")
               ,fMaxNumberOfEntriesModified(kFALSE)
               ,fTreeInputFileName("")
               ,fTreeInputFileNameModified(kFALSE)
               ,fTreeOutputFileName("")
               ,fTreeOutputFileNameModified(kFALSE)
               ,fSaveConfiguration("")
               ,fSaveConfigurationModified(kFALSE)
               ,flightclusters(new lightclusters())
               ,flightclustersModified(kFALSE)
               {
               }
               ~DataTree() {
                  SafeDelete(flightclusters);
               }
            };
            DataTree* fDataTree;
            Bool_t fDataTreeModified;
         private:
            Trees(const Trees &c); // not implemented
            Trees &operator=(const Trees &c); // not implemented
         public:
            Trees()
            :fAccumulate("")
            ,fAccumulateModified(kFALSE)
            ,fMaxMemory("")
            ,fMaxMemoryModified(kFALSE)
            ,fDataTree(new DataTree())
            ,fDataTreeModified(kFALSE)
            {
            }
            ~Trees() {
               SafeDelete(fDataTree);
            }
         };
         Trees* fTrees;
         Bool_t fTreesModified;

         // GlobalSteeringParameters;
         class GlobalSteeringParameters {
         public:
         private:
            GlobalSteeringParameters(const GlobalSteeringParameters &c); // not implemented
            GlobalSteeringParameters &operator=(const GlobalSteeringParameters &c); // not implemented
         public:
            GlobalSteeringParameters()
            {
            }
            ~GlobalSteeringParameters() {
            }
         };
         GlobalSteeringParameters* fGlobalSteeringParameters;
         Bool_t fGlobalSteeringParametersModified;
      private:
         Common(const Common &c); // not implemented
         Common &operator=(const Common &c); // not implemented
      public:
         Common()
         :fSettings(new Settings())
         ,fSettingsModified(kFALSE)
         ,fFloatingPointExceptionTrap(new FloatingPointExceptionTrap())
         ,fFloatingPointExceptionTrapModified(kFALSE)
         ,fSocketServer(new SocketServer())
         ,fSocketServerModified(kFALSE)
         ,fDataBase(0)
         ,fDataBaseModified(0)
         ,fDataBaseArrayModified(kFALSE)
         ,fDataBaseArraySize(0)
         ,fFolders(new Folders())
         ,fFoldersModified(kFALSE)
         ,fTrees(new Trees())
         ,fTreesModified(kFALSE)
         ,fGlobalSteeringParameters(new GlobalSteeringParameters())
         ,fGlobalSteeringParametersModified(kFALSE)
         {
         }
         ~Common() {
            int i;
            SafeDelete(fSettings);
            SafeDelete(fFloatingPointExceptionTrap);
            SafeDelete(fSocketServer);
            for (i = 0; i < fDataBaseArraySize; i++) {
               SafeDelete(fDataBase[i]);
            }
            SafeDeleteArray(fDataBaseModified);
            SafeDeleteArray(fDataBase);
            SafeDelete(fFolders);
            SafeDelete(fTrees);
            SafeDelete(fGlobalSteeringParameters);
         }
      };
      Common* fCommon;
      Bool_t fCommonModified;

      // Analyzer;
      class Analyzer {
      public:

         // HistogramRead;
         class HistogramRead {
         public:
            ROMEString   fRead;
            Bool_t       fReadModified;
            ROMEString   fRunNumber;
            Bool_t       fRunNumberModified;
            ROMEString   fPath;
            Bool_t       fPathModified;
            ROMEString   fFileName;
            Bool_t       fFileNameModified;
         private:
            HistogramRead(const HistogramRead &c); // not implemented
            HistogramRead &operator=(const HistogramRead &c); // not implemented
         public:
            HistogramRead()
            :fRead("")
            ,fReadModified(kFALSE)
            ,fRunNumber("")
            ,fRunNumberModified(kFALSE)
            ,fPath("")
            ,fPathModified(kFALSE)
            ,fFileName("")
            ,fFileNameModified(kFALSE)
            {
            }
            ~HistogramRead() {
            }
         };
         HistogramRead* fHistogramRead;
         Bool_t fHistogramReadModified;

         // HistogramWrite;
         class HistogramWrite {
         public:
            ROMEString   fWrite;
            Bool_t       fWriteModified;
            ROMEString   fPath;
            Bool_t       fPathModified;
            ROMEString   fFileName;
            Bool_t       fFileNameModified;
            ROMEString   fAccumulateAll;
            Bool_t       fAccumulateAllModified;
            ROMEString   fDeactivateAll;
            Bool_t       fDeactivateAllModified;
            ROMEString   fAutoSavePeriod;
            Bool_t       fAutoSavePeriodModified;
            ROMEString   fSnapShotFileName;
            Bool_t       fSnapShotFileNameModified;
            ROMEString   fSnapShotEvents;
            Bool_t       fSnapShotEventsModified;
         private:
            HistogramWrite(const HistogramWrite &c); // not implemented
            HistogramWrite &operator=(const HistogramWrite &c); // not implemented
         public:
            HistogramWrite()
            :fWrite("")
            ,fWriteModified(kFALSE)
            ,fPath("")
            ,fPathModified(kFALSE)
            ,fFileName("")
            ,fFileNameModified(kFALSE)
            ,fAccumulateAll("")
            ,fAccumulateAllModified(kFALSE)
            ,fDeactivateAll("")
            ,fDeactivateAllModified(kFALSE)
            ,fAutoSavePeriod("")
            ,fAutoSavePeriodModified(kFALSE)
            ,fSnapShotFileName("")
            ,fSnapShotFileNameModified(kFALSE)
            ,fSnapShotEvents("")
            ,fSnapShotEventsModified(kFALSE)
            {
            }
            ~HistogramWrite() {
            }
         };
         HistogramWrite* fHistogramWrite;
         Bool_t fHistogramWriteModified;

         // Macros;
         class Macros {
         public:
            ROMEString   fBeginOfRun;
            Bool_t       fBeginOfRunModified;
            ROMEString   fBeginOfEvent;
            Bool_t       fBeginOfEventModified;
            ROMEString   fEndOfEvent;
            Bool_t       fEndOfEventModified;
            ROMEString   fEndOfRun;
            Bool_t       fEndOfRunModified;
         private:
            Macros(const Macros &c); // not implemented
            Macros &operator=(const Macros &c); // not implemented
         public:
            Macros()
            :fBeginOfRun("")
            ,fBeginOfRunModified(kFALSE)
            ,fBeginOfEvent("")
            ,fBeginOfEventModified(kFALSE)
            ,fEndOfEvent("")
            ,fEndOfEventModified(kFALSE)
            ,fEndOfRun("")
            ,fEndOfRunModified(kFALSE)
            {
            }
            ~Macros() {
            }
         };
         Macros* fMacros;
         Bool_t fMacrosModified;

         // Tasks;
         class Tasks {
         public:

            // ReadData;
            class ReadData {
            public:
               ROMEString   fActive;
               Bool_t       fActiveModified;
            private:
               ReadData(const ReadData &c); // not implemented
               ReadData &operator=(const ReadData &c); // not implemented
            public:
               ReadData()
               :fActive("")
               ,fActiveModified(kFALSE)
               {
               }
               ~ReadData() {
               }
            };
            ReadData* fReadData;
            Bool_t fReadDataModified;

            // WriteData;
            class WriteData {
            public:
               ROMEString   fActive;
               Bool_t       fActiveModified;
            private:
               WriteData(const WriteData &c); // not implemented
               WriteData &operator=(const WriteData &c); // not implemented
            public:
               WriteData()
               :fActive("")
               ,fActiveModified(kFALSE)
               {
               }
               ~WriteData() {
               }
            };
            WriteData* fWriteData;
            Bool_t fWriteDataModified;
         private:
            Tasks(const Tasks &c); // not implemented
            Tasks &operator=(const Tasks &c); // not implemented
         public:
            Tasks()
            :fReadData(new ReadData())
            ,fReadDataModified(kFALSE)
            ,fWriteData(new WriteData())
            ,fWriteDataModified(kFALSE)
            {
            }
            ~Tasks() {
               SafeDelete(fReadData);
               SafeDelete(fWriteData);
            }
         };
         Tasks* fTasks;
         Bool_t fTasksModified;
      private:
         Analyzer(const Analyzer &c); // not implemented
         Analyzer &operator=(const Analyzer &c); // not implemented
      public:
         Analyzer()
         :fHistogramRead(new HistogramRead())
         ,fHistogramReadModified(kFALSE)
         ,fHistogramWrite(new HistogramWrite())
         ,fHistogramWriteModified(kFALSE)
         ,fMacros(new Macros())
         ,fMacrosModified(kFALSE)
         ,fTasks(new Tasks())
         ,fTasksModified(kFALSE)
         {
         }
         ~Analyzer() {
            SafeDelete(fHistogramRead);
            SafeDelete(fHistogramWrite);
            SafeDelete(fMacros);
            SafeDelete(fTasks);
         }
      };
      Analyzer* fAnalyzer;
      Bool_t fAnalyzerModified;

      // Monitor;
      class Monitor {
      public:

         // Display;
         class Display {
         public:
            ROMEString   fWindowScale;
            Bool_t       fWindowScaleModified;
            ROMEString   fStatusBar;
            Bool_t       fStatusBarModified;
            ROMEString   fUpdateFrequency;
            Bool_t       fUpdateFrequencyModified;
            ROMEString   fScreenShotPeriod;
            Bool_t       fScreenShotPeriodModified;
            ROMEString   fListTreeView;
            Bool_t       fListTreeViewModified;
            ROMEString   fTimeZone;
            Bool_t       fTimeZoneModified;

            // AnalyzerController;
            class AnalyzerController {
            public:
               ROMEString   fActive;
               Bool_t       fActiveModified;
               ROMEString   fNetFolderName;
               Bool_t       fNetFolderNameModified;
            private:
               AnalyzerController(const AnalyzerController &c); // not implemented
               AnalyzerController &operator=(const AnalyzerController &c); // not implemented
            public:
               AnalyzerController()
               :fActive("")
               ,fActiveModified(kFALSE)
               ,fNetFolderName("")
               ,fNetFolderNameModified(kFALSE)
               {
               }
               ~AnalyzerController() {
               }
            };
            AnalyzerController* fAnalyzerController;
            Bool_t fAnalyzerControllerModified;
         private:
            Display(const Display &c); // not implemented
            Display &operator=(const Display &c); // not implemented
         public:
            Display()
            :fWindowScale("")
            ,fWindowScaleModified(kFALSE)
            ,fStatusBar("")
            ,fStatusBarModified(kFALSE)
            ,fUpdateFrequency("")
            ,fUpdateFrequencyModified(kFALSE)
            ,fScreenShotPeriod("")
            ,fScreenShotPeriodModified(kFALSE)
            ,fListTreeView("")
            ,fListTreeViewModified(kFALSE)
            ,fTimeZone("")
            ,fTimeZoneModified(kFALSE)
            ,fAnalyzerController(new AnalyzerController())
            ,fAnalyzerControllerModified(kFALSE)
            {
            }
            ~Display() {
               SafeDelete(fAnalyzerController);
            }
         };
         Display* fDisplay;
         Bool_t fDisplayModified;

         // SocketClient;
         class SocketClient {
         public:
            ROMEString   fHost;
            Bool_t       fHostModified;
            ROMEString   fPort;
            Bool_t       fPortModified;
         private:
            SocketClient(const SocketClient &c); // not implemented
            SocketClient &operator=(const SocketClient &c); // not implemented
         public:
            SocketClient()
            :fHost("")
            ,fHostModified(kFALSE)
            ,fPort("")
            ,fPortModified(kFALSE)
            {
            }
            ~SocketClient() {
            }
         };
         SocketClient* fSocketClient;
         Bool_t fSocketClientModified;

         // Tabs;
         class Tabs {
         public:

            // EventDisplay;
            class EventDisplay {
            public:
               ROMEString   fActive;
               Bool_t       fActiveModified;
               ROMEString   fScreenShotFileName;
               Bool_t       fScreenShotFileNameModified;
               ROMEString   fNewWindow;
               Bool_t       fNewWindowModified;
            private:
               EventDisplay(const EventDisplay &c); // not implemented
               EventDisplay &operator=(const EventDisplay &c); // not implemented
            public:
               EventDisplay()
               :fActive("")
               ,fActiveModified(kFALSE)
               ,fScreenShotFileName("")
               ,fScreenShotFileNameModified(kFALSE)
               ,fNewWindow("")
               ,fNewWindowModified(kFALSE)
               {
               }
               ~EventDisplay() {
               }
            };
            EventDisplay* fEventDisplay;
            Bool_t fEventDisplayModified;

            // Features;
            class Features {
            public:
               ROMEString   fActive;
               Bool_t       fActiveModified;
               ROMEString   fScreenShotFileName;
               Bool_t       fScreenShotFileNameModified;
               ROMEString   fNewWindow;
               Bool_t       fNewWindowModified;
            private:
               Features(const Features &c); // not implemented
               Features &operator=(const Features &c); // not implemented
            public:
               Features()
               :fActive("")
               ,fActiveModified(kFALSE)
               ,fScreenShotFileName("")
               ,fScreenShotFileNameModified(kFALSE)
               ,fNewWindow("")
               ,fNewWindowModified(kFALSE)
               {
               }
               ~Features() {
               }
            };
            Features* fFeatures;
            Bool_t fFeaturesModified;
         private:
            Tabs(const Tabs &c); // not implemented
            Tabs &operator=(const Tabs &c); // not implemented
         public:
            Tabs()
            :fEventDisplay(new EventDisplay())
            ,fEventDisplayModified(kFALSE)
            ,fFeatures(new Features())
            ,fFeaturesModified(kFALSE)
            {
            }
            ~Tabs() {
               SafeDelete(fEventDisplay);
               SafeDelete(fFeatures);
            }
         };
         Tabs* fTabs;
         Bool_t fTabsModified;
      private:
         Monitor(const Monitor &c); // not implemented
         Monitor &operator=(const Monitor &c); // not implemented
      public:
         Monitor()
         :fDisplay(new Display())
         ,fDisplayModified(kFALSE)
         ,fSocketClient(new SocketClient())
         ,fSocketClientModified(kFALSE)
         ,fTabs(new Tabs())
         ,fTabsModified(kFALSE)
         {
         }
         ~Monitor() {
            SafeDelete(fDisplay);
            SafeDelete(fSocketClient);
            SafeDelete(fTabs);
         }
      };
      Monitor* fMonitor;
      Bool_t fMonitorModified;

      // Midas;
      class Midas {
      public:
         ROMEString   fMidasFileName;
         Bool_t       fMidasFileNameModified;
         ROMEString   fMidasByteSwap;
         Bool_t       fMidasByteSwapModified;
         ROMEString   fMidasOnlineCommunicationThread;
         Bool_t       fMidasOnlineCommunicationThreadModified;
         ROMEString   fMidasOnlineLoopPeriod;
         Bool_t       fMidasOnlineLoopPeriodModified;

         // DAQ;
         class DAQ {
         public:
            ROMEString   fActive;
            Bool_t       fActiveModified;

            // TDC0;
            class TDC0 {
            public:
               ROMEString   fActive;
               Bool_t       fActiveModified;
            private:
               TDC0(const TDC0 &c); // not implemented
               TDC0 &operator=(const TDC0 &c); // not implemented
            public:
               TDC0()
               :fActive("")
               ,fActiveModified(kFALSE)
               {
               }
               ~TDC0() {
               }
            };
            TDC0* fTDC0;
            Bool_t fTDC0Modified;

            // DIG0;
            class DIG0 {
            public:
               ROMEString   fActive;
               Bool_t       fActiveModified;
            private:
               DIG0(const DIG0 &c); // not implemented
               DIG0 &operator=(const DIG0 &c); // not implemented
            public:
               DIG0()
               :fActive("")
               ,fActiveModified(kFALSE)
               {
               }
               ~DIG0() {
               }
            };
            DIG0* fDIG0;
            Bool_t fDIG0Modified;

            // DIG1;
            class DIG1 {
            public:
               ROMEString   fActive;
               Bool_t       fActiveModified;
            private:
               DIG1(const DIG1 &c); // not implemented
               DIG1 &operator=(const DIG1 &c); // not implemented
            public:
               DIG1()
               :fActive("")
               ,fActiveModified(kFALSE)
               {
               }
               ~DIG1() {
               }
            };
            DIG1* fDIG1;
            Bool_t fDIG1Modified;

            // CAM0;
            class CAM0 {
            public:
               ROMEString   fActive;
               Bool_t       fActiveModified;
            private:
               CAM0(const CAM0 &c); // not implemented
               CAM0 &operator=(const CAM0 &c); // not implemented
            public:
               CAM0()
               :fActive("")
               ,fActiveModified(kFALSE)
               {
               }
               ~CAM0() {
               }
            };
            CAM0* fCAM0;
            Bool_t fCAM0Modified;
         private:
            DAQ(const DAQ &c); // not implemented
            DAQ &operator=(const DAQ &c); // not implemented
         public:
            DAQ()
            :fActive("")
            ,fActiveModified(kFALSE)
            ,fTDC0(new TDC0())
            ,fTDC0Modified(kFALSE)
            ,fDIG0(new DIG0())
            ,fDIG0Modified(kFALSE)
            ,fDIG1(new DIG1())
            ,fDIG1Modified(kFALSE)
            ,fCAM0(new CAM0())
            ,fCAM0Modified(kFALSE)
            {
            }
            ~DAQ() {
               SafeDelete(fTDC0);
               SafeDelete(fDIG0);
               SafeDelete(fDIG1);
               SafeDelete(fCAM0);
            }
         };
         DAQ* fDAQ;
         Bool_t fDAQModified;
      private:
         Midas(const Midas &c); // not implemented
         Midas &operator=(const Midas &c); // not implemented
      public:
         Midas()
         :fMidasFileName("")
         ,fMidasFileNameModified(kFALSE)
         ,fMidasByteSwap("")
         ,fMidasByteSwapModified(kFALSE)
         ,fMidasOnlineCommunicationThread("")
         ,fMidasOnlineCommunicationThreadModified(kFALSE)
         ,fMidasOnlineLoopPeriod("")
         ,fMidasOnlineLoopPeriodModified(kFALSE)
         ,fDAQ(new DAQ())
         ,fDAQModified(kFALSE)
         {
         }
         ~Midas() {
            SafeDelete(fDAQ);
         }
      };
      Midas* fMidas;
      Bool_t fMidasModified;
   private:
      ConfigData(const ConfigData &c); // not implemented
      ConfigData &operator=(const ConfigData &c); // not implemented
   public:
      ConfigData()
      :fLastRunNumberIndex(-1)
      ,fRunNumberArray()
      ,fLastInputFileNameIndex(-1)
      ,fInputFileNameArray()
      ,fModes(new Modes())
      ,fModesModified(kFALSE)
      ,fOffline(new Offline())
      ,fOfflineModified(kFALSE)
      ,fOnline(new Online())
      ,fOnlineModified(kFALSE)
      ,fPaths(new Paths())
      ,fPathsModified(kFALSE)
      ,fCommon(new Common())
      ,fCommonModified(kFALSE)
      ,fAnalyzer(new Analyzer())
      ,fAnalyzerModified(kFALSE)
      ,fMonitor(new Monitor())
      ,fMonitorModified(kFALSE)
      ,fMidas(new Midas())
      ,fMidasModified(kFALSE)
      {
      }
      ~ConfigData() {
         SafeDelete(fModes);
         SafeDelete(fOffline);
         SafeDelete(fOnline);
         SafeDelete(fPaths);
         SafeDelete(fCommon);
         SafeDelete(fAnalyzer);
         SafeDelete(fMonitor);
         SafeDelete(fMidas);
      }
   };
   ConfigData** fConfigData;

#endif
   ROMEString fXSDFile;
   Int_t fNumberOfRunConfigs;
   Int_t fActiveConfiguration;

private:
   CYGConfig(const CYGConfig &c); // not implemented
   CYGConfig &operator=(const CYGConfig &c); // not implemented

public:
   CYGConfig();
   virtual ~CYGConfig();
   Bool_t WriteConfigurationFile(const char *file) const;
   Bool_t ReadConfigurationFile(const char *file);
   Bool_t CheckConfiguration(Long64_t runNumber);
   Bool_t CheckConfiguration(const char *file);
   Bool_t CheckConfigurationModified(Int_t indx) const;

protected:
   Bool_t ReadProgramConfiguration(ROMEXML *xml);
   Bool_t ReadConfiguration(ROMEXML *xml,ROMEString& path,Int_t indx);
   Bool_t WriteProgramConfiguration(ROMEXML *xml) const;
   Bool_t WriteConfiguration(ROMEXML *xml,Int_t indx) const;
   Bool_t SetConfiguration(Int_t modIndex,Int_t indx);

   ClassDef(CYGConfig, 0)
};

#endif   // CYGConfig_H
