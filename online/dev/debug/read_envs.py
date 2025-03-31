#!/usr/bin/python3
import argparse
from xmlrpc.client import ServerProxy
import time
import utilities as ut  # Assuming this contains encodeArd and decodeArd functions
from datetime import datetime as dt
import csv

log_path = '/home/cygno01/daq/online/ArduinoServerManager/log.txt'

def main():
    parser = argparse.ArgumentParser(description='Set the source position after calibrating')
    parser.add_argument("-v", "--verbose", help='Additional check and printout', action='store', type=int, default=None)
    parser.add_argument("-m", "--midas", help='Enable the midas mode', action='store', type=int, default=None)
    args = parser.parse_args()

    port = 8000
    server = ServerProxy(f"http://localhost:{port}")

    name="KEG"
    try:
        if args.verbose is not None:
            # Use the prefixed method name for getting port information
            ser_port = server.arduino_get_port(name)
            print(f"Info for {name}: Port={ser_port}")
        ut.log(f"Reading {name} variables...")
        KEG_env=ut.WriteAndRead(server, name, "R",verbose=args.verbose)[0]
        time.sleep(1)        
        position=ut.WriteAndRead(server, name, "G",verbose=args.verbose)[0]
        parts = position.split(':')
        source_pos = parts[1].strip()
        time.sleep(1)        
    except Exception as e:
        ut.log(f"Error interacting with {name}: {e}",log_path)
        if args.verbose is not None: print(f"Error interacting with {name}: {e}")
    name="MANGOlino"
    try:
        if args.verbose is not None:
            # Use the prefixed method name for getting port information
            ser_port = server.arduino_get_port(name)
            print(f"Info for {name}: Port={ser_port}")
        ut.log(f"Reading {name} variables...")
        MANGO_env=ut.WriteAndRead(server, name, "R",verbose=args.verbose)[0]
        time.sleep(1)              
    except Exception as e:
        ut.log(f"Error interacting with {name}: {e}",log_path)
        if args.verbose is not None: print(f"Error interacting with {name}: {e}")
        
    	
    KEG_env_vars = KEG_env.split(";")
    KEG_dict = {}
    KEG_dict["Temp"] = float(KEG_env_vars[0])
    KEG_dict["Pres"] = float(KEG_env_vars[1])/100000.
    KEG_dict["Humi"] = float(KEG_env_vars[2])
    KEG_dict["Posi"] = float(source_pos)
    print(KEG_dict)
    
    MANGO_env_vars = MANGO_env.split(";")
    MANGO_dict = {}
    MANGO_dict["Temp"] = float(MANGO_env_vars[0])
    MANGO_dict["Pres"] = float(MANGO_env_vars[1])/100000.
    MANGO_dict["Humi"] = float(MANGO_env_vars[2])
    print(MANGO_dict)

    now=dt.now().strftime("%d/%m/%Y_%H-%M-%S")
    if args.midas is None: print(f"Timestamp: {now} KEG: {KEG_env};{source_pos} - MANGOlino: {MANGO_env}")
    else: 
        line=str(now)+";"+str(KEG_env)+";"+str(source_pos)+";"+str(MANGO_env)
        #print(line)
        with open('/home/software/daq/online/ArduinoServerManager/env_log.csv', 'a') as file:
            file.write(line+"\n")
if __name__ == "__main__":
    main()
