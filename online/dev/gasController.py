import sys, os
import numpy as np
import re
import time
import datetime
import midas
import midas.client
from asyncua import Client
import asyncio

def getRawSCS3000(IP = "192.168.99.15", ADDRESS = "65535"):
    ret = os.popen("msc -d "+IP+" -a "+ADDRESS+" -c 'read'").read()
    readout = np.array(re.findall("([0-9.]+[0-9]+) [mv]", ret)).astype(float)
    if len(readout) != 24:
        return int(-24)
    else:
        return readout

    
def getVarsFromRaw(readout, verbose = False):
    
    ## p_atm = P0IIn3 = readout[3]
    p_atm_raw = readout[3]
    ## p_ext = P0IIn5 = readout5]
    p_ext_raw = readout[5]
    ## T_gas = P0IIn0 = readout[0]
    T_gas_raw = readout[0]
    ## T_atm = P1UIn1 = readout[9]
    T_atm_raw = readout[9]
    
    p_atm_bar = (p_atm_raw-4)/16*1.6*1.007618
    p_ext_bar = (p_ext_raw-4)/16*1.6*1.005454
    T_gas_C   = (T_gas_raw-4)/16*200-50
    T_atm_C   = -22.7*T_atm_raw+82.3
    
    if verbose:
        print("p_atm = {0:.5f} bar".format(p_atm_bar))
        print("p_ext = {0:.5f} bar".format(p_ext_bar))
        print("T_gas = {0:.2f} C".format(T_gas_C))
        print("T_atm = {0:.2f} C".format(T_atm_C))
    
    return p_atm_bar, p_ext_bar, T_gas_C, T_atm_C

def getSetPoint(p_atm_bar, lower_dp = 0.004, upper_dp = 0.008):
    setpoint = round((p_atm_bar + (lower_dp + upper_dp)/2.) * 1000.0) / 1000.
    return setpoint

async def readSetPointFromGasSystem():
    async with Client(url='opc.tcp://192.168.99.20:4870') as client:
        node = client.get_node("ns=3;s=DB_PAR_PID_Camera_SP")
        setpoint = await node.read_value()
    return setpoint

async def setSetPointFromGasSystem(setpoint):
    async with Client(url='opc.tcp://192.168.99.20:4870') as client:
        node = client.get_node("ns=3;s=DB_PAR_PID_Camera_SP")
        val = await node.write_value(setpoint)

def writeTimeStamp():
    with open('/home/standard/daq/online/dev/tmp_gascontroller.txt', 'w') as f:
        f.write(str(datetime.datetime.now()))

def writePatm(patm):
    with open('/home/standard/daq/online/dev/tmp_history_patm_std.txt', 'a') as f:
        f.write('{0:.3e}\n'.format(patm))

if __name__ == "__main__":
    
    lower_dp     = 0.003
    upper_dp     = 0.007
    
    #cache_p_atm = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    
    try:
        while True:
            # Read setpoint from gas system
            setpoint_old = asyncio.run(readSetPointFromGasSystem())
            
            # Readout SCS3000...
            print("Readout SCS3000 [Time", datetime.datetime.now(),"] ...", flush = True)
            readout = getRawSCS3000()
            while isinstance(readout, int):
                readout = getRawSCS3000()
            p_atm, p_ext, T_gas, T_atm = getVarsFromRaw(readout, verbose = True)
            print("SetPoint at {0:.3f} bar".format(setpoint_old), flush = True)
            
            # Check if setpoint in range, otherwise recompute it and set it in the gas system
            if setpoint_old < p_atm + lower_dp or setpoint_old > p_atm + upper_dp:
                setpoint_new = getSetPoint(p_atm, lower_dp = lower_dp, upper_dp = upper_dp)
                print("NEW SetPoint at {0:.3f} bar".format(setpoint_new), flush = True)
                asyncio.run(setSetPointFromGasSystem(setpoint_new))

            writeTimeStamp()
            #cache_p_atm.pop(0)
            #cache_p_atm.append(p_atm)
            #writePatm(np.std(cache_p_atm))
            
            time.sleep(60)
            
    except KeyboardInterrupt:
        print ("\nBye, bye...")
        sys.exit()
    
