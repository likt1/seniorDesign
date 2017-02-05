#!/usr/bin/python

import sys
import dropbox
import ConfigParser
import datetime

"""
INSPIRADO

Proof of Concept: 
    Upload to Dropbox - Python script

Requires ini file with Dropbox API credentials

Author: Thomas Foertmeyer
Issue: #1
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

    dbx.files_upload(fileName,'/'+fileName)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print 'Usage: {0} <file name>'.format(sys.argv[0])
        sys.exit(-1)
    fileName = str(sys.argv[1]);
    main(fileName)