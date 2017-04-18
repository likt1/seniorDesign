#!/usr/bin/python3
import subprocess
import os
from time import sleep

print("dio program started\n")

# ensure the pins are configured
subprocess.call(['sh','/root/seniorDesign/finalApp/gpio/eqep-pin-config.sh'])

sd_loc = "/media/store/sd-card"

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
recordingFlag = -1 # will be set when actively recording
flagSD = -1
activeReady = False # will be set when in Active Mode
activeSwitch = False # will initialize to value of currentSwitchState when active Recording is set
currentSwitchState = 0

temp = "[DIO]\nCompRotary=xx\nTimeRotary=yy\nFootswitch=zz\nMemoryLow=aa\nIsRecording=bb" #set template

# TODO: enhance to support MemoryLow and IsRecording...

def getIndex(val, length):
    return val%length

while True:
    sleep(0.1)

    # check file for previous settings (allows phone app to also modify...)    
    if os.path.isfile("/root/conf/DIO.config"):
        f = open("/root/conf/DIO.config",'r')
        settings = f.readlines()

        # if a file is writing concurently with our read, we may get an empty file
        # in this case just continue iterating until a valid file is read
        if len(settings) == 0:
            continue 

        idxTime = settingsTime.index(settings[2].split("=",1)[1].strip()) # Time
        idxType = settingsType.index(settings[1].split("=",1)[1].strip()) # Type
        currentSwitchState = settings[3].split("=",1)[1].strip() in ["true", "True"] # switch
        
        # May be used in future
        #prev_warning = settings[3].split(":",1)[1].strip()
        #prev_active = settings[4].split(":",1)[1].strip()
        
        f.close()
    
    # get rotary value for retro-Time/Active
    target = open('/sys/devices/platform/ocp/48300000.epwmss/48300180.eqep/position', 'r')
    currentReading = int(target.read())
    target.close()

    if prevRotaryReading1 == -500000: # if init, set previous as current
        prevRotaryReading1 = currentReading
        temp = temp.replace("yy",settingsTime[getIndex(idxTime,len(settingsTime))])
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
        temp = temp.replace("xx",settingsType[getIndex(idxType,len(settingsType))])
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
        if count == 20:
            currentSwitchState = not currentSwitchState
            temp = temp.replace("Footswitch="+str(not currentSwitchState),"Footswitch="+str(currentSwitchState))  # write to config
            count = 0
            debounce = False
            flag = 1

    # check if we are actively recording (need to blink)
    # NOTE: this functionality may be moved to the circular buffer if needed, this is tentative
    #if 
    
    if recordingFlag == -1: # initiallize IsRecording
        recordingFlag = False
        temp = temp.replace("bb","False") # write to config
        flag = 1
    
    if settingsTime[getIndex(idxTime,len(settingsTime))] == "active" and activeReady == False:
        activeReady = True
        activeSwitch = currentSwitchState
        
    elif settingsTime[getIndex(idxTime,len(settingsTime))] != "active" and activeReady == True:
        if recordingFlag == True:
            recordingFlag = False
            temp = temp.replace("IsRecording=True","IsRecording=False") # write to config
            activeCount = 0
            # Set Recording LED to solid ON
            flag = 1
        activeReady = False
        
    elif activeReady == True and activeSwitch != currentSwitchState and recordingFlag == False:
        recordingFlag = True
        temp = temp.replace("IsRecording=False","IsRecording=True") # write to config
        activeSwitch = currentSwitchState
        flag = 1
    
    elif activeReady == True and activeSwitch != currentSwitchState and recordingFlag == True:
        recordingFlag = False
        temp = temp.replace("IsRecording=True","IsRecording=False") # write to config
        activeSwitch = currentSwitchState
        # Set Recording LED to solid ON
        flag = 1

            
    # check if we have low sd-card memory (need to blink)
    # NOTE: this functionality may be moved to the circular buffer if needed, this is tentative

    if flagSD == -1:
        flagSD = False
        temp = temp.replace("aa","False") # write to config
    
    if os.path.ismount(sd_loc):
        #print("discovered sd card")

        df = subprocess.Popen(["df", sd_loc], stdout=subprocess.PIPE)

        output = df.communicate()[0]

        device, size, used, available, percent, mountpoint = \
            output.decode("UTF-8").split("\n")[1].split()

        # uncomment to audit df return
        #print(device, size, used, available, percent, mountpoint)

        if False:#int(available) < 300000 and flagSD == False:
            #print("warn the user, space available (in sd card) is below 30MB")
            flagSD = True
            temp = temp.replace("MemoryLow=False","MemoryLow=True") # write to 
            flag = 1
            
        elif False:#int(available) >= 300000 and flagSD == True:
            #print("space available is fine (for sd card)")
            flagSD = False
            temp = temp.replace("MemoryLow=True","MemoryLow=False") # write to config
            flag = 1
    
    
    if flag == 1:        
        target = open('/root/conf/DIO.config', 'w')
        target.write(temp.strip())
        target.close()
        flag = 0        
            
