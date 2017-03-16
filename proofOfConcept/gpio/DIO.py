import subprocess
import os

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
activeReady = False # will be set when in Active Mode
activeSwitch = False # will initialize to value of currentSwitchState when active Recording is set
activeCount = 0
currentSwitchState = 0
flagSD = -1

temp = "CompRotary:xx\nTimeRotary:yy\nFootswitch:zz\nMemoryLow:aa\nIsRecording:bb\n" #set template

# TODO: enhance to support MemoryLow and IsRecording...

def getIndex(val, length):
    return val%length

while True:

    # check file for previous settings (allows phone app to also modify...)    
    if os.path.isfile("/root/conf/DIO.config")
        f = open(settings_file,'r')
        settings = f.readlines()
        prevRotaryReading2 = settings[0].split(":",1)[1].strip() # compression
        prevRotaryReading1 = settings[1].split(":",1)[1].strip() # time
        prev_warning = settings[3].split(":",1)[1].strip()
        prev_active = settings[4].split(":",1)[1].strip()
        # ensure good indices
        idxTime = settingsTime.index(prevRotaryReading2)
        idxType = settingsType.index(prevRotaryReading1)
        f.close()
    
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

    # check if we are actively recording (need to blink)
    # NOTE: this functionality may be moved to the circular buffer if needed, this is tentative
    #if 
    
    if recordingFlag == -1: # initiallize IsRecording
        recordingFlag = 0
        temp = temp.replace("bb","No") # write to config
        flag = 1
    
    if settingsTime[getIndex(idxTime,len(settingsTime))] == "active" and activeReady == False:
        activeReady = True
        activeSwitch = currentSwitchState
        
    elif settingsTime[getIndex(idxTime,len(settingsTime))] != "active" and activeReady == True:
        if recordingFlag == 1:
            recordingFlag = 0
            temp = temp.replace("Yes","No") # write to config
            activeCount = 0
            # Set Recording LED to solid ON
            flag = 1
        activeReady = False
        
    elif activeReady == True and activeSwitch != currentSwitchState and recordingFlag == 0:
        recordingFlag = 1
        temp = temp.replace("No","Yes") # write to config
        activeSwitch = currentSwitchState
        flag = 1
    
    elif activeReady == True and activeSwitch != currentSwitchState and recordingFlag == 1:
        recordingFlag = 0
        temp = temp.replace("Yes","No") # write to config
        activeSwitch = currentSwitchState
        activeCount = 0
        # Set Recording LED to solid ON
        flag = 1

    elif recordingFlag == 1:
        activeCount += 1
        if activeCount >= 50:  # number subject to change
            #Blink Recording LED
            activeCount = 0

            
    # check if we have low sd-card memory (need to blink)
    # NOTE: this functionality may be moved to the circular buffer if needed, this is tentative

    if flagSD == -1:
        flagSD = 0
        temp = temp.replace("aa","Open Memory") # write to config
    
    if os.path.ismount(sd_loc):
        #print("discovered sd card")

        df = subprocess.Popen(["df", sd_loc], stdout=subprocess.PIPE)

        output = df.communicate()[0]

        device, size, used, available, percent, mountpoint = \
            output.decode("UTF-8").split("\n")[1].split()

        # uncomment to audit df return
        #print(device, size, used, available, percent, mountpoint)

        if int(available) < 300000 and flagSD == 0:
            #print("warn the user, space available (in sd card) is below 30MB")
            flagSD = 1
            temp = temp.replace("Open Memory","Low Memory") # write to 
            flag = 1
            
        elif int(available) >= 300000 and flagSD == 1:
            #print("space available is fine (for sd card)")
            flagSD = 0
            temp = temp.replace("Low Memory","Open Memory") # write to config
            flag = 1
    
    
    if flag == 1:        
        target = open('/root/conf/DIO.config', 'w')
        target.write(temp)
        target.close()
        flag = 0        
            
