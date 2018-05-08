/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGLightCluster_H
#define CYGLightCluster_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGLightCluster                                                            //
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
class CYGLightCluster;
Bool_t operator==(const CYGLightCluster &lhs, const CYGLightCluster &rhs);
inline Bool_t operator!=(const CYGLightCluster &lhs, const CYGLightCluster &rhs) { return !(lhs == rhs); }

class CYGLightCluster : public TObject
{
protected:
   Int_t         nph;      // Number of photons
   Bool_t        fModified;  //! Modified Folder Flag

private:
   CYGLightCluster(const CYGLightCluster &c); // not implemented

public:
   CYGLightCluster(Int_t nph_value=0 );

   virtual ~CYGLightCluster();

   // Warning! All fields may not be compared.
   friend Bool_t operator==(const CYGLightCluster &lhs, const CYGLightCluster &rhs);
   friend Bool_t operator!=(const CYGLightCluster &lhs, const CYGLightCluster &rhs);

   // Warning! All fields may not be copied.
   CYGLightCluster& operator=(const CYGLightCluster &c);

   Int_t          Getnph() const      { return nph;      }

   virtual Bool_t         isModified();

   void Setnph     (Int_t         nph_value     ) { nph      = nph_value;      SetModified(true); }

   void Addnph     (Int_t         nph_value     ) { nph      += nph_value;      SetModified(true); }

   virtual void SetModified(Bool_t        modified) { fModified = modified;}

   virtual void ResetModified();

   void UpdateVariableSize();

   void SetAll( Int_t nph_value=0 );

   virtual void Reset();

   void Print(Option_t* option) const;

private:


   ClassDef(CYGLightCluster,1)
};

#endif   // CYGLightCluster_H
