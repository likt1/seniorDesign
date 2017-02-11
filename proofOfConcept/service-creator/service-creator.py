#!/bin/python3
# Python script to create and install a service based on a template
# Inspired by / based on https://gist.github.com/naholyr/4275302

# Usage: python3 service-creator.py
#      <enter service name>
#      "<enter executable and any arguments>"
#      <enter user here (probably root)>
#      <enter description of what you hope to do here>
#    /etc/init.d/<service name> {start|stop|restart|uninstall}
#    systemctl {start|stop|restart|uninstall} <service name>
#    service <service name> {start|stop|restart|uninstall}
#
#    note: this program makes a service out of a command, so put sets of commands in a 
#      script or executable, then move to /opt/service-prgms/ and enter
#        "/opt/service-prgms/<executable name> <any args>" for the 2nd part of the creator
import os
import pwd
import subprocess
import sys

# Validate the template file exists
template_file = 'template-service.sh'
if not os.path.isfile(template_file):
	sys.exit('template file ' + template_file + ' not found, cancelling service creation...')
print('Template script found, continuing service creation...')

# Setup vars for template replacement
print('You will now be asked to enter various configurations for the script')
print('Empty values will not be accepted!')
name, command, username, description = '', '', '', ''

while not name or not command or not username:
	# prompt service name, command to run, and username until all are valid
	if not name:
		name = input("Service name: ")
	if not command:
		command = input("Command: ")
	if not username:
		username = input("Username: ")
	if not description:
		description = input("Description: ")

	# check for existing service
	if os.path.isfile('/etc/init.d/' + name):
		print('Error: service ' + name + ' already exists')
		name = ''
	
	# validate username
	try:
		pwd.getpwnam(username)
	except KeyError:
		print('Error: user ' + username + ' not found')
		username = ''

# Copy the template file and replace key variables with user input
template_script = open(template_file,'r')
service_script = open(name, 'w')
for lineRead in template_script:
	lineWrite = lineRead.replace("<COMMAND>", command)
	lineWrite = lineWrite.replace("<NAME>", name)
	lineWrite = lineWrite.replace("<USERNAME>", username)
	service_script.write(lineWrite)
template_script.close()
service_script.close()
print('Service script successfully created, continuing to service installation...')

# Issue various commands for installation (if permissions allow)
pidFile = "/var/run/" + name + ".pid"
logFile = "/var/log/" + name + ".log"
if not os.access('/etc/init.d', os.W_OK):
	print('This script does not have enough permissions to install the service...')
	print('To install it manually run the following: ')
	print('\t0. chmod 755 ' + name)
	print('\t1. mv ' + name + ' /etc/init.d/' + name)
	print('\t2. touch ' + logFile + ' && chown ' + username + logFile)
	print('\t3. update-rc.d ' + name + ' defaults')
	print('\t4. service ' + name + ' start')
else:
	print('Installing...')
	print('chmod 755 ' + name)
	os.chmod(name, 0o755)
	print('mv ' + name + ' /etc/init.d/' + name)
	subprocess.call(['mv', name, '/etc/init.d/' + name])
	print('touch ' + logFile + ' && chown ' + username + logFile)
	subprocess.call(['touch', logFile])
	subprocess.call(['chown', username,logFile])
	print('update-rc.d ' + name + ' defaults')
	subprocess.call(['update-rc.d', name, 'defaults'])
	print('service ' + name + ' start')
	subprocess.call(['service', name, 'start'])

# Final note on uninstalling
print('')
print('Uninstall Instructions:')
print('The service can uninstall itself:')
print('\tservice ' + name + ' uninstall')
print('It will simply run update-rc.d -f ' + name + 'remove && rm -f /etc/init.d/' + name)

