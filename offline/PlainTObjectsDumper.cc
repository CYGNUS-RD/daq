#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TGraph.h>
#include <TH2F.h>

#include <iostream>
#include <string>

#include "DataTreeBaseClass.h"

using namespace std;

int main(int argc, char* argv[]) {

  char inputFileName[500];
  if ( argc < 2 ){
    std::cout << "missing argument: insert at least inputFile with the name of the DataTree file to convert" << std::endl;
    return 1;
  }
  strcpy(inputFileName,argv[1]);

  TFile *tf = TFile::Open(inputFileName);
  TTree *tree = (TTree*)tf->Get("DataTree");

  // form the output filename
  string infile(inputFileName);
  string ext = ".root";
  string delimiter = "0";
  
  std::string barename = infile.substr(0, infile.find(ext));
  std::string runNumber = barename.substr(barename.find(delimiter)+1,barename.length());

  TString outputFileName = TString::Format("histograms_Run0%s.root",runNumber.c_str());

  DataTreeBaseClass dt(tree);
  dt.SetOutputFile(string(outputFileName.Data()));
  dt.Loop();

  std::cout << "DATA CONVERTED IN PLAIN ROOT INTO " << outputFileName.Data() << std::endl;
  tf->Close();
  
}
