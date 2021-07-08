#!/usr/bin/env python3
#USAGE: python3 Cycle_2cloud.py   number_of_first_run    number_of_last_run
 
#It sends the histograms_RunXXXXX.root present in offline folder to the cloud.

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
        return True

    except botocore.exceptions.ClientError:
        print('No file with this name was found on the cloud, it will be uploaded')

        try:
              out = key+os.path.basename(file_name)
              response = s3.upload_file(file_name, bucket, out)
              print ('Uploaded file: '+out)
        except Exception as e:
                   logging.error(e)
                   return False
        return True

    except (Exception,botocore.exceptions.ConnectionError, requests.exceptions.ConnectionError):
              print("There was a connection error or failed")
              return False





####MAIN####

import os, sys

begin=int(sys.argv[1])
end=int(sys.argv[2])+1
lista=list(range(begin,end))
total=lista[-1]-lista[0]+1
max_tries=5
status=0
current_try=0
f=open('logupload.txt','w')
for j in lista:
       name= 'histograms_Run%05d.root' % (j)

       if not os.path.isfile(name):
               line=name+' does not exist locally :('
               print(line)
               f.write(line)
               continue

       print('Transferring '+name)

       while(not status):   #tries many times for errors of connection or others that may be worth another try

              status=upload_file(name,tag=TAG)
              current_try=current_try+1
              if current_try==max_tries:
                   #print('Try number '+current_try)
                   status=1
                   line=name+' failed'
                   f.write(line)

       current_try=0
       status=0

       total=total-1
       if(total==0):
              print("\n Finished!!")
       else:
              print(str(total)+ ' still to UPLOAD! :)\n')

f.flush()
f.close()



