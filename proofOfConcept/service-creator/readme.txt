# after install, service can be interacted with via
#	sudo /etc/init.d/<myservicename> {start|stop|restart|uninstall}
#	sudo service <myservicename> {start|stop|restart|uninstall}
#	sudo systemctl {start|stop|restart} <myservicename> 

# example usage of service-creator.py :
gcc test-service-prgm.c -o test-service
mkdir -p /opt/service-prgms
mkdir -p /home/vagrant
mv test-service /opt/service-prgms/
python3 service-creator.py
    test-service-read
    /opt/service-prgms/test-service r
    root
    test service to read contents of one file, then write to another
python3 service-creator.py
    test-service-write
    /opt/service-prgms/test-service w
    root
    test service to periodically (and indefinitely) write to a file
python3 service-creator.py
    test-service-append
    /opt/service-prgms/test-service a
    root
    test service to periodically (and indefinitely) append to a file
cd /home/vagrant
# note: there should now be two files
# catting the read log should alternate between append and overwrite
# atting the other log should alternate between appending and overwriting
service test-service-read stop
systemctl stop test-service-write
/etc/init.d/test-service-append stop
# note 2: these services will be started on boot

#Uninstall Instructions:
#The service can uninstall itself:
#	service my-service uninstall
#It will simply run update-rc.d -f my-serviceremove && rm -f /etc/init.d/my-service
