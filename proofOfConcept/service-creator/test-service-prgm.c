// Program to test file read / write at same time by 2 different executions
// Usage (separate terminals): gcc test-service-prgm.c -o logger
//    ./logger r
//    ./logger a
//    ./logger w
// Usage (service check): gcc test-service-prgm.c -o test-service-read
//    mkdir /opt/service-prgms/
//    mv test-service-read /opt/service-prgms/
//    python3 service-creator.py
//      test-service-read
//      "/opt/service-prgms/test-service-read r"
//      root
//      simple service to read from one file (/home/vagrant/service-test.log) to another (/home/vagrant/test-service-read.log)
//    #then read from / write to the above locations to validate the service is running
//    see service-creator.py for using the new service 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// custom macros to configure log location and log content
#define DIR_LOC "/home/vagrant/"
#define FILE_NAME "service-test.log"
#define TO_LOG(arg) "hello from " #arg "\n"
#define BUFFER_SIZE 50

// prototypes for generic file IO
int WriteFile(char* file, char* text);
int AppendFile(char* file, char* text);
int ReadFile(char* file, char* buffer);
void ClearString(char* buffer);

// this program will be run as a service to dynamically read and write from a file
// to be run in tandem with another instance of the file (validate read / write to same file)
int main(int argc, char *argv[])
{
    char fileLocation[BUFFER_SIZE];
    strcpy(fileLocation,DIR_LOC);
    strcat(fileLocation,FILE_NAME);

    char readLog[BUFFER_SIZE];
    strcpy(readLog,DIR_LOC);
    strcat(readLog,"test-service-read.log");

    char* buffer;
    buffer = (char*)malloc(BUFFER_SIZE);

    while(1)
    {
        ClearString(buffer);
        
        // based on if reading, writing or appending: stagger sleep so we can see
        // that both are interacting with the file without breaking...
	if (argc >= 2)
        {
            if (argv[1][0] == 'a') 
            {
                sleep(2);
                AppendFile(fileLocation, TO_LOG(appended));
            } 
            else if (argv[1][0] == 'w')
            {
                sleep(3);
                WriteFile(fileLocation, TO_LOG(overwritten));   
            } else
            {
                sleep(1);
                ReadFile(fileLocation, buffer);
                AppendFile(readLog, buffer); // continuously log most recent entry 
            }
        } 
        else {
            sleep(0.75);
            WriteFile(fileLocation, TO_LOG(overwritten));
        }
    }

    free(buffer);
    return 0;
}

void ClearString(char* buffer)
{
    memset(buffer,0,sizeof(buffer));
}

int WriteFile(char* fileLocation, char* textToWrite)
{
    FILE *fptr = fopen(fileLocation, "w");
    if(fptr == NULL)
    {
       printf("Error while opening the file to write!\n");
       return -1;
    }

    fprintf(fptr,"%s", textToWrite);

    return fclose(fptr);
}

int AppendFile(char* fileLocation, char* textToWrite)
{
    FILE *fptr = fopen(fileLocation, "a");
    if(fptr == NULL)
    {
       printf("Error while opening the file to write!\n");
       return -1;
    }

    fprintf(fptr,"%s", textToWrite);

    return fclose(fptr);
}

int ReadFile(char* fileLocation, char* buffer)
{
    // this current code interates through all lines in a file, but only prints the last
    // the while loop could be modified to do further action upon each line...

    FILE *fptr = fopen(fileLocation, "r");
    if(fptr == NULL)
    {
       printf("Error while opening the file to read!\n");
       return -1;
    }

    // get most recent line in file
    char tmp[BUFFER_SIZE];
    while(fgets(buffer, BUFFER_SIZE, fptr) != NULL)
        strcpy(tmp, buffer);
    strcpy(buffer, tmp);

    return fclose(fptr);
}

