/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGConfigToForm_H
#define CYGConfigToForm_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGConfigToForm                                                            //
//                                                                            //
// This class makes form of configuration for CYGAnalyzer.                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include "ROMEHisto.h"
#include "ROMEGraph.h"
#include "ROMEConfigToForm.h"

class CYGConfigToForm : public ROMEConfigToForm
{

private:
   CYGConfigToForm(const CYGConfigToForm &c); // not implemented
   CYGConfigToForm &operator=(const CYGConfigToForm &c); // not implemented

public:
   CYGConfigToForm();
   virtual ~CYGConfigToForm() {}

   void AddConfig(XMLToFormFrame *frame);
   void AddHisto(XMLToFormFrame *frame,ROMEHisto* histo,Int_t histoDimension);
   void AddGraph(XMLToFormFrame *frame,ROMEGraph* graph);
   void AddModesFrame(XMLToFormFrame *frame);
   void AddOfflineFrame(XMLToFormFrame *frame);
   void AddOnlineFrame(XMLToFormFrame *frame);
   void AddPathsFrame(XMLToFormFrame *frame);
   void AddCommonFrame(XMLToFormFrame *frame);
   void AddSettingsFrame(XMLToFormFrame *frame);
   void AddFloatingPointExceptionTrapFrame(XMLToFormFrame *frame);
   void AddSocketServerFrame(XMLToFormFrame *frame);
   void AddDataBaseFrame(XMLToFormFrame *frame);
   void AddFoldersFrame(XMLToFormFrame *frame);
   void AddODBFrame(XMLToFormFrame *frame);
   void AddTreesFrame(XMLToFormFrame *frame);
   void AddGlobalSteeringParametersFrame(XMLToFormFrame *frame);
   void AddAnalyzerFrame(XMLToFormFrame *frame);
   void AddMonitorFrame(XMLToFormFrame *frame);
   void AddMidasFrame(XMLToFormFrame *frame);

   ClassDef(CYGConfigToForm, 0)
};

#endif   // CYGConfigToForm_H
