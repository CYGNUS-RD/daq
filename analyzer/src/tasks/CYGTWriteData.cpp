////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGTWriteData                                                              //
//                                                                            //
// Begin_Html <!--
/*-->

<!--*/
// --> End_Html
//                                                                            //
//                                                                            //
// Please note: The following information is only correct after executing     //
// the ROMEBuilder.                                                           //
//                                                                            //
// This task accesses the following folders :                                 //
//     Event                                                                  //
//                                                                            //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/* Generated header file containing necessary includes                        */
#include "generated/CYGTWriteDataGeneratedIncludes.h"

////////////////////////////////////////////////////////////////////////////////
/*  This header was generated by ROMEBuilder. Manual changes above the        *
 * following line will be lost next time ROMEBuilder is executed.             */
/////////////////////////////////////----///////////////////////////////////////

#include "generated/CYGAnalyzer.h"
#include "tasks/CYGTWriteData.h"
#include "ROMEiostream.h"

// uncomment if you want to include headers of all folders
//#include "CYGAllFolders.h"


ClassImp(CYGTWriteData)

//______________________________________________________________________________
void CYGTWriteData::Init()
{
}

//______________________________________________________________________________
void CYGTWriteData::BeginOfRun()
{
}

//______________________________________________________________________________
void CYGTWriteData::Event()
{

  CYGEvent *event = gAnalyzer->GetEvent();
  
  if(event->GetCamPictureSize() > 0){

    Picture *pic = event->GetCamPictureAt(0);

    Waveform *wf0 = event->GetDGTZWaveformAt(0);

    Waveform *wf1 = event->GetDGTZWaveformAt(1);

    ///Write to file
    
  }

}

//______________________________________________________________________________
void CYGTWriteData::EndOfRun()
{
}

//______________________________________________________________________________
void CYGTWriteData::Terminate()
{
}
