#!/bin/sh

./kill_daq.sh

odbedit -c clean
#odbedit -c "rm /Analyzer/Trigger/Statistics"
#odbedit -c "rm /Analyzer/Scaler/Statistics"

mhttpd -D
sleep 2
#xterm -e ./cygnus_fe &
sleep 2
#xterm -e ./scfe &
mlogger -D
msequencer -D
#xterm -e ../analyzer/cyganalyzer.exe -i ../analyzer/romeConfig.xml &
#xterm -e python3 event_receiver.py &

echo Please point your web browser to http://localhost:8080
echo Or run: mozilla http://localhost:8080 &
echo To look at live histograms, run: roody -Hlocalhost

#end file
