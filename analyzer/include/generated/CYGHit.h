/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGHit_H
#define CYGHit_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGHit                                                                     //
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
class CYGHit;
Bool_t operator==(const CYGHit &lhs, const CYGHit &rhs);
inline Bool_t operator!=(const CYGHit &lhs, const CYGHit &rhs) { return !(lhs == rhs); }

class CYGHit : public TObject
{
protected:
   Int_t         channel;  // channel number
   Int_t         time;     // hit time
   Bool_t        fModified;  //! Modified Folder Flag

private:
   CYGHit(const CYGHit &c); // not implemented

public:
   CYGHit(Int_t channel_value=0, Int_t time_value=0 );

   virtual ~CYGHit();

   // Warning! All fields may not be compared.
   friend Bool_t operator==(const CYGHit &lhs, const CYGHit &rhs);
   friend Bool_t operator!=(const CYGHit &lhs, const CYGHit &rhs);

   // Warning! All fields may not be copied.
   CYGHit& operator=(const CYGHit &c);

   Int_t          Getchannel() const  { return channel;  }
   Int_t          Gettime() const     { return time;     }

   virtual Bool_t         isModified();

   void Setchannel (Int_t         channel_value ) { channel  = channel_value;  SetModified(true); }
   void Settime    (Int_t         time_value    ) { time     = time_value;     SetModified(true); }

   void Addchannel (Int_t         channel_value ) { channel  += channel_value;  SetModified(true); }
   void Addtime    (Int_t         time_value    ) { time     += time_value;     SetModified(true); }

   virtual void SetModified(Bool_t        modified) { fModified = modified;}

   virtual void ResetModified();

   void UpdateVariableSize();

   void SetAll( Int_t channel_value=0,Int_t time_value=0 );

   virtual void Reset();

   void Print(Option_t* option) const;

private:


   ClassDef(CYGHit,1)
};

#endif   // CYGHit_H
