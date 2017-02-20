#include <stdio.h>
#include <stdlib.h>     // For exit() function
#include <dirent.h>     // For directory reading
#include <sys/mount.h>  // To mount devices
#include <string.h>

// macros to define defaults...
#define USD_DEV_NAME "mmcblk0p1"
#define USB_DEV_NAME "sda1"
#define USD_MNT_LOC "/media/store/sd-card/"
#define USB_MNT_LOC "/media/store/usb-drive/"
#define FILE_NAME "storage-write-test.txt"

// prototypes for functionality
void usage();
int write_device(char* file_name, char* to_write, char* device_name, char* mount_loc);
int device_mount_check(char* dev_name, char* mount_loc);
int mount_device(char* dev_name, char* mount_loc);
int unmount_device(char* mount_loc);

int main(int argc, char* argv[])
{
    // initiate vars to use during prog
   char functionSelector = '0';
   char file_name[50];
   char to_write[100];
   char device_name[50];
   char mount_loc[50];
   
   // ensure strings are cleared...
   memset(file_name, 0, 50);
   memset(to_write, 0, 100);
   memset(device_name, 0, 50);
   memset(mount_loc, 0, 50);
   
   // fill out defaults
   strcpy(file_name, FILE_NAME);
   strcpy(to_write, "default to write content...");
   strcpy(device_name, USD_DEV_NAME);
   strcpy(mount_loc, USD_MNT_LOC);
   
   // parse args
   if (argc-1 < 1 || argc-1 > 5 )
       printf("error: found %d args, was expecting 1 to 5...\n", argc-1);
   else
   {
       int i = 1; // skip <executable_name>
       for(; i < argc; i++)
       {
           size_t len = strlen(argv[i]);
           
           if (len <= 1)
           {
                printf("error: arg too small (%s)...\n", argv[i]);
                continue;
           }
           
           // check if it is a function arg
           if (argv[i][0] == '-')
           {
                if (len > 2)
                    printf("arg too big (%s)...\n", argv[i]);
                else
                    functionSelector = argv[i][1];
           }
           // check if it is an info arg
           else if (argv[i][1] == '=')
           {
               // put the info into the appropriate var
               switch(argv[i][0])
               {
                   case 'f':
                    sprintf(file_name, "%s", argv[i] + 2);
                    break;
                   case 's':
                    sprintf(to_write, "%s", argv[i] + 2);
                    break;
                   case 'd':
                    sprintf(device_name, "%s", argv[i] + 2);
                    break;
                   case 'm':
                    sprintf(mount_loc, "%s", argv[i] + 2);
                    break;
                   default:
                    printf("unrecognized info arg: %s\n",argv[i]);
               }
           }
       }
   }
   
   // (debug only) audit arg contents after parse
   /*printf("var contents:\n\tselector = %c\n\tfname = %s\n\tsentence = %s\n\tdevice_name = %s\n\tmount_loc = %s\n", 
    functionSelector, file_name, to_write, device_name, mount_loc);*/
   
   switch (functionSelector)
   {
       case 'w':
        write_device(file_name, to_write, device_name, mount_loc);
        break;
       case 'm':
        device_mount_check(device_name, mount_loc);
        break;
       case 'u':
        unmount_device(mount_loc);
        break;
       default:
        usage();
        break;
   }
   
   return 0;
}

void usage()
{
    printf("Help: Portable Media Operations Tool:\n");
    printf("Designed to assist with various operations on portable media (i.e. mount sd-cards, usb etc.)\n");
    printf("Usage: ./main <operation> f=<target_file_name> s=<to_write> d=<device_location> m=<mount_location>\n");
    printf("\t<operation>:\n\t\t-w: write sentence to a file\n\t\t-m: mount device\n\t\t-u: unmount device\n\t\t-h: help\n");
    printf("\tf=<target_file_name>: file to write to, only applicable for -w\n");
    printf("\ts=<to_write>: sentence to write, only applicable for -w\n");
    printf("\td=<device_location>: device to mount (defaults to mmcblk0p1 (sd), otherwise sda1 (usb), otherwise errors out)\n");
    printf("\tm=<mount_location>: mount location (defaults to /media/store/sd-card or /media/store/usb depending)\n");
    printf("\ti.e. ./main -w f=filename s='sentence to write' d=devicename m=/mount/location\n");
}

int write_device(char* file_name, char* to_write, char* device_name, char* mount_loc)
{
    printf("attempting write to %s\n", file_name);
    device_mount_check(device_name,mount_loc);
    printf("writing: %s\n", to_write);
    /*
    FILE *fptr;
    
    int device_mounted = device_mount_check(device_name, mount_loc);
    if(device_mounted == 0)
    {
       char fileLocation[50];
       strcpy(fileLocation,mount_loc);
       strcat(fileLocation,file_name);
       fptr = fopen(fileLocation, "w");
       if(fptr == NULL)
       {
          printf("Error while opening the output file!\n");
          return -1;
       }
    }
    else
       return -1; // error out if unable to find the device / mount point    
    
    fprintf(fptr,"%s\n", to_write);
    fclose(fptr);*/
    
    return 0;
}

int device_mount_check(char* dev_name, char* mount_loc)
{
    printf("checking if %s is mounted\n", mount_loc);
    /*DIR *directory_struct;
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
        success = mount_device(dev_name, mount_loc);
    else
        printf("device not found, please insert a portable storage device\n");
    return(success);*/
    return mount_device(dev_name, mount_loc);
}

int mount_device(char* dev_name, char* mount_loc)
{
    printf("attempting mount of %s to %s\n", dev_name, mount_loc);
    /*char devLocation[25];
    strcpy(devLocation,"/dev/");
    strcat(devLocation,dev_name);
    
    // check if mounted, otherwise attempt mount
    if(system("mount | grep \"store\"") == 0)
        printf("device already mounted, good to write...\n");
    else if(mount(devLocation, mount_loc, "vfat", MS_NOATIME, NULL)) 
    {
        printf("something went wrong while mounting...\n");
        return -1;
    } 
    else
        printf("Mount successful");*/
    return 0;
}

int unmount_device(char* mount_loc)
{
    printf("attempting umount at %s\n", mount_loc);
    /*if(umount(mount_loc))
    {
        printf("something went wrong while unmounting...\n");
        return -1;
    }*/
    return 0;
}

