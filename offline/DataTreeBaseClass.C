#define DataTreeBaseClass_cxx
#include "DataTreeBaseClass.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

#include <iostream>

void DataTreeBaseClass::Loop()
{
//   In a ROOT session, you can do:
//      root> .L DataTreeBaseClass.C
//      root> DataTreeBaseClass t
//      root> t.GetEntry(12); // Fill t data members with entry number 12
//      root> t.Show();       // Show values of entry 12
//      root> t.Show(16);     // Read and show values of entry 16
//      root> t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
   if (fChain == 0) return;

   Long64_t nentries = fChain->GetEntriesFast();

   TFile *tfout = new TFile(m_outfile.c_str(),"recreate");

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
   
      
      std::cout << "Processing event # " << jentry << std::endl;
      
      char postfix[500];
      sprintf(postfix,"run%05d_ev%d",int(Info_run),int(Info_event));

      // get the TH2F of the camera picture
      CamPicture.GetZaxis()->SetRangeUser(99,105);
      CamPicture.SetName(Form("pic_%s",postfix));
      CamPicture.SetTitle(Form("Camera, timestamp %d",Info_time));

      // get the TGraph of the PMT waveforms
      int wavetosave=3;
      int position[wavetosave]={0 , 6 , 8 };

      TGraph **g=new TGraph*[wavetosave];
      double *xaxis;
      double *yaxis;
      int npoints;

      for(int i=0;i<wavetosave;i++){
	xaxis=WF_fX[position[i]];
	yaxis=WF_fY[position[i]];
	npoints=WF_fNpoints[position[i]];
	g[i]=new TGraph(npoints,xaxis,yaxis);
	g[i]->SetName(Form("wfm_%s_ch%d",postfix,position[i]));
	g[i]->SetTitle(Form("wfm, timestamp %d_ch%d",Info_time,position[i]));
	g[i]->GetXaxis()->SetTitle("Time [ns]");
	g[i]->GetYaxis()->SetTitle("Amplitude [mV]");
      }
   
      // write to the output ROOT file
      CamPicture.Write();
      for(int i=0;i<wavetosave;i++){
	g[i]->Write();
	delete g[i];
      }
      delete[] g;
   }
   tfout->Close();
}
