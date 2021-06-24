#!/usr/bin/env python3

#USAGE: python3 Cycle_Dumper_mid2cloud.py   number_of_first_run    number_of_last_run
 
#It transforms the .mid (MIDAS file) into histograms_RunXXXXX.root, storing them into offline folder. Then sends them to the cloud and if succesful removes the histograms_Run locally

#REMEMBER to correctly choose the tag: 'LAB' for LNF, 'MAN' for MANGO @ LNGS

TAG='MAN'

def upload_file(file_name, bucket='cygnus', tag='LAB'):
    # https://boto3.amazonaws.com/v1/documentation/api/latest/guide/s3-uploading-files.html#uploading-files
    import boto3
    from boto3sts import credentials as creds
    import logging
    import botocore
    import requests
    #
    endpoint='https://minio.cloud.infn.it/'
    version='s3v4'
    oidctoken = 'infncloud'
    #
    aws_session = creds.assumed_session(oidctoken)
    s3 = aws_session.client('s3', endpoint_url=endpoint, config=boto3.session.Config(signature_version=version),verify=True)

    key = 'Data/'+tag+'/'

    # Upload the file
    
    try:
        response=s3.head_object(Bucket=bucket,Key=key+file_name)
        print("The file already exists and has a dimension of "+str(response['ContentLength']/1024./1024.)+' MB')
        return True, False
    
    except (botocore.exceptions.ConnectionError, requests.exceptions.ConnectionError):
              print("There was a connection error or failed")
              return False, False

    except botocore.exceptions.ClientError:
        print('No file with this name was found on the cloud, it will be uploaded')

        try:
              out = key+os.path.basename(file_name)
              response = s3.upload_file(file_name, bucket, out)
              print ('Uploaded file: '+out)
        except Exception as e:
                   logging.error(e)
                   return False, False
        return True, True







import os,sys

begin=int(sys.argv[1])
end=int(sys.argv[2])+1
lista=list(range(begin,end))
total=lista[-1]-lista[0]+1

current_try=0
max_tries=5

status=False		#flag to know if to go to next run
remove=False		#flag to know if to remove histo.root

f=open('logupload_frommid.txt','w')

for j in lista:
       #Converting the mid to histo.root
       nome="DataTree%05d.root" % (j)
       outnome="histograms_Run%05d.root" % (j)

       os.chdir("../analyzer")
       os.system("./cyganalyzer.exe -b -i offlineConfig.xml -r "+str(j))
       os.chdir("../offline")
       print("\nConverting "+nome+"\n")
       os.system("./PlainTObjectsDumper ../analyzer/"+nome+" [--max-entries=10] --outdir .")
       os.system("rm ../analyzer/"+nome)


       #Now uploads the file  
       if not os.path.isfile(outnome):
               line=outnome+' does not exist locally :('
               print(line)
               f.write(line)
               continue

       print('Transferring '+outnome)

       while(not status):   #tries many times for errors of connection or others that may be worth another try
              status,remove=upload_file(outnome,tag=TAG)
              if remove:
                       os.system('rm -f '+outnome)
              if status and not remove:
                      line=outnome +' was not uploaded beacause it was already present a file with the same name on the cloud'
                      f.write(line)
              current_try=current_try+1
              if current_try==max_tries:
                   #print('Try number '+current_try)
                   status=1
                   line=outnome+' uploading failed'
                   f.write(line)
       current_try=0
       status=False
       remove=False

       total=total-1
       if(total==0):
              print("\n Finished!!")
       else:
              print(str(total)+ ' still to UPLOAD! :)\n')

f.flush()
f.close()



