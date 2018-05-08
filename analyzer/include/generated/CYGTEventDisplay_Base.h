/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGTEventDisplay_Base_H
#define CYGTEventDisplay_Base_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGTEventDisplay_Base                                                      //
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

struct EventDisplayArgs{
   void*  inst;
   Long_t msg;
   Long_t param1;
   Long_t param2;
};

class CYGTEventDisplay_Base : public ArgusTab
{
protected:

private:
   CYGTEventDisplay_Base(const CYGTEventDisplay_Base &c); // not implemented
   CYGTEventDisplay_Base &operator=(const CYGTEventDisplay_Base &c); // not implemented

public:
   CYGTEventDisplay_Base(CYGWindow* window);
   virtual ~CYGTEventDisplay_Base();

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


   ClassDef(CYGTEventDisplay_Base,0) // Base class of CYGTEventDisplay
};

#endif   // CYGTEventDisplay_Base_H
