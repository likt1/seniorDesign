import time

# NOTE: requires enable-pwm.py to be run, also need to hook up wires and run
# config-pin <pin_num> pwm
# the valid pin nums are displayed after running enable-pwm

# store some rgb color presets
off = [0, 0, 0]
red = [255, 0, 0]
green = [0, 255, 0]
blue = [0, 0, 255]
purple = [158, 23, 256]
orange = [255, 140, 0]
yellow = [255, 255, 0]
white = [255, 255, 255]

# known settings according to DIO.py
color_dict_1 = {"wav": red, "mp3": orange, "flac": yellow, "ogg": green}
color_dict_2 = {"active": red, "30s": orange, "1m": yellow, "1m30s": green, "2m": blue, "2m30s": purple, "3m": white}
settings_file = "/root/conf/DIO.config"

# setup each pin
sys_folder = "/sys/class/pwm/"

pwm_1A = sys_folder + "pwmchip4/pwm0/" #B
pwm_2A = sys_folder + "pwmchip6/pwm0/" #G
pwm_2B = sys_folder + "pwmchip6/pwm1/" #R
RGB_1 = [pwm_2B, pwm_2A, pwm_1A] 

pwm_0A = sys_folder + "pwmchip2/pwm0/" #B
pwm_0B = sys_folder + "pwmchip2/pwm1/" #G
pwm_1B = sys_folder + "pwmchip4/pwm1/" #R
RGB_2 = [pwm_1B, pwm_0B, pwm_0A]

# enable and setup each pin
for RGB_LED in [RGB_1, RGB_2]:
    for pwm_pin in RGB_LED:
        enable_tmp = open(pwm_pin + "enable", 'w')
        enable_tmp.write("1")
        period_tmp = open(pwm_pin + "period", 'w')
        period_tmp.write("100000")
        enable_tmp.close()
        period_tmp.close()

def setRGB(rgb_led, rgb, debug=False):
    scale = 50000 // 255 # scale pwm writing according to standard rgb
    # max pwm write = 50000, max rgb = 255
    red_f = open(rgb_led[0] + "duty_cycle", 'w')
    green_f = open(rgb_led[1] + "duty_cycle", 'w')
    blue_f = open(rgb_led[2] + "duty_cycle", 'w')
    red_f.write(str(rgb[0] * scale))
    green_f.write(str(rgb[1] * scale))
    blue_f.write(str(rgb[2] * scale))
    red_f.close()
    green_f.close()
    blue_f.close()
    if debug:
        time.sleep(1)

# test some colors
def testLEDs():
    setRGB(RGB_1, red, True)
    setRGB(RGB_1, green, True)
    setRGB(RGB_1, blue, True)
    setRGB(RGB_1, purple, True)
    setRGB(RGB_1, orange, True)
    setRGB(RGB_1, yellow, True)
    setRGB(RGB_1, white, True)
    #setRGB(RGB_1, off)
    setRGB(RGB_2, red, True)
    setRGB(RGB_2, green, True)
    setRGB(RGB_2, blue, True)
    setRGB(RGB_2, purple, True)
    setRGB(RGB_2, orange, True)
    setRGB(RGB_2, yellow, True)
    setRGB(RGB_2, white, True)
    #setRGB(RGB_2, off)

#testLEDs()

# for blinking...
counter_size = 1#200
active_count = counter_size
active_toggle = 1
warning_count = counter_size
warning_toggle = 1

while True:
    f = open(settings_file,'r')
    settings = f.readlines()
    if len(settings) == 0:
        continue # expect timing issues...

    # debug purposes:
    if len(settings) < 6:
        print("error, file read contained", len(settings), "settings")
        print("dump = ", settings)
    for line in settings:
        if "DIO" in line:
            continue
        s = line.split("=")
        if len(s) < 2: 
            print("error, setting was too short")
            print("dump = ", s)

    compression_setting = settings[1].split("=",1)[1].strip()
    time_setting = settings[2].split("=",1)[1].strip()
    warning = settings[4].split("=",1)[1].strip()
    active_recording = settings[5].split("=",1)[1].strip()
    f.close()
    #print(compression,time,warning,active)
    #print(color_dict_1[compression_setting], color_dict_2[time_setting], warning, active)
    
    if warning == "True" and warning_toggle == 0:
        setRGB(RGB_1, off)
    else:
        setRGB(RGB_1, color_dict_1[compression_setting])
    
    if active_recording == "True" and active_toggle == 0:
        setRGB(RGB_2, off)
    else:
        setRGB(RGB_2, color_dict_2[time_setting])

    # check & count for toggling...
    if time_setting == "active" and active_recording == "True" and active_count == 0:
        active_count = counter_size
        active_toggle = 1 - active_toggle
    elif active_recording == "True":
        active_count -= 1

    if warning == "True" and warning_count == 0:
        warning_count = counter_size
        warning_toggle = 1 - warning_toggle
    elif warning == "True":
        warning_count -= 1
    
    time.sleep(0.2)
