import sys, os
import numpy as np
import re
import time
import datetime
import midas
import midas.client

#### to do:
###### add this program to midas
###### 

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
    


def getSecondsFromLastSave():
    with open('/home/standard/daq/online/dev/tmp_gascontroller.txt', 'r') as f:
        line = f.read()
    last = datetime.datetime.strptime(line, '%Y-%m-%d %H:%M:%S.%f')
    now  = datetime.datetime.now()
    return (now-last).total_seconds()


if __name__ == "__main__":
    
    mclient = midas.client.MidasClient("checkGasController")
    
    try:
        
        last = True ## provvisorio
        while True:
            
            #print("Seconds from last save:")
            tdiff_seconds = getSecondsFromLastSave()
            mclient.odb_set("/Equipment/GasSystem/Settings/seconds_from_last_GC_save", tdiff_seconds);
            
            #print(tdiff_seconds)
            if tdiff_seconds > 120 and last:
                print("WARNING: GasSystemController not communicating in the last {0:.0f} s!".format(tdiff_seconds))
                mclient.msg("WARNING: GasSystemController not communicating in the last {0:.0f} s!".format(tdiff_seconds),
                            is_error=True)
                printMess("WARNING: GasSystemController not communicating in the last {0:.0f} s!".format(tdiff_seconds))
                last = False
            elif tdiff_seconds <= 120:
                last = True
                
            mclient.communicate(60)
            time.sleep(60)
            
    except KeyboardInterrupt:
        mclient.disconnect()
        print ("\nBye, bye...")
        sys.exit()
