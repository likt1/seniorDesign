#include <stdio.h>
#include <string.h>  // For string ops...

// custom macros to configure log location and log content
#define DIR_LOC "/home/kevin/"
#define FILE_NAME "poc-service-test.log"
#define TO_LOG "hello\n"

int main()
{
   FILE *fptr;
   char fileLocation[50];
   strcpy(fileLocation,DIR_LOC);
   strcat(fileLocation,FILE_NAME);
   fptr = fopen(fileLocation, "w");
   if(fptr == NULL)
   {
      printf("Error while opening the output file!\n");
      return -1;
   }
   
   fprintf(fptr,"%s\n", TO_LOG);
   
   fclose(fptr);

   return 0;
}
