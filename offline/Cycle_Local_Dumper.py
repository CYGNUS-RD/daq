#!/usr/bin/env python3
#USAGE: python3 Cycle_Local_Dumper.py   number_of_first_run    number_of_last_run
#Transforms the .mid files (MIDAS files) into histograms_RunXXXXX.root, storing them into offline folder 

import os,sys

begin=int(sys.argv[1])
end=int(sys.argv[2])+1
for j in range(begin,end):
	nome="DataTree%05d.root" % (j)
	outnome="histograms_Run%05d.root" % (j)

	os.chdir("../analyzer")
	os.system("./cyganalyzer.exe -b -i offlineConfig.xml -r "+str(j))
	os.chdir("../offline")
	print("\nConverting "+nome+"\n")
	os.system("./PlainTObjectsDumper ../analyzer/"+nome+" [--max-entries=10] --outdir .")
	os.system("rm ../analyzer/"+nome)
print("Cycle Terminated")
