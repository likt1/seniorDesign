import time

# NOTE: requires enable-pwm.py to be run, also need to hook up wires and run
# config-pin <pin_num> pwm
# the valid pin nums are displayed after running enable-pwm

sys_folder = "/sys/class/pwm/"

pwm_1A = sys_folder + "pwmchip4/pwm0/" #B
pwm_2A = sys_folder + "pwmchip6/pwm0/" #G
pwm_2B = sys_folder + "pwmchip6/pwm1/" #R

RGB_1 = [pwm_2B, pwm_2A, pwm_1A] 

# enable and setup each pin
for pwm_pin in RGB_1:
    enable_tmp = open(pwm_pin + "enable", 'w')
    enable_tmp.write("1")
    period_tmp = open(pwm_pin + "period", 'w')
    period_tmp.write("100000")
    enable_tmp.close()
    period_tmp.close()

# test some colors

def setRGB(rgb, red, green, blue):
    red_f = open(rgb[0] + "duty_cycle", 'w')
    green_f = open(rgb[1] + "duty_cycle", 'w')
    blue_f = open(rgb[2] + "duty_cycle", 'w')
    red_f.write(red)
    green_f.write(green)
    blue_f.write(blue)
    red_f.close()
    green_f.close()
    blue_f.close()


# red
setRGB(RGB_1, "50000", "0", "0")
time.sleep(1)

# green
setRGB(RGB_1, "0", "50000", "0")
time.sleep(1)

# blue
setRGB(RGB_1, "0", "0", "50000")
time.sleep(1)

# purple
setRGB(RGB_1, "30000", "1000", "50000")
time.sleep(1)

# orange
setRGB(RGB_1, "40000", "9000", "100")
time.sleep(1)

# yellow
setRGB(RGB_1, "50000", "30000", "0")
time.sleep(1)

# white
setRGB(RGB_1, "50000", "50000", "50000")
time.sleep(1)

