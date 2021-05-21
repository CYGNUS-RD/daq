//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri May 21 14:15:16 2021 by ROOT version 6.22/03
// from TTree DataTree/
// found on file: DataTree00046.root
//////////////////////////////////////////////////////////

#ifndef DataTreeBaseClass_h
#define DataTreeBaseClass_h

#include <string>

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TH2F.h>
#include <TGraph.h>

// Header file for the classes stored in the TTree if any.
//#include "ROMETreeInfo.h"
#include "TObject.h"
#include "TClonesArray.h"
//#include "CYGLightCluster.h"
//#include "CYGRawOutput.h"
#include "TNamed.h"
#include "TAttLine.h"
#include "TAttFill.h"
#include "TAttMarker.h"
#include "TGraph.h"

class DataTreeBaseClass {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.
   static constexpr Int_t kMaxInfo = 1;
   static constexpr Int_t kMaxlightclusters = 1;
   static constexpr Int_t kMaxWF = 34;

   // Declaration of leaf types
 //ROMETreeInfo    *Info_;
   UInt_t          Info_TObject_fUniqueID;
   UInt_t          Info_TObject_fBits;
   Long64_t        Info_run;
   Long64_t        Info_event;
   Int_t           Info_time;
   Int_t           lightclusters_;
   UInt_t          lightclusters_fUniqueID[kMaxlightclusters];   //[lightclusters_]
   UInt_t          lightclusters_fBits[kMaxlightclusters];   //[lightclusters_]
   Int_t           lightclusters_nph[kMaxlightclusters];   //[lightclusters_]
 //CYGRawOutput    *rawoutputs;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   TH2F            CamPicture;
   Int_t           WF_;
   UInt_t          WF_fUniqueID[kMaxWF];   //[WF_]
   UInt_t          WF_fBits[kMaxWF];   //[WF_]
   TString         WF_fName[kMaxWF];
   TString         WF_fTitle[kMaxWF];
   Short_t         WF_fLineColor[kMaxWF];   //[WF_]
   Short_t         WF_fLineStyle[kMaxWF];   //[WF_]
   Short_t         WF_fLineWidth[kMaxWF];   //[WF_]
   Short_t         WF_fFillColor[kMaxWF];   //[WF_]
   Short_t         WF_fFillStyle[kMaxWF];   //[WF_]
   Short_t         WF_fMarkerColor[kMaxWF];   //[WF_]
   Short_t         WF_fMarkerStyle[kMaxWF];   //[WF_]
   Float_t         WF_fMarkerSize[kMaxWF];   //[WF_]
   Int_t           WF_fNpoints[kMaxWF];   //[WF_]
   Double_t       *WF_fX[kMaxWF];   //[WF_fNpoints]
   Double_t       *WF_fY[kMaxWF];   //[WF_fNpoints]
   Double_t        WF_fMinimum[kMaxWF];   //[WF_]
   Double_t        WF_fMaximum[kMaxWF];   //[WF_]

   // List of branches
   TBranch        *b_Info_TObject_fUniqueID;   //!
   TBranch        *b_Info_TObject_fBits;   //!
   TBranch        *b_Info_run;   //!
   TBranch        *b_Info_event;   //!
   TBranch        *b_Info_time;   //!
   TBranch        *b_lightclusters_;   //!
   TBranch        *b_lightclusters_fUniqueID;   //!
   TBranch        *b_lightclusters_fBits;   //!
   TBranch        *b_lightclusters_nph;   //!
   TBranch        *b_rawoutputs_fUniqueID;   //!
   TBranch        *b_rawoutputs_fBits;   //!
   TBranch        *b_rawoutputs_CamPicture;   //!
   TBranch        *b_rawoutputs_WF_;   //!
   TBranch        *b_WF_fUniqueID;   //!
   TBranch        *b_WF_fBits;   //!
   TBranch        *b_WF_fName;   //!
   TBranch        *b_WF_fTitle;   //!
   TBranch        *b_WF_fLineColor;   //!
   TBranch        *b_WF_fLineStyle;   //!
   TBranch        *b_WF_fLineWidth;   //!
   TBranch        *b_WF_fFillColor;   //!
   TBranch        *b_WF_fFillStyle;   //!
   TBranch        *b_WF_fMarkerColor;   //!
   TBranch        *b_WF_fMarkerStyle;   //!
   TBranch        *b_WF_fMarkerSize;   //!
   TBranch        *b_WF_fNpoints;   //!
   TBranch        *b_WF_fX;   //!
   TBranch        *b_WF_fY;   //!
   TBranch        *b_WF_fMinimum;   //!
   TBranch        *b_WF_fMaximum;   //!

