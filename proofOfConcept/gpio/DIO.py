prevRotaryReading1 = -500000 # arbitrary init val
prevRotaryReading2 = -500000
prevSwitchReading = -1
settingsTime = ["active", "30s", "1m", "1m30s", "2m", "2m30s", "3m"]
settingsType = ["wav", "mp3", "flac", "ogg"]
idxTime = 0
idxType = 0
count = 0
debounce = False
flag = 0
currentSwitchState = 0

def getIndex(val, length):
    return val%length

while True:
    temp = "CompRotary:xx\nTimeRotary:yy\nFootswitch:zz\n" #set template

    # get rotary value for retro-Time/Active
	
    target = open('/sys/devices/platform/ocp/48304000.epwmss/48304180.eqep/position', 'r')
    currentReading = int(target.read())
    target.close()

    if prevRotaryReading1 == -500000: # if init, set previous as current
        prevRotaryReading1 = currentReading
        idxTime = 3
        flag = 1
    
    if (currentReading - prevRotaryReading1) > 10:
        idxTime+=1
        prevRotaryReading1 = currentReading
        flag = 1
    elif (prevRotaryReading1 - currentReading) > 10:
        idxTime-=1
        prevRotaryReading1 = currentReading
        flag = 1
		
	temp = temp.replace("yy",settingsTime[getIndex(idxTime,len(settingsTime))]) # write to config
    
    
    # get rotary value for compression type
	
    target = open('/sys/devices/platform/ocp/48304000.epwmss/48304180.eqep/position' , 'r')
    currentReading = int(target.read())
    target.close()

    if prevRotaryReading2 == -500000: # if init, set previous as current
        prevRotaryReading2 = currentReading
        idxType = 0
        temp = temp.replace("xx",settingsType[0])
        flag = 1
    
    if (currentReading - prevRotaryReading1) > 10:
        idxType+=1
        prevRotaryReading2 = currentReading
        flag = 1
    elif (prevRotaryReading2 - currentReading) > 10:
        idxType-=1
        prevRotaryReading2 = currentReading
        flag = 1
		
	temp = temp.replace("xx",settingsType[getIndex(idxType,len(settingsType))])	# write to config

    # get latchswitch value for recording
    if prevSwitchReading == -1: # if init, set previous as current
        prevSwitchReading = currentReading
        currentSwitchState = False
        flag = 1
    
    if prevSwitchReading != currentReading:
        count = 0
        prevSwitchReading  = currentReading
        debounce = not debounce
    elif debounce == True and prevSwitchReading == currentReading:
        count +=1 #debouncing
        if count == 5:
            currentSwitchState = not currentSwitchState # write to config
            count = 0
            debounce = False
            flag = 1
		
	temp = temp.replace("zz",str(currentSwitchState)) # write to config
    
    if flag == 1:        
        target = open('/root/conf/DIO.config', 'w')
        target.write(temp)
        target.close()
        flag = 0        
            
