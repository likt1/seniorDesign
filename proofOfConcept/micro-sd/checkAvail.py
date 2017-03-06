import subprocess
import os

sd_loc = "/media/store/sd-card"

if os.path.ismount(sd_loc):
    print("discovered sd card")

    df = subprocess.Popen(["df", sd_loc], stdout=subprocess.PIPE)

    output = df.communicate()[0]

    device, size, used, available, percent, mountpoint = \
        output.decode("UTF-8").split("\n")[1].split()

    # uncomment to audit df return
    #print(device, size, used, available, percent, mountpoint)

    if int(available) < 300000:
        print("warn the user, space available (in sd card) is below 30MB")
    else:
        print("space available is fine (for sd card)")
else:
    print("sd card not mounted")
