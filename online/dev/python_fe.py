"""
Example of a basic midas frontend that has one periodic equipment.

See `examples/multi_frontend.py` for an example that uses more
features (frontend index, polled equipment, ODB settings etc). 
"""

import midas
import midas.frontend
import midas.event
from paramiko import SSHClient
import numpy as np
from time import sleep
from datetime import datetime

def printMess(mess, file_tf=False, dt=0, verbose=False):
    import os
    import threading
    import subprocess
    import sys
    from discord_webhook import DiscordWebhook
    discord_webhook_URL = "https://discord.com/api/webhooks/1076820494310981662/" + \
                          "jDr12DfVDGRE4isrtlntuafcZCDjHU2h08znNXEw9uCsI0GvGi6Y9WVZRzrswc8f3tkz"

    # TEST webhook
    #discord_webhook_URL = "https://discord.com/api/webhooks/1101445537724633149/" + \
    #                      "rHwS15gv3rQ-R15s7Yrl18N0TWE5lAoW6un5NDLF-lpk-M-pq3LgQ59R4Z3UDD5Zih44"
    if file_tf:
        webhook = DiscordWebhook(url=discord_webhook_URL, username=mess)
        with open(mess, "rb") as f:
            webhook.add_file(file=f.read(), filename=mess)
    else:
        webhook = DiscordWebhook(url=discord_webhook_URL, content=mess)
    response = webhook.execute()
    if dt>0:
        thread = threading.Thread(target=del_m(webhook,dt))
        thread.daemon = True
        thread.start()


