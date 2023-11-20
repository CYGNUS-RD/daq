import os
import midas
import midas.client
import time
import subprocess
import select
import sys
"""
Output a list containing 
G3,T2,G2,T1,G1,D
"""
filename='/media/cygno01/LaCIA/HVGEMlogger/run.dat'
client = midas.client.MidasClient("db_display")
buffer_handle = client.open_event_buffer("SYSTEM",None,1000000000)
request_id = client.register_event_request(buffer_handle, sampling_type = 2)
temp_str="28       28       30       30       29       27       57"
while True:
    try:    
        last_line=os.popen("tail -n 1 "+filename).read()
        #print(len(last_line))
        if last_line==temp_str or len(last_line)!=136:
            continue
        else:      
            temp_str=last_line 
            output=last_line.split()[2:8]
            #print(output)
            for i in range(1,7):
                client.odb_set("/Equipment/HV/Variables/Demand["+str(i)+"]",output[i-1])
    except KeyboardInterrupt:
        client.deregister_event_request(buffer_handle, request_id)
        client.disconnect()
        print ("\nBye, bye Scemott* <3...")
        sys.exit()