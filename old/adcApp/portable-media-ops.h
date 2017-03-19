// implementation file for generic portable media ops
// for the beaglebone black (wireless)

#include <stdio.h>
#include <stdlib.h>     // For exit() function
#include <dirent.h>     // For directory reading
#include <sys/mount.h>  // To mount devices
#include <string.h>
#include <unistd.h>
#include <sys/statvfs.h> // for file space checking...

// macros to define defaults...
#define USD_DEV_NAME "mmcblk0p1"
#define USB_DEV_NAME "sda1"
#define USD_MNT_LOC "/media/store/sd-card/"
#define USB_MNT_LOC "/media/store/usb-drive/"
#define MOUNT_MODE 1
#define CHECK_MODE 0
#define SPACE_REQUIREMENTS_IN_MB 30

// prototypes for functionality
int CopyToDevice(char* file_to_copy, char* device_name, char* mount_loc);
int DeviceMountCheck(char* dev_name, char* mount_loc, int mode);
int UnmountDevice(char* mount_loc);
int CheckFileSpaceAvailable(char* mount_loc);
