# daq
Code based on Midas and Rome for CYGNUS Data Acquisition

## To run the code that converts the DAQ (.mid) files to ROOT Trees:

mkdir results
./cyganalyzer.exe -i offlineConfig.xml -r 000029 -m offline -p 0

* where -r <run number> should correspond to a file run00029.mid.gz in the input directory)
* the input directory is for now hardcoded in offlineConfig.xml (<InputFilePath>...</InputFilePath>)
* the output will be in results/DataTree000029.root, and a TH2F (for the camera) and TGraph (for the PMT) will be saved as branch in the Tree
