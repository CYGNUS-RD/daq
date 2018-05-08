#include "util/Waveform.h"
#include "TAxis.h"
#include "TVirtualFFT.h"
#include "TMath.h"
#include <iostream>

using namespace std;

ClassImp(Waveform)

Waveform::Waveform()
{

  fNPoints = 0;
  fTime = 0;
  fAmpl = 0;
  fGraph = 0;
  fHasSignal = false;
  fEdgeBin = -1;

}

Waveform::Waveform(Int_t npoints)
{

  fNPoints = npoints;
  fTime = new Double_t[npoints];
  fAmpl = new Double_t[npoints];

}

Waveform::~Waveform()
{

  if(fTime) delete [] fTime;
  fTime = 0;

  if(fAmpl) delete [] fAmpl;
  fAmpl = 0;

}

void Waveform::SetNPoints(Int_t npoints)
{

  fNPoints = npoints;

  delete [] fTime;
  delete [] fAmpl;

  fTime = new Double_t[npoints];
  fAmpl = new Double_t[npoints];

}

void Waveform::Set(Int_t npoints, Double_t *time, Double_t *ampl)
{

  fNPoints = npoints;

  delete [] fTime;
  delete [] fAmpl;

  fTime = new Double_t[npoints];
  fAmpl = new Double_t[npoints];

  for(Int_t i=0;i<fNPoints;i++){

    fTime[i] = time[i];
    fAmpl[i] = ampl[i];

  }

}

void Waveform::Add(Double_t *ampl)
{

  for(Int_t i=0;i<fNPoints;i++) fAmpl[i] += ampl[i];

}

TGraph *Waveform::GetGraph()
{ 

  if(!fTime || !fAmpl) { cout << "Waveform not set -- cannot build the graph" << endl; return 0; }

  if(!fGraph) fGraph = new TGraph(fNPoints);

  for(Int_t i=0;i<fNPoints;i++){

    fGraph->SetPoint(i,fTime[i],fAmpl[i]);

  }

  fGraph->GetXaxis()->SetRangeUser(fTime[0],fTime[fNPoints-1]);

  return fGraph;

}

void Waveform::GetSpectrum(Double_t *spectrum){

  Int_t n = GetNPoints();

  Double_t amplRe[(const Int_t)n], amplIm[(const Int_t)n];

  TVirtualFFT *fftS = TVirtualFFT::FFT(1, &n, "R2C ES");
  for(Int_t i=0;i<n;i++) fftS->SetPoint(i,GetAmplAt(i));
  fftS->Transform();
  fftS->GetPointsComplex(amplRe,amplIm);

  for(Int_t i=0;i<n/2;i++){

    spectrum[i] = amplRe[i]*amplRe[i] + amplIm[i]*amplIm[i];

  }

}

Double_t Waveform::Baseline()
{

  Double_t mean = 0.;

  for(Int_t j=10;j<110;j++){
    
    mean += fAmpl[j];

  }

  mean /= 100.;

  return mean;

}

Double_t Waveform::Integral(Int_t edge, Double_t tpre, Double_t tpost)
{

  Int_t start = TMath::Nint(edge - tpre/(fTime[edge+1] - fTime[edge]));
  Int_t stop = TMath::Nint(edge + tpost/(fTime[edge+1] - fTime[edge]));

  Double_t baseline = Baseline();

  if(start < 0) start = 0;
  if(stop > fNPoints-2) stop = fNPoints;

  Double_t sum = 0.;

  for(Int_t is=start;is<stop;is++){

    sum += (fAmpl[is]-baseline)*(fTime[is+1] - fTime[is]);

  }

  return sum;

}
