# to effectively setup pwm for our RGB LEDS in terminal

# these will be done by running both python scripts...
# echo 1 > /sys/class/pwm/<chip_dir>/<pwm_dir>/enable
# echo 100000 > /sys/class/pwm/<chip_dir>/<pwm_dir>/period
# echo 50000 > /sys/class/pwm/<chip_dir>/<pwm_dir>/duty_cycle

# these need to be done before running the python scripts
config-pin P8.13 pwm # red on LED1: 2B (pwmchip6/pwm1)
config-pin P8.19 pwm # green on LED1: 2A (pwmchip6/pwm0)
config-pin P9.14 pwm # blue on LED1: 1A (pwmchip4/pwm0)

config-pin P9.16 pwm # red on LED2: 1B (pwmchip4/pwm1)
config-pin P9.21 pwm # green on LED2: 0B (pwmchip2/pwm1)
config-pin P9.22 pwm # blue on LED2: 0A (pwmchip2/pwm0)
