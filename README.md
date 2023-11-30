# daq
Code based on Midas and Rome for CYGNUS Data Acquisition

**WORK IN PROGRESS**

# screen connection to CAEN N1570
sudo screen /dev/ttyACM0 9600,cs8,-parenb,-cstopb

## To run the code that converts the DAQ (.mid) files to ROOT Trees:

mkdir results
./cyganalyzer.exe -i offlineConfig.xml -r 000029 -m offline -p 0

* where -r <run number> should correspond to a file run00029.mid.gz in the input directory)
* the input directory is for now hardcoded in offlineConfig.xml (<InputFilePath>...</InputFilePath>)
* the output will be in results/DataTree000029.root, and a TH2F (for the camera) and TGraph (for the PMT) will be saved as branch in the Tree


## To dump the code that only saves the TH2 of the Camera Picture and TGraph of the PMT
./PlainTObjectsDumper.py ~/Work/cygnus/daq/analyzer/results/DataTree00070.root [--max-entries=10] --outdir results

## Similarly for dumping hdf5 files:
./PlainHdf5Dumper.py ~/Work/cygnus/daq/analyzer/results/DataTree00070.root [--max-entries=10] --outdir results

* it saves a root/hdf5 file for each run

