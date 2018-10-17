#ifndef PICTURE_H
#define PICTURE_H

#include <TObject.h> 
#include <TGraph.h> 
#include <TH2F.h> 

class TImage;

class Picture : public TObject {

 protected:

  Int_t fNpx;
  Int_t fNpy;
  Double_t* fData;
  TImage* fImage;
  TH2F* fHisto;
  TH1F *fLightHisto;
  
 public:

  Picture();
  Picture(Int_t Npx, Int_t Npy);
  Picture(Int_t Npx, Int_t Npy, Double_t **Data);
  virtual ~Picture();

  void SetNPixels(Int_t Npx, Int_t Npy);
  void SetData(Double_t** data);
  void SetData(Double_t* data);
  void SetDataAt(Int_t px, Int_t py, Double_t data);
  Double_t GetDataAt(Int_t px, Int_t py) { return fData[px*fNpy + py]; }

  TImage* GetImage();
  TH2F* GetHisto();

  TH1F* GetLightHisto() { return fLightHisto; }
  
  ClassDef(Picture,1) //Picture class
  
};

#endif 
