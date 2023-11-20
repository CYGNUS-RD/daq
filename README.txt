1. Check experimental setup

2. Turn on camera, NIM and VME crate and oscilloscope

3. From start menu open VirtualBox. Turn on the Windows virtual machine (Start button)

	3a. In windows, open HVGEM AMP 6.2.vi 
	3.b After loading click the arrow and it should work

4. Back to Ubuntu

5. Open terminal and wait for it to complete the bashrc

6. cd daq/online

7. ./start_daq.sh

8. On the browser go to http://localhost:8080

9. On the Program tab turn on all the required programs (all but SC frontend)

10. Open another terminal

11. cd daq/middleware/dev

12. python3 readHVGEM.py &

13. python3 event_receiver_pmt_lnf.py

14. When you change a voltage setting on the Labview program, once the voltages have stabilized, click on "Write data" button (bottom left)

Suggestion:

15. Use the status page to do runs for the settings, mostly not when you want to save them.

16. Use the sequencer to take the actual data


OTHER THINGS
When taking data always check that the HVstate variable is 1 for data and 0 for pedestal. You can set this value in the sequencer. Alternatively you can manually check and modify it and other sql database value on the ODB tab and going through the path /Logger/Runlog/SQL/Links BOR/

If the cygnus_daq crashes, in program folder restart it. Also Ctrl+C the event_receiver_pmt_lnf.py program and restart it.
If the problem continues turn off and on the VME crate

To close the readHVGEM.py go htop and SIGTERM SIGKILL the corresponding process

