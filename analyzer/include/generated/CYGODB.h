/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGODB_H
#define CYGODB_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGODB                                                                     //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include <RConfig.h>
#include <vector>
#include <TObject.h>
#include <TClass.h>
#include <TClonesArray.h>
#include "ROMEString.h"

// Equality operator.
// Warning! All fields may not be compared.
class CYGODB;
Bool_t operator==(const CYGODB &lhs, const CYGODB &rhs);
inline Bool_t operator!=(const CYGODB &lhs, const CYGODB &rhs) { return !(lhs == rhs); }

class CYGODB : public TObject
{
protected:
   Int_t         RunNumber; // Runinfo/Run number
   ROMEString    StartTime; // Runinfo/Start time
   Bool_t        fModified; //! Modified Folder Flag

private:
   CYGODB(const CYGODB &c); // not implemented

public:
   CYGODB(Int_t RunNumber_value=0, ROMEString StartTime_value="" );

   virtual ~CYGODB();

   // Warning! All fields may not be compared.
   friend Bool_t operator==(const CYGODB &lhs, const CYGODB &rhs);
   friend Bool_t operator!=(const CYGODB &lhs, const CYGODB &rhs);

   // Warning! All fields may not be copied.
   CYGODB& operator=(const CYGODB &c);

   Int_t          GetRunNumber() const { return RunNumber; }
   ROMEString     GetStartTime() const { return StartTime; }

   virtual Bool_t         isModified();

   void SetRunNumber(Int_t         RunNumber_value) { RunNumber = RunNumber_value; SetModified(true); }
   void SetStartTime(ROMEString    StartTime_value) { StartTime = StartTime_value; SetModified(true); }

   void AddRunNumber(Int_t         RunNumber_value) { RunNumber += RunNumber_value; SetModified(true); }

   virtual void SetModified (Bool_t        modified ) { fModified  = modified; }

   virtual void ResetModified();

   void UpdateVariableSize();

   void SetAll( Int_t RunNumber_value=0,ROMEString StartTime_value="" );

   virtual void Reset();

   void Print(Option_t* option) const;

private:


   ClassDef(CYGODB,1)
};

#endif   // CYGODB_H
