import os
import glob
from collections import OrderedDict
from pprint import pprint

os.chdir('/sys/class/pwm/')
chips = glob.glob('pwmchip*')

pwms = ['PWM0A', 'PWM0B', 'PWM1A', 'PWM1B', 'PWM2A', 'PWM2B']
nbpwm = 0
pwm_dict = OrderedDict()

for chip in chips:
    npwm = int(open('{}/npwm'.format(chip)).read())
    if npwm == 2:
        for i in [0, 1]:
            path = '/sys/class/pwm/{}/'.format(chip)
            open(path+'export', 'w').write(str(i))
            pwm_dict[pwms[nbpwm+i]] = '{}/pwm{}/'.format(path, i)
        nbpwm += 2

pprint(pwm_dict)
