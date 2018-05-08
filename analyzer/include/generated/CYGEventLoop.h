/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGEventLoop_H
#define CYGEventLoop_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGEventLoop                                                               //
//                                                                            //
// Framework specific event loop class for CYGAnalyzer.                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include "ROMEEventLoop.h"

class ROMEString;


class CYGEventLoop : public ROMEEventLoop
{

private:
   CYGEventLoop(const CYGEventLoop &c); // not implemented
   CYGEventLoop &operator=(const CYGEventLoop &c); // not implemented

public:
   CYGEventLoop(const char *name,const char *title);
   virtual ~CYGEventLoop() {}
   void AddTreeBranches();

protected:
   // Folder Methodes
   void InitArrayFolders();
   void ResetFolders();
   void CleanUpFolders();

   // Tree Methodes
   void InitTrees();
   void FillTrees();
   void GetTreeFileName(ROMEString& buffer,Int_t treeIndex, Bool_t inputFile = kFALSE) const;
   void WriteRunHeaders();
   void ReadRunHeaders();

   // Hot Links
   void InitHotLinks();
   void UpdateHotLinks();

   void ResetStatistics();

   ClassDef(CYGEventLoop, 0)
};

#endif   // CYGEventLoop_H
