/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGGlobalSteering_H
#define CYGGlobalSteering_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGGlobalSteering                                                          //
//                                                                            //
// Global steering parameter class for CYGNUS                                 //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include <Rtypes.h>
#include <TString.h>

class CYGGlobalSteering
{
public:
protected:

private:
   CYGGlobalSteering(const CYGGlobalSteering &c); // not implemented
   CYGGlobalSteering &operator=(const CYGGlobalSteering &c); // not implemented

public:
   CYGGlobalSteering()
   {
   }

   ~CYGGlobalSteering()
   {
   }

};

#endif   // CYGGlobalSteering_H
