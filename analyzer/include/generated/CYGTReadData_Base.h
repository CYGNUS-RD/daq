/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGTReadData_Base_H
#define CYGTReadData_Base_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGTReadData_Base                                                          //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include <RConfig.h>
#include "ROMETask.h"
#include "ROMEAnalyzer.h"

class CYGTReadData_Base : public ROMETask
{
protected:

private:
   CYGTReadData_Base(const CYGTReadData_Base &c); // not implemented
   CYGTReadData_Base &operator=(const CYGTReadData_Base &c); // not implemented

public:
   CYGTReadData_Base(const char *name,const char *title,int level,const char *taskSuffix,TFolder *histoFolder);
   virtual ~CYGTReadData_Base() {}

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


   ClassDef(CYGTReadData_Base,0) // Base class of CYGTReadData
};

#endif   // CYGTReadData_Base_H
