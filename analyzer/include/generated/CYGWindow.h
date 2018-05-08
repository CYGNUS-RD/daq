/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGWINDOW_H
#define CYGWINDOW_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGWindow                                                                  //
//                                                                            //
// Main window class for the CYGAnalyzer.                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include "ArgusWindow.h"
class CYGTEventDisplay_Base;
class CYGTFeatures_Base;

class CYGWindow : public ArgusWindow {  
public:
   enum CommandIdentifiers{
      M_FILE_CONNECT_ROOT = 10
   };

   enum MenuEnumeration {
      M_ROOT = 10000
   };

private:
   CYGWindow(const CYGWindow &c); // not implemented
   CYGWindow &operator=(const CYGWindow &c); // not implemented

public:
   CYGWindow();
   CYGWindow(const TGWindow *p,Bool_t tabWindow=kTRUE);
   virtual ~CYGWindow() {}

   ArgusWindow *CreateSubWindow();
   void         ConstructTabs();
private:
   Bool_t       AddMenuNetFolder(TGPopupMenu* menu);
   Bool_t       ProcessMessageNetFolder(Long_t param1);
public:
   TGPopupMenu* GetMenuHandle(const char* menuName) const;

   // Tabs
   CYGTEventDisplay_Base* GetEventDisplayTab() const { return reinterpret_cast<CYGTEventDisplay_Base*>(GetTabObjectAt(0)); }
   CYGTFeatures_Base* GetFeaturesTab() const { return reinterpret_cast<CYGTFeatures_Base*>(GetTabObjectAt(1)); }

   ClassDef(CYGWindow, 0)
};

#endif
