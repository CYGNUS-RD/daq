#/bin/bash
#source /home/DAQsetup.sh
# export discord_webhook_URL='https://discord.com/api/webhooks/1076436817345265694/Jq6hoGlyrmLov43GV_CSFIaDlQ6MNgV3wBFa52zP-oW_bifL6Qq1UvbO5irw_yWq07YD' # tes mazzitelli
#export discord_webhook_URL='https://discord.com/api/webhooks/1101445537724633149/rHwS15gv3rQ-R15s7Yrl18N0TWE5lAoW6un5NDLF-lpk-M-pq3LgQ59R4Z3UDD5Zih44' # test channel
#export discord_webhook_URL='https://discord.com/api/webhooks/1076820494310981662/jDr12DfVDGRE4isrtlntuafcZCDjHU2h08znNXEw9uCsI0GvGi6Y9WVZRzrswc8f3tkz'

export discord_webhook_URL='https://discord.com/api/webhooks/1288129500684488795/qFnFkDRUe-EUrYgRc8yv9w4l1w63ZE0sek4SAh89igcGJTOKLvmCs6IP87Lb4AJoyf64'

#setupd='grep discord_webhook_URL /home/DAQsetup.sh'
#env $setupd >/dev/null
tail -10 /data/midas.log >  /home/cygno01/daq/online/log/attach.txt
if [ "$#" -eq  "0" ]
    then
        subject="Generic_ALARM"
    else
        subject=$1"_ALARM"
fi
 
if [ "$1" = "Software" ] || [ "$1" = "Warning" ]
    then
        odbedit -c "set /Sequencer/State/Paused y"
	#odbedit -c pause
        mess=`tail -n 2 /home/cygno01/daq/online/log/attach.txt`
        /home/cygno01/daq/middleware/dev/discord.py "**$subject**\n$mess"
    else
        odbedit -c "set /Sequencer/State/Paused y"
	#odbedit -c pause
        mess=`tail -n 2 /home/cygno01/daq/online/log/attach.txt`
        /home/cygno01/daq/middleware/dev/discord.py "**$subject**\n$mess"
#        echo -e "\n \n go to: https://172.18.9.72:8443/ \n bye, bye" | mutt -s $subject -i /home/standard/daq/online/log/attach.txt giovanni.mazzitelli@lnf.infn.it, francesco.renga@roma1.infn.it, stefano.piacentini@roma1.infn.it, davide.pinci@roma1.infn.it
#        mess=`cat /home/standard/daq/online/log/attach.txt`
#        /home/standard/daq/middleware/dev/discord.py "**$subject**\n$mess"
fi
