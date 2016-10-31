import dropbox
import ConfigParser
import datetime

print 'Dropbox API test'

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

time = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')

#Upload a dummy text file
dbx.files_upload(time,'/'+time+'.txt')

for entry in dbx.files_list_folder('').entries:
    print(entry.name)