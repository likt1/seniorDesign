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


while True:
	temp = "CompRotary:xx\nTimeRotary:yy\nFootswitch:zz\n"

	# get rotary value for retro-Time/Active
	target = open('/sys/devices/platform/ocp/48304000.epwmss/48304180.eqep/position', 'r')
	currentReading=int(target.read())
	target.close()

	if prevRotaryReading1 == -500000: # if init, set previous as current
		prevRotaryReading1 = currentReading
		temp = temp.replace("yy",settingsTime[3])
		flag = 1
	
	if (currentReading - prevRotaryReading1) > 10:
		idxTime+=1
		prevRotaryReading1 = settingsTime[idxTime] # write to config
		temp = temp.replace("yy",prevRotaryReading1)
		prevRotaryReading1 = currentReading
		flag = 1
	elif (prevRotaryReading1 - currentReading) > 10:
		idxTime-=1
		prevRotaryReading1 = settingsTime[idxTime] # write to config
		temp = temp.replace("yy",prevRotaryReading1)
		prevRotaryReading1 = currentReading
		flag = 1
	
	
	# get rotary value for compression type
	target = open('/sys/devices/platform/ocp/48304000.epwmss/48304180.eqep/position' , 'r')
        currentReading=int(target.read())
        target.close()

	if prevRotaryReading2 == -500000: # if init, set previous as current
		prevRotaryReading2 = currentReading
		temp = temp.replace("xx",settingsType[0])
		flag = 1
	
	if (currentReading - prevRotaryReading1) > 10:
		idxTime+=1
		prevRotaryReading2 = settingsType[idxTime] # write to config
		temp = temp.replace("xx",prevRotaryReading2)
		prevRotaryReading2 = currentReading
		flag = 1
	elif (prevRotaryReading2 - currentReading) > 10:
		idxTime-=1
		prevRotaryReading2 = settingsType[idxTime] # write to config
		temp = temp.replace("xx",prevRotaryReading2)
		prevRotaryReading2 = currentReading
		flag = 1

	# get latchswitch value for recording
	#currentReading = readLatchswitch TODO: determine switch read command
	if prevSwitchReading == -1: # if init, set previous as current
		prevSwitchReading = currentReading
		currentSwitchState = False
		flag = 1
		temp = temp.replace("zz",str(currentSwitchState))
	
	if prevSwitchReading != currentReading:
		count = 0
		prevSwitchReading  = currentReading
		debounce = not debounce
	elif debounce == True and prevSwitchReading == currentReading:
		count +=1 #debouncing
		if count == 5:
			currentSwitchState = not currentSwitchState # write to config
			temp = temp.replace("zz",str(currentSwitchState))
			count = 0
			debounce = False
			flag = 1
			
	if flag == 1:		
		target = open('/root/conf/DIO.config', 'w')
		target.write(temp)
		target.close()
		flag = 0		
			
