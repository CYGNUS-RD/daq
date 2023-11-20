#!/usr/bin/env python3
#
# I. Abritta and G. Mazzitelli March 2022
# Middelware online recostruction 
# Modify by ... in date ...
#

import os, sys

# Load globals env
DAQ_ROOT            = os.environ['DAQ_ROOT']
DEFAULT_PATH_ONLINE = DAQ_ROOT+'/online/'
TAG                 = os.environ['TAG']

def image_jpg(image, vmin, vmax, event_number, event_time):
    from matplotlib import pyplot as plt
    im = plt.imshow(image, cmap='gray', vmin=vmin, vmax=vmax)
    plt.title ("Event: {:d} at {:s}".format(event_number, event_time))
    plt.savefig(DEFAULT_PATH_ONLINE+'custom/tmp.png')
    plt.close()
    del image
    return 

def on_success(metadata):
    print(f"Message produced to topic '{metadata.topic}' at offset {metadata.offset}")

def on_error(e):
    print(f"Error sending message: {e}")

def main(sendodb, sendevent, verbose=True):
    import numpy as np

    from datetime import datetime

    import time
    import base64
    import json

    import midas
    import midas.client

    import mysql.connector

    import cygno as cy
    import io

    from json import dumps
    from kafka import KafkaProducer
    import subprocess
    if sendodb:
        producer = KafkaProducer(
            bootstrap_servers=['localhost:9092'],
            value_serializer=lambda x: dumps(x).encode('utf-8')
        )
    if sendevent:
        producerb = KafkaProducer(
            bootstrap_servers = "localhost:9092",
            max_request_size  = 31457280,
#             compression_type  = 'gzip',
            max_block_ms      = 300000
        )
    topic1 = 'midas-odb-'+TAG
    topic2 = 'midas-event-'+TAG
    
    client = midas.client.MidasClient("middleware")
    buffer_handle = client.open_event_buffer("SYSTEM",None,1000000000)
    request_id = client.register_event_request(buffer_handle, sampling_type = 2) 

    # init program variables #####
    vmin         = 95
    vmax         = 130
    odb_update   = 3 # probabilemnte da mettere in middleware 
    event_info   = {}
    end1 = time.time()

    while True:
        start1 = time.time()
        if (start1-end1)>odb_update and sendodb:
            # update ODB
            odb_json = dumps(client.odb_get("/"))
            producer.send(topic1, value=odb_json)
            end1 = time.time()
            if verbose: print("ODB elapsed: {:.2f}, payload size {:.1f} kb".format(end1-start1, len(odb_json.encode('utf-8'))/1024))
            # ######
            
        start2 = time.time()
        event = client.receive_event(buffer_handle, async_flag=True)
        if event is not None:
            if event.header.is_midas_internal_event():
                if verbose:
                    print("Saw a special event")
                continue
                
            # load global event useful variables from header
            bank_names    = ", ".join(b.name for b in event.banks.values())
            event_number  = event.header.serial_number
            event_time    = datetime.fromtimestamp(event.header.timestamp).strftime('%Y-%m-%d %H:%M:%S')
            run_number    = client.odb_get("/Runinfo/Run number")
            event_info["timestamp"]             = event.header.timestamp
            event_info["serial_number"]         = event.header.serial_number
            event_info["event_id"]              = event.header.event_id
            event_info["trigger_mask"]          = event.header.trigger_mask
            event_info["event_data_size_bytes"] = event.header.event_data_size_bytes
            event_info["run_number"]            = run_number
            event_info_json = dumps(event_info)
            # #################
            # update EVENT
            if sendevent:
                payload = event.pack()

#                 payload_name =  "/tmp/payload.dat"

# test load from file (non cambia)
#                 with open(payload_name, "wb") as f:
#                     f.write(payload)
#                 with open(payload_name, 'rb') as f:
#                     data_bytes = f.read()
#                 data_base64 = base64.b64encode(data_bytes).decode('utf-8')
# oroiginario
#                 binary_data = io.BytesIO()
#                 binary_data.write(payload)
#                 binary_data.seek(0)
#                 encoded_data = binary_data.read()
#                 data_base64 = base64.b64encode(encoded_data).decode('utf-8')
# shorter

#                 data_base64 = base64.b64encode(payload).decode('utf-8')

#                 future = producerb.send(topic2, data_base64.encode('utf-8'))
        


                binary_data = io.BytesIO()
                binary_data.write(payload)
                binary_data.seek(0)
                encoded_data = binary_data.read()

                future = producerb.send(topic2, encoded_data)

                future.add_errback(on_error)
                end3 = time.time()
                #subprocess.Popen(producerb.flush(), stdout=None, stderr=None, stdin=None, close_fds=True)
                producerb.flush()

                end2 = time.time()
                if verbose: 
                    future.add_callback(on_success)
                    print("EVENT elapsed: {:.2f}, flush {:.2f}, payload size {:.1f} Mb, timestamp {:} ".format(end2-start2, end2-end3, np.size(payload)/1024/1024, event.header.timestamp))

            image_update_time = client.odb_get("/middleware/image_update_time")
            if ('CAM0' in bank_names) and (int(time.time())%image_update_time==0): # CAM image
                image, _, _ = cy.daq_cam2array(event.banks['CAM0']) # matrice delle imagine
                image_jpg(image, vmin, vmax, event_number, event_time)

              
        client.communicate(10)
        time.sleep(0.1)
        

    client.deregister_event_request(buffer_handle, request_id)

    client.disconnect()
    
        
if __name__ == "__main__":
    from optparse import OptionParser
    parser = OptionParser(usage='usage: %prog\t ')
    parser.add_option('-e','--event', dest='event', action="store_false", default=True, help='NO send event [True];');
    parser.add_option('-o','--odb', dest='odb', action="store_false", default=True, help='NO send odb [True];');
    parser.add_option('-v','--verbose', dest='verbose', action="store_true", default=False, help='verbose output;');
    (options, args) = parser.parse_args()
    if options.verbose: print(options, args)
    main(options.odb, options.event, verbose=options.verbose)
