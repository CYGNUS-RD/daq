/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGTFeatures_Base_H
#define CYGTFeatures_Base_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGTFeatures_Base                                                          //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include <RConfig.h>
#include <TGMenu.h>
#include <TThread.h>
#include <TRootEmbeddedCanvas.h>
#include <TPad.h>
#include "ArgusTab.h"
#include "ROMEiostream.h"

class CYGWindow;

struct FeaturesArgs{
   void*  inst;
   Long_t msg;
   Long_t param1;
   Long_t param2;
};

class CYGTFeatures_Base : public ArgusTab
{
protected:

private:
   CYGTFeatures_Base(const CYGTFeatures_Base &c); // not implemented
   CYGTFeatures_Base &operator=(const CYGTFeatures_Base &c); // not implemented

public:
   CYGTFeatures_Base(CYGWindow* window);
   virtual ~CYGTFeatures_Base();

   void BaseInit();
   virtual void Init() = 0;
   void EndInit() {}

   void BaseDisplay(bool processEvents=true);

   void BaseEventHandler();

   virtual void EventHandler() = 0;

   virtual Bool_t ProcessMessage(Long_t msg, Long_t param1, Long_t param2);

   virtual Bool_t ProcessMessageThread(Long_t /*msg*/, Long_t /*param1*/, Long_t /*param2*/) {return kTRUE;}

   static void ThreadProcessMessageThread(void* arg);

   Bool_t RunProcessMessageThread(Long_t msg, Long_t param1, Long_t param2);

   void RegisterObjects();
   void UnRegisterObjects();
   // User Methods
   void         BaseTabSelected();
   virtual void TabSelected() = 0;
   void         BaseTabUnSelected();
   virtual void TabUnSelected() = 0;

   void         BaseMenuClicked(TGPopupMenu *menu,Long_t param);
   virtual void MenuClicked(TGPopupMenu *menu,Long_t param) = 0;


   ClassDef(CYGTFeatures_Base,0) // Base class of CYGTFeatures
};

#endif   // CYGTFeatures_Base_H
