/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGDBACCESS_H
#define CYGDBACCESS_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGDBAccess                                                                //
//                                                                            //
// Handles the database access for the CYGAnalyzer.                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include <TArrayI.h>
#include <TObject.h>
#include "ROMEString.h"

class CYGDBAccess : public TObject {  
private:
   ROMEString *fDBName[5];         //! Name of the database from which to read the field
   ROMEString *fDBPath[5];         //! Database path to the value of the field
   TArrayI   **fDBCode[5];         //! Object Interpreter codes for database path to the value of the field
   Int_t       fNumberOfValues[5]; //! Number of values in folder

private:
   CYGDBAccess(const CYGDBAccess &c); // not implemented
   CYGDBAccess &operator=(const CYGDBAccess &c); // not implemented

public:
   CYGDBAccess();
   virtual ~CYGDBAccess();

   const char* GetDBNameAt(Int_t folderIndex,Int_t valueIndex) const;
   const char* GetDBPathAt(Int_t folderIndex,Int_t valueIndex,ROMEString& path) const;
   const char* GetDBPathOriginalAt(Int_t folderIndex,Int_t valueIndex) const;
   Int_t       GetDBNumberOfCodeAt(Int_t folderIndex,Int_t valueIndex) const;
   Int_t       GetDBCodeAt(Int_t folderIndex,Int_t valueIndex,Int_t i) const;

   void SetDBNameAt(Int_t folderIndex,Int_t valueIndex,const char* name);
   void SetDBPathAt(Int_t folderIndex,Int_t valueIndex,const char* path);
   void SetDBNumberOfCodeAt(Int_t folderIndex,Int_t valueIndex,Int_t num);
   void SetDBCodeAt(Int_t folderIndex,Int_t valueIndex,Int_t i,Int_t code);

   Bool_t ReadODB();

   ClassDef(CYGDBAccess, 0)
};

#endif
