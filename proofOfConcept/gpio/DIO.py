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


while (True)
	temp = "CompRotary:xx\nTimeRotary:yy\nFootswitch:zz\n"

	# get rotary value for retro-Time/Active
	currentReading = readRotaryEncoder1 #TODO: determine encoder read command
	if (prevRotaryReading1 == -500000) # if init, set previous as current...
		prevRotaryReading1 = currentReading
		temp.replace(yy,settingsTime[3])
		flag = 1
	
	if (currentReading - prevRotaryReading1 > 10)
		idxTime+=1
		prevRotaryReading1 = settingsTime[idxTime] # write to config
		temp.replace(yy,prevRotaryReading1)
		prevRotaryReading1 = currentReading
		flag = 1
	elif (prevRotaryReading1 - currentReading > 10)
		idxTime-=1
		prevRotaryReading1 = settingsTime[idxTime] # write to config
		temp.replace(yy,prevRotaryReading1)
		prevRotaryReading1 = currentReading
		flag = 1
	
	
	# get rotary value for compression type
	currentReading = readRotaryEncoder2 #TODO: determine encoder read command
	if (prevRotaryReading2 == -500000) # if init, set previous as current...
		prevRotaryReading2 = currentReading
		temp.replace(xx,settingsType[0])
		flag = 1
	
	if (currentReading - prevRotaryReading1 > 10)
		idxTime+=1
		prevRotaryReading2 = settingsType[idxTime] # write to config
		temp.replace(xx,prevRotaryReading2)
		prevRotaryReading2 = currentReading
		flag = 1
	elif (prevRotaryReading2 - currentReading > 10)
		idxTime-=1
		prevRotaryReading2 = settingsType[idxTime] # write to config
		temp.replace(xx,prevRotaryReading2)
		prevRotaryReading2 = currentReading
		flag = 1

	# get latchswitch value for recording
	currentReading = readLatchswitch #TODO: determine switch read command
	if (prevSwitchReading == -1) # if init, set previous as current...
		prevSwitchReading = currentReading
		currentSwitchState = False
		flag = 1
	
	if (prevSwitchReading != currentReading)
		count = 0
		prevSwitchReading  = currentReading
		debounce = not debounce
	elif (debounce == True and prevSwitchReading == currentReading)
		count +=1 #debouncing
		if (count == 5)
			currentSwitchState = not currentSwitchState # write to config
			temp.replace(zz,currentSwitchState)
			count = 0
			debounce = False
			flag = 1
			
	if (flag == 1)		
		target = open("~/conf/DIO.config", 'w')
		target.write(temp)
		target.close()
		flag = 0		
			