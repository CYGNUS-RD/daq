#ifndef CYGTWriteData_H
#define CYGTWriteData_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGTWriteData                                                              //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include "generated/CYGTWriteData_Base.h"

class CYGTWriteData : public CYGTWriteData_Base
{

private:
   CYGTWriteData(const CYGTWriteData &c); // not implemented
   CYGTWriteData &operator=(const CYGTWriteData &c); // not implemented

public:
   CYGTWriteData(const char *name = 0, const char *title = 0, int level = 0, const char *taskSuffix = 0, TFolder *histoFolder = 0)
   :CYGTWriteData_Base(name,title,level,taskSuffix,histoFolder) {}
   virtual ~CYGTWriteData() {}

protected:
   // Event Methods
   void Init();
   void BeginOfRun();
   void Event();
   void EndOfRun();
   void Terminate();


   ClassDef(CYGTWriteData,0)
};

#endif   // CYGTWriteData_H
