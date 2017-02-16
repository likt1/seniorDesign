#include <stdio.h>
#include <stdlib.h>  // For exit() function
#include <dirent.h>  // For directory reading
#include <sys/mount.h> // To mount devices
#include <string.h>  // For string ops...

#define USD_DEV_NAME "mmcblk0p1"
#define USB_DEV_NAME "sda1"
#define USD_MNT_LOC "/media/store/sd-card/"
#define USB_MNT_LOC "/media/store/usb-drive/"
#define FILE_NAME "storage-write-test.txt"

int device_mount_check(char* dev_name, char* mount_loc);
int mount_device(char* dev_name, char* mount_loc);
int unmount_device(char* mount_loc);

int main()
{
   char sentence[1000];
   FILE *fptr;

   int device_mounted = device_mount_check(USB_DEV_NAME, USB_MNT_LOC);
   if(device_mounted == 0)
   {
       char fileLocation[50];
       strcpy(fileLocation,USB_MNT_LOC);
       strcat(fileLocation,FILE_NAME);
       fptr = fopen(fileLocation, "w");
       if(fptr == NULL)
       {
          printf("Error while opening the output file!\n");
          return -1;
       }
   }
   else
       return -1; // error out if unable to find the device / mount point	
   
   printf("Enter a sentence:\n");
   fgets(sentence, sizeof(sentence),stdin);

   fprintf(fptr,"%s\n", sentence);
   fclose(fptr);

   unmount_device(USB_MNT_LOC);

   return 0;
}

int device_mount_check(char* dev_name, char* mount_loc)
{
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
		success = mount_device(dev_name, mount_loc);
	else
		printf("device not found, please insert a portable storage device\n");
	return(success);
}

int mount_device(char* dev_name, char* mount_loc)
{
	char devLocation[25];
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
    	printf("Mount successful");
	return 0;
}

int unmount_device(char* mount_loc)
{
	if(umount(mount_loc))
	{
		printf("something went wrong while unmounting...\n");
		return -1;
	}
	return 0;
}
