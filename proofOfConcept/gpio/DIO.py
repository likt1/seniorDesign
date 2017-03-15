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
temp = "CompRotary:xx\nTimeRotary:yy\nFootswitch:zz\nMemoryLow:asdf\n" #set template

def getIndex(val, length):
    return val%length

while True:

    # get rotary value for retro-Time/Active
    
    target = open('/sys/devices/platform/ocp/48300000.epwmss/48300180.eqep/position', 'r')
    currentReading = int(target.read())
    target.close()

    if prevRotaryReading1 == -500000: # if init, set previous as current
        prevRotaryReading1 = currentReading
        idxTime = 3
        temp = temp.replace("yy",settingsTime[3])
        flag = 1
    
    if (currentReading - prevRotaryReading1) > 10:
        idxTime+=1
        temp = temp.replace(settingsTime[getIndex((idxTime-1),len(settingsTime))],settingsTime[getIndex(idxTime,len(settingsTime))]) # write to config
        prevRotaryReading1 = currentReading
        flag = 1
    elif (prevRotaryReading1 - currentReading) > 10:
        idxTime-=1
        temp = temp.replace(settingsTime[getIndex((idxTime+1),len(settingsTime))],settingsTime[getIndex(idxTime,len(settingsTime))]) # write to config
        prevRotaryReading1 = currentReading
        flag = 1
    
    
    # get rotary value for compression type
    
    target2 = open('/sys/devices/platform/ocp/48304000.epwmss/48304180.eqep/position' , 'r')
    currentReading = int(target2.read())
    target2.close()

    if prevRotaryReading2 == -500000: # if init, set previous as current
        prevRotaryReading2 = currentReading
        idxType = 0
        temp = temp.replace("xx",settingsType[0])
        flag = 1
    
    if (currentReading - prevRotaryReading2) > 10:
        idxType+=1
        temp = temp.replace(settingsType[getIndex((idxType-1),len(settingsType))],settingsType[getIndex(idxType,len(settingsType))]) # write to config
        prevRotaryReading2 = currentReading
        flag = 1
    elif (prevRotaryReading2 - currentReading) > 10:
        idxType-=1
        temp = temp.replace(settingsType[getIndex((idxType+1),len(settingsType))],settingsType[getIndex(idxType,len(settingsType))]) # write to config
        prevRotaryReading2 = currentReading
        flag = 1

    # get latchswitch value for recording
    target3 = open('/sys/class/gpio/gpio69/value' , 'r')
    currentReading = int(target3.read())
    target3.close()
	
    if prevSwitchReading == -1: # if init, set previous as current
        prevSwitchReading = currentReading
        currentSwitchState = False
        temp = temp.replace("zz",str(currentSwitchState)) # write to config
        flag = 1
    
    if prevSwitchReading != currentReading:
        count = 0
        prevSwitchReading  = currentReading
        debounce = not debounce
    elif debounce == True and prevSwitchReading == currentReading:
        count +=1 #debouncing
        if count == 5:
            currentSwitchState = not currentSwitchState
            temp = temp.replace(str(not currentSwitchState),str(currentSwitchState))  # write to config
            count = 0
            debounce = False
            flag = 1
    
    if flag == 1:        
        target = open('/root/conf/DIO.config', 'w')
        target.write(temp)
        target.close()
        flag = 0        
            
