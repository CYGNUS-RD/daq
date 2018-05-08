/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGRomeDAQ_H
#define CYGRomeDAQ_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGRomeDAQ                                                                 //
//                                                                            //
// This class implements the ROOT TTree access for CYGAnalyzer.               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include "ROMERomeDAQ.h"
#include "ROMETree.h"
class TClonesArray;

class CYGRomeDAQ : public ROMERomeDAQ
{

private:
   CYGRomeDAQ(const CYGRomeDAQ &c); // not implemented
   CYGRomeDAQ &operator=(const CYGRomeDAQ &c); // not implemented

public:
   CYGRomeDAQ();
   virtual ~CYGRomeDAQ() {}
   void ReadRunHeaders();
   void UpdateConfigParameters();
   TFile* GetDataTreeFile() const { return fROMETrees ? static_cast<ROMETree*>(fROMETrees->At(0))->GetFile() : 0; }
protected:
   void ConnectTrees();
   void UpdateVariableSize(Int_t treeNum);

   ClassDef(CYGRomeDAQ, 0)
};

#endif   // CYGRomeDAQ_H
