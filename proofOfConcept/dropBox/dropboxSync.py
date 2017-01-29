#!/usr/bin/python3

import sys
import os
import dropbox
import configparser
import datetime
import contextlib
import time
import unicodedata
import six

"""
INSPIRADO

Proof of Concept: 
    Sync with Dropbox - Python script

Author: Thomas Foertmeyer
"""

def main(localDir, cloudDir):
    ### Load key, secret, and access token from config.ini ###
    ini = 'config.ini'
    config = configparser.SafeConfigParser()
    config.read(ini)

    app_key = config.get('Dropbox','app_key')
    app_secret = config.get('Dropbox','app_secret')
    access_token = config.get('Dropbox','access_token')
    ###

    dbx = dropbox.Dropbox(access_token)
    dbx.users_get_current_account()

    print("%s\n%s" % (localDir,cloudDir))

    for dn, dirs, files in os.walk(localDir):
        subfolder = dn[len(localDir):].strip(os.path.sep)
        listing = list_folder(dbx, cloudDir, subfolder)
        print('Descending into', subfolder, '...')

    # First do all the files.
    for name in files:
        fullname = os.path.join(dn, name)
        if not isinstance(name, six.text_type):
            name = name.decode('utf-8')
        nname = unicodedata.normalize('NFC', name)
        if name.startswith('.'):
            print('Skipping dot file:', name)
        elif name.startswith('@') or name.endswith('~'):
            print('Skipping temporary file:', name)
        elif name.endswith('.pyc') or name.endswith('.pyo'):
            print('Skipping generated file:', name)
        elif nname in listing:
            md = listing[nname]
            mtime = os.path.getmtime(fullname)
            mtime_dt = datetime.datetime(*time.gmtime(mtime)[:6])
            size = os.path.getsize(fullname)
            if (isinstance(md, dropbox.files.FileMetadata) and
                mtime_dt == md.client_modified and size == md.size):
                print(name, 'is already synced [stats match]')
            else:
                print(name, 'exists with different stats, downloading')
                res = download(dbx, localDir, subfolder, name)
                with open(fullname) as f:
                    data = f.read()
                if res == data:
                    print(name, 'is already synced [content match]')
                else:
                    print(name, 'has changed since last sync')
                    if yesno('Refresh %s' % name):
                        print("DUMMY UPLOAD")
                        #upload(dbx, fullname, folder, subfolder, name,
                              # overwrite=True)
        elif yesno('Upload %s' % name):
            print("DUMMY UPLOAD")
            #upload(dbx, fullname, folder, subfolder, name)

def download(dbx, folder, subfolder, name):
    """Download a file.
    Return the bytes of the file, or None if it doesn't exist.
    """
    path = '/%s/%s/%s' % (folder, subfolder.replace(os.path.sep, '/'), name)
    while '//' in path:
        path = path.replace('//', '/')
    with stopwatch('download'):
        try:
            md, res = dbx.files_download(path)
        except dropbox.exceptions.HttpError as err:
            print('*** HTTP error', err)
            return None
    data = res.content
    print(len(data), 'bytes; md:', md)
    return data

def yesno(message):
    """Handy helper function to ask a yes/no question.
    """

    message += '? [N/y] '

    while True:
        answer = input(message).strip().lower()
        if not answer:
            return default
        if answer in ('y', 'yes'):
            return True
        if answer in ('n', 'no'):
            return False
        if answer in ('q', 'quit'):
            print('Exit')
            raise SystemExit(0)
        if answer in ('p', 'pdb'):
            import pdb
            pdb.set_trace()
        print('Please answer YES or NO.')

def list_folder(dbx, folder, subfolder):
    """List a folder.
    Return a dict mapping unicode filenames to
    FileMetadata|FolderMetadata entries.
    """
    path = '/%s/%s' % (folder, subfolder.replace(os.path.sep, '/'))
    while '//' in path:
        path = path.replace('//', '/')
    path = path.rstrip('/')
    try:
        with stopwatch('list_folder'):
            res = dbx.files_list_folder(path)
    except dropbox.exceptions.ApiError as err:
        print('Folder listing failed for', path, '-- assumped empty:', err)
        return {}
    else:
        rv = {}
        for entry in res.entries:
            rv[entry.name] = entry
    return rv

@contextlib.contextmanager
def stopwatch(message):
    """Context manager to print how long a block of code took."""
    t0 = time.time()
    try:
        yield
    finally:
        t1 = time.time()
    print('Total elapsed time for %s: %.3f' % (message, t1 - t0))

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print('Usage: {0} <local directory> <dropbox directory>'.format(sys.argv[0]))
        sys.exit(-1)
    localDir = str(sys.argv[1]);
    cloudDir = str(sys.argv[2])
    main(localDir,cloudDir)