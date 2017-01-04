#!/bin/bash
# use external provisioning setup
# content will be run upon first vagrant up, or using --provision arg
echo "Provisioning virtual machine..."

echo "Installing Git"
apt-get install git -y > /dev/null

echo "Installing Vim"
apt-get install vim -y > /dev/null

# if we discover any new packages are necessary, add them here and pipe to null

# run custom linking & configuration file copying here (some examples are commented below)
#echo "Running example configuration"
#cp /var/www/provision/config/config-example /home/vagrant/ > /dev/null
#ln -s /home/vagrant/config-example /etc/<some needed link...>
#mkdir /home/vagrant/stuff
#rm -rf /home/vagrant/stuff
#service <some service i.e. ssh> restart > /dev/null
