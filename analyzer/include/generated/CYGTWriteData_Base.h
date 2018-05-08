/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGTWriteData_Base_H
#define CYGTWriteData_Base_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGTWriteData_Base                                                         //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include <RConfig.h>
#include "ROMETask.h"
#include "ROMEAnalyzer.h"

class CYGTWriteData_Base : public ROMETask
{
protected:

private:
   CYGTWriteData_Base(const CYGTWriteData_Base &c); // not implemented
   CYGTWriteData_Base &operator=(const CYGTWriteData_Base &c); // not implemented

public:
   CYGTWriteData_Base(const char *name,const char *title,int level,const char *taskSuffix,TFolder *histoFolder);
   virtual ~CYGTWriteData_Base() {}

   // User Methods
   Int_t GetObjectInterpreterCode(const char* objectPath) const;
   Int_t GetObjectInterpreterIntValue(Int_t code,Int_t defaultValue) const;
   Double_t GetObjectInterpreterDoubleValue(Int_t code,Double_t defaultValue) const;
   ROMEString& GetObjectInterpreterCharValue(Int_t code,ROMEString& defaultValue,ROMEString& buffer) const;

protected:
   // Event Methods
   virtual void Init() = 0;
   virtual void BeginOfRun() = 0;
   virtual void Event() = 0;
   virtual void EndOfRun() = 0;
   virtual void Terminate() = 0;

   // Histo Methods
   void SetOriginalHistoParameters();

   // Graph Methods
   void SetOriginalGraphParameters();


   ClassDef(CYGTWriteData_Base,0) // Base class of CYGTWriteData
};

#endif   // CYGTWriteData_Base_H
