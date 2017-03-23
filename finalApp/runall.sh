# kick off all things at once
cd /root/seniorDesign/finalApp/adc
make clean
make

cd /
/root/seniorDesign/finalApp/adc/circularBuffer &
python3 /root/seniorDesign/finalApp/bluetooth/simpleBluetoothServer.py &
python3 /root/seniorDesign/finalApp/gpio/DIO.py &
python3 /root/seniorDesign/finalApp/gpio/EnactPWM.py &

# to start the services run the python3 service-creator.py
# then run "executeProgram.sh python3 /root/seniorDesign/finalApp/gpio/EnactPWM.py"
