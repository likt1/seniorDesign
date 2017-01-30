#include <stdio.h>
#include <stdlib.h>  // For exit() function
#include <dirent.h>  // For directory reading
#include <sys/mount.h> // To mount devices
#include <string.h>  // For string ops...

#define DEV_NAME "mmcblk0p1"
#define MOUNT_LOC "/media/store/"
#define FILE_NAME "sd-card-write-test.txt"

int device_mount_check();
int mount_device();
int unmount_device();

int main()
{
   char sentence[1000];
   FILE *fptr;

   int device_mounted = device_mount_check();
   if(device_mounted == 0)
   {
       char fileLocation[50];
       strcpy(fileLocation,MOUNT_LOC);
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

   unmount_device();

   return 0;
}

int device_mount_check()
{
	DIR *directory_struct;
	struct dirent *dir;
	directory_struct = opendir("/dev/");
    int success = -1;
	if(directory_struct)
	{
		while((dir = readdir(directory_struct)) != NULL)
		{
			if(strstr(dir->d_name,DEV_NAME))
				success = 0;
    	}
		closedir(directory_struct);
	}
	if(success == 0)
		success = mount_device();
	else
		printf("device not found, please insert a micro-sd card\n");
	return(success);
}

int mount_device()
{
	char devLocation[25];
	strcpy(devLocation,"/dev/");
	strcat(devLocation,DEV_NAME);
	
	// check if mounted, otherwise attempt mount
	if(system("mount | grep \"store\"") == 0)
		printf("device already mounted, good to write...\n");
	else if(mount(devLocation, MOUNT_LOC, "vfat", MS_NOATIME, NULL)) 
	{
		printf("something went wrong while mounting...\n");
		return -1;
	} 
	else
    	printf("Mount successful");
	return 0;
}

int unmount_device()
{
	if(umount(MOUNT_LOC))
	{
		printf("something went wrong while unmounting...\n");
		return -1;
	}
	return 0;
}
