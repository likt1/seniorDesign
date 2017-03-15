// implementation file for generic portable media ops
// for the beaglebone black (wireless)
// all print / log content is tagged with [PMO] representing Portable Media Ops
#include <portable-media-ops.h>

// private prototypes...
int mountDevice(char* dev_name, char* mount_loc, int mode);

int main(int argc, char* argv[])
{
    // example usage of this api

    // this will check for a device, mount if available and not already mounted, then copy a target file to the mount location
    CopyToDevice("/root/hi.txt", USD_DEV_NAME, USD_MNT_LOC);

    // simply check if mounted location has enough space
    if(DeviceMountCheck(USD_DEV_NAME, USD_MNT_LOC, CHECK_MODE) == 0)
    {
        if(CheckFileSpaceAvailable(USD_MNT_LOC) == 0)
            printf("OK, size > 30MB\n");
        else
            printf("WARNING, size < 30MB\n");
    }

    UnmountDevice(USD_MNT_LOC);
    
    if(DeviceMountCheck(USD_DEV_NAME, USD_MNT_LOC, CHECK_MODE) == 0)
        printf("device wasn't unmounted...\n");
}

int CopyToDevice(char* file_to_copy, char* device_name, char* mount_loc)
{
    // printf("[PMO] attempting copy %s to %s\n", file_to_copy, mount_loc);
    FILE *fptr;
    int success = 0;
    
    // check if target device is mounted
    int device_mounted = DeviceMountCheck(device_name, mount_loc, MOUNT_MODE);
    if(device_mounted == 0)
    {
       // check if file to copy is available
       if(access(file_to_copy, F_OK) != -1) 
       {
          // printf("[PMO] %s found\n", file_to_copy);
          char copy_command[100];
          sprintf(copy_command, "cp %s %s", file_to_copy, mount_loc);
          system(copy_command);
       } 
       else 
       {
          printf("[PMO] %s not found\n", file_to_copy);
          success = -1;
       }
    }
    else
       success = -1;   

    return success;
}

int DeviceMountCheck(char* dev_name, char* mount_loc, int mode)
{
    // printf("[PMO] DeviceMountCheck: %s\n", mount_loc);
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
    if(success == 0)
        success = mountDevice(dev_name, mount_loc, mode);
    else if(success < 0)
        printf("[PMO] device not found, please insert a portable storage device\n");
    return success;
}

int UnmountDevice(char* mount_loc)
{
    // printf("[PMO] UnmountDevice %s\n", mount_loc);
    if(umount(mount_loc))
    {
        printf("[PMO] something went wrong while unmounting...\n");
        return -1;
    }
    else
        printf("[PMO] %s has been unmountedi\n", mount_loc);
    return 0;
}

int CheckFileSpaceAvailable(char* mount_loc)
{
    struct statvfs stat;
  
    if (statvfs(mount_loc, &stat) != 0) 
    {
        // error happens, just quits here
        printf("[PMO] error checking file space at %s", mount_loc);
        return -1;
    }

    unsigned int availMB = (stat.f_bsize / 1000 * stat.f_bavail / 1000); 
    printf("[PMO] ~%dMB is available at %s\n", availMB, mount_loc);
    if (availMB > SPACE_REQUIREMENTS_IN_MB)
        return 0;
    return -1;
}

int mountDevice(char* dev_name, char* mount_loc, int mode)
{
    // printf("[PMO] mountDevice %s to %s\n", dev_name, mount_loc);
    char devLocation[25];
    strcpy(devLocation,"/dev/");
    strcat(devLocation,dev_name);
    
    // check if mounted, otherwise attempt mount
    if(system("mount | grep \"store\"") == 0)
    {
        printf("[PMO] device already mounted, good to write...\n");
        return 0;
    }
    else if(mode == 1 && mount(devLocation, mount_loc, "vfat", MS_NOATIME, NULL)) 
    {
        printf("[PMO] something went wrong while mounting...\n");
        return -1;
    } 
    else if(mode == 0)
    {
        // device isn't mounted...
        return -1;
    }
    else
        printf("[PMO] Mount successful\n");
    return 0;
}