  std::string m_outfile;
  
   DataTreeBaseClass(TTree *tree=0);
   virtual ~DataTreeBaseClass();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
   virtual void     SetOutputFile(std::string ofname) {m_outfile = ofname;}
};

#endif

#ifdef DataTreeBaseClass_cxx
DataTreeBaseClass::DataTreeBaseClass(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("DataTree00046.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("DataTree00046.root");
      }
      f->GetObject("DataTree",tree);

   }
   Init(tree);
}

DataTreeBaseClass::~DataTreeBaseClass()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t DataTreeBaseClass::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t DataTreeBaseClass::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void DataTreeBaseClass::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set array pointer
   for(int i=0; i<kMaxWF; ++i) WF_fX[i] = 0;
   for(int i=0; i<kMaxWF; ++i) WF_fY[i] = 0;

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("Info.TObject.fUniqueID", &Info_TObject_fUniqueID, &b_Info_TObject_fUniqueID);
   fChain->SetBranchAddress("Info.TObject.fBits", &Info_TObject_fBits, &b_Info_TObject_fBits);
   fChain->SetBranchAddress("Info.run", &Info_run, &b_Info_run);
   fChain->SetBranchAddress("Info.event", &Info_event, &b_Info_event);
   fChain->SetBranchAddress("Info.time", &Info_time, &b_Info_time);
   fChain->SetBranchAddress("lightclusters", &lightclusters_, &b_lightclusters_);
   fChain->SetBranchAddress("lightclusters.fUniqueID", &lightclusters_fUniqueID, &b_lightclusters_fUniqueID);
   fChain->SetBranchAddress("lightclusters.fBits", &lightclusters_fBits, &b_lightclusters_fBits);
   fChain->SetBranchAddress("lightclusters.nph", &lightclusters_nph, &b_lightclusters_nph);
   fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_rawoutputs_fUniqueID);
   fChain->SetBranchAddress("fBits", &fBits, &b_rawoutputs_fBits);
   fChain->SetBranchAddress("CamPicture", &CamPicture, &b_rawoutputs_CamPicture);
   fChain->SetBranchAddress("WF", &WF_, &b_rawoutputs_WF_);
   fChain->SetBranchAddress("WF.fUniqueID", WF_fUniqueID, &b_WF_fUniqueID);
   fChain->SetBranchAddress("WF.fBits", WF_fBits, &b_WF_fBits);
   fChain->SetBranchAddress("WF.fName", WF_fName, &b_WF_fName);
   fChain->SetBranchAddress("WF.fTitle", WF_fTitle, &b_WF_fTitle);
   fChain->SetBranchAddress("WF.fLineColor", WF_fLineColor, &b_WF_fLineColor);
   fChain->SetBranchAddress("WF.fLineStyle", WF_fLineStyle, &b_WF_fLineStyle);
   fChain->SetBranchAddress("WF.fLineWidth", WF_fLineWidth, &b_WF_fLineWidth);
   fChain->SetBranchAddress("WF.fFillColor", WF_fFillColor, &b_WF_fFillColor);
   fChain->SetBranchAddress("WF.fFillStyle", WF_fFillStyle, &b_WF_fFillStyle);
   fChain->SetBranchAddress("WF.fMarkerColor", WF_fMarkerColor, &b_WF_fMarkerColor);
   fChain->SetBranchAddress("WF.fMarkerStyle", WF_fMarkerStyle, &b_WF_fMarkerStyle);
   fChain->SetBranchAddress("WF.fMarkerSize", WF_fMarkerSize, &b_WF_fMarkerSize);
   fChain->SetBranchAddress("WF.fNpoints", WF_fNpoints, &b_WF_fNpoints);
   fChain->SetBranchAddress("WF.fX", WF_fX, &b_WF_fX);
   fChain->SetBranchAddress("WF.fY", WF_fY, &b_WF_fY);
   fChain->SetBranchAddress("WF.fMinimum", WF_fMinimum, &b_WF_fMinimum);
   fChain->SetBranchAddress("WF.fMaximum", WF_fMaximum, &b_WF_fMaximum);
   Notify();
}

Bool_t DataTreeBaseClass::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void DataTreeBaseClass::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t DataTreeBaseClass::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef DataTreeBaseClass_cxx