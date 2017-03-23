# kick off all things at once
#cd /root/seniorDesign/finalApp/adc
#make clean
#make

cd /root/seniorDesign/finalApp/adc
nice --10 ./circularBuffer &

cd /root/seniorDesign/finalApp/bluetooth
python3 simpleBluetoothServer.py &

cd /root/seniorDesign/finalApp/gpio
python3 DIO.py &
python3 EnactPWM.py &

# to start the services run the python3 service-creator.py
# then run "executeProgram.sh python3 /root/seniorDesign/finalApp/gpio/EnactPWM.py"
