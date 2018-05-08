/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGAnalyzerConfigParameters_H
#define CYGAnalyzerConfigParameters_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGAnalyzerConfigParameters                                                //
//                                                                            //
// folder for configuration parameters                                        //
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
class CYGAnalyzerConfigParameters;
Bool_t operator==(const CYGAnalyzerConfigParameters &lhs, const CYGAnalyzerConfigParameters &rhs);
inline Bool_t operator!=(const CYGAnalyzerConfigParameters &lhs, const CYGAnalyzerConfigParameters &rhs) { return !(lhs == rhs); }

class CYGAnalyzerConfigParameters : public TObject
{
protected:
   TString       ConfigFileName; // File name of configuration file
   TString       ConfigString;   // Content of configuration file
   Bool_t        fModified;      //! Modified Folder Flag

private:
   CYGAnalyzerConfigParameters(const CYGAnalyzerConfigParameters &c); // not implemented

public:
   CYGAnalyzerConfigParameters(TString ConfigFileName_value="", TString ConfigString_value="" );

   virtual ~CYGAnalyzerConfigParameters();

   // Warning! All fields may not be compared.
   friend Bool_t operator==(const CYGAnalyzerConfigParameters &lhs, const CYGAnalyzerConfigParameters &rhs);
   friend Bool_t operator!=(const CYGAnalyzerConfigParameters &lhs, const CYGAnalyzerConfigParameters &rhs);

   // Warning! All fields may not be copied.
   CYGAnalyzerConfigParameters& operator=(const CYGAnalyzerConfigParameters &c);

   TString        GetConfigFileName() const { return ConfigFileName; }
   TString        GetConfigString() const   { return ConfigString;   }

   virtual Bool_t         isModified();

   void SetConfigFileName(TString       ConfigFileName_value) { ConfigFileName = ConfigFileName_value; SetModified(true); }
   void SetConfigString  (TString       ConfigString_value  ) { ConfigString   = ConfigString_value;   SetModified(true); }


   virtual void SetModified      (Bool_t        modified      ) { fModified       = modified;      }

   virtual void ResetModified();

   void UpdateVariableSize();

   void SetAll( TString ConfigFileName_value="",TString ConfigString_value="" );

   virtual void Reset();

   void Print(Option_t* option) const;

private:


   ClassDef(CYGAnalyzerConfigParameters,1)
};

#endif   // CYGAnalyzerConfigParameters_H
