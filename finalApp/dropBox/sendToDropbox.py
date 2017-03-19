#!/usr/bin/python3

import sys
import dropbox
import configparser
import datetime

"""
INSPIRADO

Proof of Concept: 
    Upload to Dropbox - Python script

Author: Thomas Foertmeyer
"""

def main(fileName):
    ### Load key, secret, and access token from config.ini ###
    ini = 'config.ini'
    config = ConfigParser.SafeConfigParser()
    config.read(ini)

    app_key = config.get('Dropbox','app_key')
    app_secret = config.get('Dropbox','app_secret')
    access_token = config.get('Dropbox','access_token')
    ###

    dbx = dropbox.Dropbox(access_token)
    dbx.users_get_current_account()

    try:
        with open(fileName, 'rb') as f:
            dbx.files_upload(f.read(),'/'+fileName, mute=True)
    except Exception as err:
        print("Failed to upload %s\n%s" % (file, err))

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print('Usage: {0} <file name>'.format(sys.argv[0]))
        sys.exit(-1)
    fileName = str(sys.argv[1]);
    main(fileName)