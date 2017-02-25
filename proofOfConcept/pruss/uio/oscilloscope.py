import beaglebone_pru_adc as adc
import time
import struct

numsamples = 65536 # how many samples to capture

capture = adc.Capture()

capture.oscilloscope_init(adc.OFF_VALUES, numsamples) # captures AIN0 - the first elt in AIN array
#capture.oscilloscope_init(adc.OFF_VALUES+8, numsamples) # captures AIN2 - the third elt in AIN array
capture.start()

for _ in range(10):
    if capture.oscilloscope_is_complete():
        break
    print('.')
    time.sleep(0.1)

capture.stop()
capture.wait()

print('Saving oscilloscope values to "data.raw"')

with open('data.raw', 'wb') as f:
    for x in capture.oscilloscope_data(numsamples):
        data = struct.pack('H', x)
        f.write(data)


print('done')

capture.close()
