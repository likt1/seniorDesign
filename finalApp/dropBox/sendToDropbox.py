#!/usr/bin/python3

import sys
import dropbox
import configparser as ConfigParser
import datetime

"""
INSPIRADO
 
Upload to Dropbox - Python script

"""

ini = '/root/conf/config.ini'

def main(fileName):
    ### Load key, secret, and access token from config.ini ###
    config = ConfigParser.SafeConfigParser()
    config.read(ini)

    app_key = config.get('Dropbox','app_key')
    app_secret = config.get('Dropbox','app_secret')
    access_token = config.get('Dropbox','access_key')
    ###
    
    try:
        dbx = dropbox.Dropbox(access_token)
        dbx.users_get_current_account()
    except Exception as e:
        print(" Unable to connect to Dropbox")
        sys.exit(1)

    try:
        with open(fileName, 'rb') as f:
            dbx.files_upload(f.read(),'/'+fileName, mute=True)
        print("[!] Uploaded %s to Dropbox\n" % (fileName))
    except Exception as e:
        print("Failed to upload %s\n%s" % (file, e))
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print('Usage: {0} <file name>'.format(sys.argv[0]))
        sys.exit(-1)
    fileName = str(sys.argv[1]);
    main(fileName)
