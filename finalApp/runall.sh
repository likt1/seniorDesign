# kick off all things at once (store process ids into /root/)
cwd=$(pwd)

#cd /root/seniorDesign/finalApp/adc
#make clean
#make

cd /root/seniorDesign/finalApp/adc
nice --10 ./circularBuffer &
echo "$!" > /root/cbufPID 

cd /root/seniorDesign/finalApp/bluetooth
python3 simpleBluetoothServer.py &
echo "$!" > /root/btPID 

cd /root/seniorDesign/finalApp/gpio
python3 DIO.py &
echo "$!" > /root/dioPID 
python3 EnactPWM.py &
echo "$!" > /root/ledPID 

# restore previous working directory
cd $cwd 

# to start the services run the python3 service-creator.py
# then run "executeProgram.sh python3 /root/seniorDesign/finalApp/gpio/EnactPWM.py"
