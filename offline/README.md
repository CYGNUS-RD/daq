## To dump the code that only saves the TH2 of the Camera Picture and TGraph of the PMT
./PlainTObjectsDumper.py ~/Work/cygnus/daq/analyzer/results/DataTree00070.root [--max-entries=10] --outdir results

## Similarly for dumping hdf5 files:
./PlainHdf5Dumper.py ~/Work/cygnus/daq/analyzer/results/DataTree00070.root [--max-entries=10] --outdir results

* it saves a root/hdf5 file for each run