#ifndef CYGTReadData_H
#define CYGTReadData_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGTReadData                                                               //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include "generated/CYGTReadData_Base.h"

class CYGTReadData : public CYGTReadData_Base
{

private:
   CYGTReadData(const CYGTReadData &c); // not implemented
   CYGTReadData &operator=(const CYGTReadData &c); // not implemented

public:
   CYGTReadData(const char *name = 0, const char *title = 0, int level = 0, const char *taskSuffix = 0, TFolder *histoFolder = 0)
   :CYGTReadData_Base(name,title,level,taskSuffix,histoFolder) {}
   virtual ~CYGTReadData() {}

protected:
   // Event Methods
   void Init();
   void BeginOfRun();
   void Event();
   void EndOfRun();
   void Terminate();


   ClassDef(CYGTReadData,0)
};

#endif   // CYGTReadData_H
