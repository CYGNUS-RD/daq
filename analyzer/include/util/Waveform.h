#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <TObject.h> 
#include <TGraph.h> 

class Waveform : public TObject {

 protected:

  Int_t fNPoints;
  Double_t *fTime; //[fNPoints]
  Double_t *fAmpl; //[fNPoints]
  TGraph *fGraph;
  Int_t fEdgeBin;
  Bool_t fHasSignal;

 public:

  Waveform();
  Waveform(Int_t npoints);
  virtual ~Waveform();

  void SetNPoints(Int_t npoints);
  Double_t *GetTime() { return fTime; }
  Double_t *GetAmpl() { return fAmpl; }
  Double_t GetTimeAt(Int_t i) { return fTime[i]; }
  Double_t GetAmplAt(Int_t i) { return fAmpl[i]; }
  Int_t GetNPoints() { return fNPoints; }
  void Set(Int_t npoints, Double_t *time, Double_t *ampl);
  void Add(Double_t *ampl);
  TGraph *GetGraph();
  void GetSpectrum(Double_t *spectrum);

  Bool_t HasSignal() { return fHasSignal; }
  Double_t GetEdgeTime() { return fTime[fEdgeBin]; }

  void SetSignal(Int_t edge_bin) { fHasSignal = true; fEdgeBin = edge_bin; }
  void RemoveSignal() { fHasSignal = false; fEdgeBin = -1; }

  Double_t Baseline();
  Double_t Integral(Int_t edge, Double_t tpre, Double_t tpost);

  ClassDef(Waveform,1) //Waveform class
  
};

#endif 
