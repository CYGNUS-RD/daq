# daq
WORK IN PROGRESS

Code based on Midas for CYGNUS Data Acquisition and used on the MANGO setup. This repository will be used as a developing site for the DAQ upgrade foreseen for CYGNO_04

# Compilation

Installation by:
```
cd online
cmake build .
make
```

# Available acquisition modes

- Mode 0: standard acquisition mode
- Mode 3: CYGNO_04 acquisition mode: continous imaging, one or more synchronized cameras

## Usage of ODB with multiple cameras

When using multiple cameras, the user should setup the `/Equipment/Trigger/Settings/` entries in the ODB:
- `nCamera`: number of connected cameras (meaning the number of cameras that are ON and connected to the DAQ server via USB)
- `CameraSN`: this is an array of 5-digit strings containing the serial numbers of the cameras. The length of the array should be as large as the precompiler defined variable `NCAM_MAX` (default is 6, the variable is defined in the `online/cygnus_fe.cxx` file). At least for this version of the DAQ code, the order matters: the code will try to establish a connection with the cameras having the serial number of the first `nCamera` entries of the `CameraSN` array.
- `CameraMask`: mask containing which cameras are going to be used for acquisition. Note that you can acquire either all the cameras connected to DAQ (meaning the first `nCamera` entries of the `CameraMask` array are `'y'`), or a subset of those. All the flags of entries greater of `nCamera` will be ignored.


  
