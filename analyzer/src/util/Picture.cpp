#include "util/Picture.h"
#include "TImage.h"
#include "TAxis.h"
#include "TVirtualFFT.h"
#include "TMath.h"
#include <iostream>

using namespace std;

ClassImp(Picture)

Picture::Picture()
{

  fNpx = 0;
  fNpy = 0;
  fData = 0;
  fImage = 0;
  fHisto = 0;

  fLightHisto = new TH1F("hLight","Light Level",65536,-0.5,65535.5);
  
}

Picture::Picture(Int_t Npx, Int_t Npy)
{

  SetNPixels(Npx,Npy);
  
  fImage = 0;
  fHisto = 0;
  
  fLightHisto = new TH1F("hLight","Light Level",65536,-0.5,65535.5);

}

Picture::Picture(Int_t Npx, Int_t Npy, Double_t **data)
{

  SetNPixels(Npx,Npy);

  SetData(data);
  
  fImage = 0;
  fHisto = 0;
  
  fLightHisto = new TH1F("hLight","Light Level",65536,-0.5,65535.5);

}

Picture::~Picture()
{

  if(fData) delete [] fData;
  fData = 0;

  if(fImage) delete fImage;
  fImage = 0;

  if(fHisto) delete fHisto;
  fHisto = 0;

  if(fLightHisto) delete fLightHisto;
  fLightHisto = 0;
  
}

void Picture::SetNPixels(Int_t Npx, Int_t Npy)
{

  if(fData) delete [] fData;
  fData = 0;

  fNpx = Npx;
  fNpy = Npy;
  
  fData = new Double_t[Npx*Npy];
  
}

void Picture::SetData(Double_t* data)
{

  memcpy(fData,data,sizeof(Double_t)*fNpx*fNpy);

  for(Int_t i=0;i<fNpx*fNpy;i++) fLightHisto->Fill(data[i]);
  
}

void Picture::SetData(Double_t** data)
{

  for(Int_t px=0;px<fNpx;px++) {

    memcpy(&fData[px*fNpy],data[px],sizeof(Int_t)*fNpy);

    for(Int_t py=0;py<fNpy;py++) {
      fLightHisto->Fill(data[px][py]);
    }

  }
  
}

void Picture::SetDataAt(Int_t px, Int_t py, Double_t data)
{

  if(px > fNpx) { cout << "ERROR IN Picture.cpp -- px = " << px << " Npx = " << fNpx << endl; exit(1); }
  if(py > fNpy) { cout << "ERROR IN Picture.cpp -- py = " << py << " Npy = " << fNpy << endl; exit(1); }

  fData[px*fNpy + py] = data;

  fLightHisto->Fill(data);
  
}

TImage* Picture::GetImage(){

  if(!fData) { cout << "Data not set -- cannot build the image" << endl; return 0; }

  if(!fImage) fImage = TImage::Create();

  fImage->SetImage(fData,fNpx,fNpy);

  return fImage;
  
}

TH2F* Picture::GetHisto(){

  if(!fData) { cout << "Data not set -- cannot build the image" << endl; return 0; }

  if(!fHisto) fHisto = new TH2F("histo","histo",fNpx,-0.5,fNpx-0.5,fNpy,-0.5,fNpy-0.5);

  for(Int_t i=0;i<fNpx;i++){
    for(Int_t j=0;j<fNpy;j++){
      fHisto->SetBinContent(i+1,fNpx-j,fData[i*fNpy + j]);
    }
  }

  return fHisto;

}
