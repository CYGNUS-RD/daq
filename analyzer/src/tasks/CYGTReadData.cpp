////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGTReadData                                                               //
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
//     Hit                                                                    //
//                                                                            //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/* Generated header file containing necessary includes                        */
#include "generated/CYGTReadDataGeneratedIncludes.h"

////////////////////////////////////////////////////////////////////////////////
/*  This header was generated by ROMEBuilder. Manual changes above the        *
 * following line will be lost next time ROMEBuilder is executed.             */
/////////////////////////////////////----///////////////////////////////////////

#include "generated/CYGAnalyzer.h"
#include "generated/CYGMidasDAQ.h"
#include "generated/CYGEvent.h"
#include "tasks/CYGTReadData.h"
#include "ROMEiostream.h"

using namespace std;

// uncomment if you want to include headers of all folders
//#include "CYGAllFolders.h"
Int_t NPX = 2048;
Int_t NPY = 2048;

ClassImp(CYGTReadData)

//______________________________________________________________________________
void CYGTReadData::Init()
{
}

//______________________________________________________________________________
void CYGTReadData::BeginOfRun()
{
}

//______________________________________________________________________________
void CYGTReadData::Event()
{

  DWORD data;
  CYGEvent *event = gAnalyzer->GetEvent();
  
  //////READ TDC DATA
  int channels[gAnalyzer->GetMidasDAQ()->GetTDC0BankEntries()];
  int times[gAnalyzer->GetMidasDAQ()->GetTDC0BankEntries()];
  
  int nHit=0;

  for(Int_t it=0;it<gAnalyzer->GetMidasDAQ()->GetTDC0BankEntries();it++){

    data = gAnalyzer->GetMidasDAQ()->GetTDC0BankAt(it);
    
    if((data & 0xf8000000) != 0x00000000) continue;
    
    int chan = 0x7F&(data>>19);
    int time = 0x7FFFF&data;
    cout<<"channel# "<<chan << "  " << time <<endl;

    times[nHit]=time;
    channels[nHit]=chan;
    nHit++;

  }

  gAnalyzer->SetHitSize(0);  
  gAnalyzer->SetHitSize(nHit);  
  
  //FILL HITS
  for(Int_t it=0; it<nHit;it++){
    
    gAnalyzer->GetHitAt(it)->Settime(times[it]);
    gAnalyzer->GetHitAt(it)->Setchannel(channels[it]);

  }

  //////DIGITIZER
  event->SetDGTZWaveformSize(0);
  event->SetDGTZWaveformSize(2);

  WORD wdata;

  int ndgtz = 100000;
  double offset[2] = {950.,200.};
  
  for(Int_t k=0;k<2;k++){

    Waveform *wfdgtz = event->GetDGTZWaveformAt(k);
    wfdgtz->RemoveSignal();

    Double_t tmpv[100000], tmpt[100000];
    for(Int_t j=0;j<100000;j++){

      if(k==0) wdata = gAnalyzer->GetMidasDAQ()->GetDIG0BankAt(j);
      else  wdata = gAnalyzer->GetMidasDAQ()->GetDIG1BankAt(j);

      tmpt[j] = j*0.25;
      tmpv[j] = ((uint16_t)wdata)/1024.*1000.-offset[k];

      //if(k==0) tmpv[j] *= -1.;
      
    }
    
    wfdgtz->SetNPoints(100000);
    wfdgtz->Set(100000,tmpt,tmpv);

  }

  //////READ CAMERA DATA
  WORD camdata = 0;

  event->SetCamPictureSize(0);

  event->SetCamPictureSize(1);
  event->GetCamPictureAt(0)->SetNPixels(NPX,NPY);    


  //FULL PICTURE
  Int_t ip=0;
  for(Int_t iy=0;iy<NPY;iy++){

    for(Int_t ix=0;ix<NPX;ix++){

      camdata = gAnalyzer->GetMidasDAQ()->GetCAM0BankAt(ip);

      Double_t pixel_data = camdata + 0.0;

      event->GetCamPictureAt(0)->SetDataAt(ix,iy,pixel_data);
      
      ip++;
      
    }

  }

  /*
  ///SPARSE
  for(Int_t ip=0;ip<gAnalyzer->GetMidasDAQ()->GetCAM0BankEntries();ip+=3){

    UInt_t ix = (UInt_t)(gAnalyzer->GetMidasDAQ()->GetCAM0BankAt(ip));
    UInt_t iy = (UInt_t)(gAnalyzer->GetMidasDAQ()->GetCAM0BankAt(ip+1));
    camdata = gAnalyzer->GetMidasDAQ()->GetCAM0BankAt(ip+2);

    Double_t pixel_data = camdata + 0.0;

    event->GetCamPictureAt(0)->SetDataAt(ix,iy,pixel_data);
    
  }
  */
  ofstream outfile("tmp.dat");

  Picture *pic = event->GetCamPictureAt(0);
  for(Int_t ix=0;ix<NPX;ix++){
    for(Int_t iy=0;iy<NPY;iy++){
      outfile << (int)(pic->GetDataAt(ix,iy)) << "  " ;
    }
    outfile << endl;
  }
  
}

//______________________________________________________________________________
void CYGTReadData::EndOfRun()
{
}

//______________________________________________________________________________
void CYGTReadData::Terminate()
{
}