class TriggerModuleEquipment(midas.frontend.EquipmentBase):
    """
    We define an "equipment" for each logically distinct task that this frontend
    performs. For example, you may have one equipment for reading data from a
    device and sending it to a midas buffer, and another equipment that updates
    summary statistics every 10s.
    
    Each equipment class you define should inherit from 
    `midas.frontend.EquipmentBase`, and should define a `readout_func` function.
    If you're creating a "polled" equipment (rather than a periodic one), you
    should also define a `poll_func` function in addition to `readout_func`.
    """
    def __init__(self, client):
        # The name of our equipment. This name will be used on the midas status
        # page, and our info will appear in /Equipment/MyPeriodicEquipment in
        # the ODB.
        equip_name = "TriggerModule"
        
        # Define the "common" settings of a frontend. These will appear in
        # /Equipment/MyPeriodicEquipment/Common. The values you set here are
        # only used the very first time this frontend/equipment runs; after 
        # that the ODB settings are used.
        default_common = midas.frontend.InitialEquipmentCommon()
        default_common.equip_type = midas.EQ_PERIODIC
        default_common.buffer_name = "SYSTEM"
        default_common.trigger_mask = 0
        default_common.event_id = 8
        default_common.period_ms = 10000
        default_common.read_when = midas.RO_ALWAYS
        default_common.log_history = 1
        
        # You MUST call midas.frontend.EquipmentBase.__init__ in your equipment's __init__ method!
        midas.frontend.EquipmentBase.__init__(self, client, equip_name, default_common)
        
        
        # Connect
        self.sshclient = SSHClient()
        self.sshclient.load_system_host_keys()
        self.sshclient.connect('192.168.99.16', username='pi', password='trigger')
        
        
        # Initialization of cache
        self.last = True ## provvisorio
        self.configuration_registers       = ['0', '0', '0', '0', '0', '0', '0', '0']
        self.configuration_registers_cache = ['0', '0', '0', '0', '0', '0', '0', '0']
        self.REG_COUNTER_MAP = {'0': 0.5, '1': 1, '2': 10, '3': 100}
        
        # You can set the status of the equipment (appears in the midas status page)
        self.set_status("Initialized")
        
    def readout_func(self):
        """
        For a periodic equipment, this function will be called periodically
        (every 100ms in this case). It should return either a `cdms.event.Event`
        or None (if we shouldn't write an event).
        """
        
        for i in range(8):
            self.configuration_registers[i] = str(self.client.odb_get("/Equipment/TriggerModule/Settings/TriggerModuleConfigReg[{0:d}]".format(i)))
        
        # Caching
        if self.configuration_registers_cache != self.configuration_registers:
            # Run the command to set the configuration
            options_string = ''
            for reg in self.configuration_registers:
                options_string += ' '+reg
            print('python3 /home/pi/Programs/LIME/setConf.py '+options_string) #DEBUG
            self.stdin, self.stdout, self.stderr = self.sshclient.exec_command('python3 /home/pi/Programs/LIME/setConf.py '+options_string)
            self.configuration_registers_cache = self.configuration_registers.copy()
        
        # Run the command to get the scaler readout
        self.stdin, self.stdout, self.stderr = self.sshclient.exec_command('python3 /home/pi/Programs/LIME/getReadout.py')
            
        # Unpack string
        results = self.stdout.read().decode("utf8")
        #print(results)
        results = results.replace(" ", "")
        results = results.split("[")[1]
        results = results.split("]")[0]
        results = results.split(",")
        
        
        reg_counter = self.configuration_registers[6]
        results = np.array([float(x)/self.REG_COUNTER_MAP[reg_counter] for x in results])
    
        print(results, datetime.now().strftime("%Y-%m-%d %H:%M:%S"), flush = True)
        
        if len(results)!=5 and self.last:
            self.client.msg("WARNING: TriggerModule not communicating properly!",
                        is_error=True)
        #    printMess("WARNING: TriggerModule not communicating properly!")
        #    self.last = False
        elif len(results)==5:
            self.last = True
            for i, r in enumerate(results):
                self.client.odb_set("/Equipment/TriggerModule/Variables/TriggerModuleRate[{0:d}]".format(i), r)
                #print(r, "---", datetime.now()) #DEBUG
        
        
        self.stdin.close()
        self.stdout.close()
        self.stderr.close()
        
        # In this example, we just make a simple event with one bank.
        event = midas.event.Event()
        
        # Create a bank (called "MYBK") which in this case will store 8 ints.
        # data can be a list, a tuple or a numpy array.
        data = list(results)
        event.create_bank("TMOD", midas.TID_FLOAT, data)
        
        
        
        return event
    
    def closeModule(self):
        # Because they are file objects, they need to be closed
        
        #print("Disconnecting from the Trigger Module...", flush = True) # DEBUG
        # Close the client itself
        self.sshclient.close()

class PythonFrontend(midas.frontend.FrontendBase):
    """
    A frontend contains a collection of equipment.
    You can access self.client to access the ODB etc (see `midas.client.MidasClient`).
    """
    def __init__(self):
        # You must call __init__ from the base class.
        midas.frontend.FrontendBase.__init__(self, "python_fe")
        
        # You can add equipment at any time before you call `run()`, but doing
        # it in __init__() seems logical.
        self.add_equipment(TriggerModuleEquipment(self.client))
        
    def begin_of_run(self, run_number):
        """
        This function will be called at the beginning of the run.
        You don't have to define it, but you probably should.
        You can access individual equipment classes through the `self.equipment`
        dict if needed.
        """
        self.set_all_equipment_status("Running", "greenLight")
        #self.client.msg("Frontend has seen start of run number %d" % run_number)
        return midas.status_codes["SUCCESS"]
        
    def end_of_run(self, run_number):
        self.set_all_equipment_status("Finished", "greenLight")
        #self.client.msg("Frontend has seen end of run number %d" % run_number)
        return midas.status_codes["SUCCESS"]
    
    def frontend_exit(self):
        self.equipment["TriggerModule"].closeModule()
        
        
if __name__ == "__main__":
    # The main executable is very simple - just create the frontend object,
    # and call run() on it.
    with PythonFrontend() as python_fe:
        python_fe.run()
