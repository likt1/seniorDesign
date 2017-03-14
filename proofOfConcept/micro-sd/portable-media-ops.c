// implementation file for generic portable media ops
// for the beaglebone black (wireless)
// all print / log content is tagged with [PMO] representing Portable Media Ops
#include <portable-media-ops.h>

int main(int argc, char* argv[])
{
    // example usage of this api

    // this will check for a device, mount if available and not already mounted, then copy a target file to the mount location
    CopyToDevice("~/umount.sh", USD_DEV_NAME, USD_MNT_LOC);

    // simply check if mounted location has enough space
    CheckLowFileSpace(USD_MNT_LOC);

    // does what it sounds like
    UnmountDevice(USD_MNT_LOC);
}

int CopyToDevice(char* file_to_copy, char* device_name, char* mount_loc)
{
    printf("[PMO] attempting copy %s to %s\n", mount_loc);
    FILE *fptr;
    int success = 0;
    
    // check if target device is mounted
    int device_mounted = DeviceMountCheck(device_name, mount_loc, MOUNT_MODE);
    if(device_mounted == 0)
    {
       // check if file to copy is available
       if(access(file_to_copy, F_OK) != -1) 
       {
          printf("[PMO] %s found", file_to_copy);
          char copy_command[100];
          sprintf(copy_command, "copy %s %s", file_to_copy, mount_loc);
          system(copy_command);
       } 
       else 
       {
          printf("[PMO] %s not found", file_to_copy);
          success = -1;
       }
    }
    else
       success = -1   

    return success;
}

int DeviceMountCheck(char* dev_name, char* mount_loc, int mode)
{
    printf("[PMO] checking if %s is mounted\n", mount_loc);
    DIR *directory_struct;
    struct dirent *dir;
    directory_struct = opendir("/dev/");
    int success = -1;
    if(directory_struct)
    {
        while((dir = readdir(directory_struct)) != NULL)
        {
            if(strstr(dir->d_name,dev_name))
                success = 0;
        }
        closedir(directory_struct);
    }
    if(success == 0 && mode == 1)
        success = MountDevice(dev_name, mount_loc);
    else if(success < 0)
        printf("[PMO] device not found, please insert a portable storage device\n");
    return success;
}

int MountDevice(char* dev_name, char* mount_loc)
{
    printf("[PMO] attempting mount of %s to %s\n", dev_name, mount_loc);
    char devLocation[25];
    strcpy(devLocation,"/dev/");
    strcat(devLocation,dev_name);
    
    // check if mounted, otherwise attempt mount
    if(system("mount | grep \"store\"") == 0)
        printf("[PMO] device already mounted, good to write...\n");
    else if(mount(devLocation, mount_loc, "vfat", MS_NOATIME, NULL)) 
    {
        printf("[PMO] something went wrong while mounting...\n");
        return -1;
    } 
    else
        printf("[PMO] Mount successful\n");
    return 0;
}

int UnmountDevice(char* mount_loc)
{
    printf("[PMO] attempting umount at %s\n", mount_loc);
    if(umount(mount_loc))
    {
        printf("[PMO] something went wrong while unmounting...\n");
        return -1;
    }
    return 0;
}

int CheckLowFileSpace(char* mount_loc)
{
  // run df -h, check if less than 30MB available, return 1 if so, else 0
  struct statvfs stat;
  
  if (statvfs(mount_loc, &stat) != 0) 
  {
    // error happens, just quits here
    return -1;
  }

  int avail = stat.f_bsize * stat.f_bavail;
  printf("[PMO] %d is available at %s", avail, mount_loc);

  return 0;
}

