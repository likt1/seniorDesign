# after install, service can be interacted with via
#	sudo /etc/init.d/<myservicename> {start|stop|restart|uninstall}
#	sudo service <myservicename> {start|stop|restart|uninstall}
#	sudo systemctl {start|stop|restart|uninstall} <myservicename> 

# example usage of service-creator.py :
kevin@VBox-Ubu16:~/Documents/seniorDesign/proofOfConcept/service-creator$ sudo python3 service-creator.py 
Template script found, continuing service creation...
You will now be asked to enter various configurations for the script
Empty values will not be accepted!
Service name: my-service
Command: /home/kevin/my-service-prgm
Username: kevin
Description: testing123
Service script successfully created, continuing to service installation...
Installing...
chmod 755 my-service
mv my-service /etc/init.d/my-service
touch /var/log/my-service.log && chown kevin/var/log/my-service.log
update-rc.d my-service defaults
service my-service start

Uninstall Instructions:
The service can uninstall itself:
	service my-service uninstall
It will simply run update-rc.d -f my-serviceremove && rm -f /etc/init.d/my-service

