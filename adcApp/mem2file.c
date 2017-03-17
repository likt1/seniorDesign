#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "circularBuffer.h"

// Opens up file and parses value in hex
word readFileVal(char filenm[]) {
  FILE* fp;
  word value = 0;
  fp = fopen(filenm, "rt");
  fscanf(fp, "%x", &value);
  fclose(fp);
  return value;
}

void main (void) {
  // Global init
  word PRU0RamAddrOff;
  PRU0RamAddrOff = readFileVal(PRU0MAP_LOC "addr");
  
  bool youAreAFailure = false;
  bool mapAccess = true;

  int fd = open("/dev/mem", O_RDONLY);
  if (fd == -1) {
    printf("Failed to open ram to fetch adc values.\n");
    mapAccess = false;
  }
  printf("fd:%d ", fd);
  
  off_t mapLoc;
  mapLoc = PRU0RamAddrOff;
  printf("mapLoc:0x%X ", mapLoc);
  
  void *map_base;
  map_base = mmap(0, MAP_SIZE, PROT_READ, MAP_SHARED, fd, mapLoc);
  if (map_base == (void *) -1) {
    printf("Failed to map memory when accessing ram 0x%X.\n", mapLoc);
    mapAccess = false;
  }
  printf("map_base:0x%X\n", map_base);
  
  if (fd) {
    close(fd);
  }
  
  halfword samples[PRU_SAMPLES_NUM]; // Init sample var
  void *virt_addr; 
  
  int i;
  for (i = 0; i < PRU_SAMPLES_NUM && mapAccess; i++) { // For each sample in pru buffer if we have access
    // Get samples
    off_t buffOff = sizeof(struct locals) + i*2;
    
    if (mapAccess) { // Grab sample
      virt_addr = map_base + (buffOff); //& MAP_MASK);
      samples[i] = *((halfword *) virt_addr);
      if (samples[i] != 0xfff) { //DEBUG
        printf("Debug failed at access:0x%X sample:0x%X virt_addr:0x%X\n", buffOff, samples[i], virt_addr);
        youAreAFailure = true;
      }
    }
        
    if (i == PRU_SAMPLES_NUM - 1) {
      printf("i:%d addr:0x%X virt_addr:0x%X\n", i, buffOff, virt_addr);
    }
    
    // Upscale to 16bit from 12bit
    //sampleBuffer[next] = sample * 16;
    //next++;
    //if (next == BUFFER_SIZE) {
    //  next = 0;
    //}
    //if (next == start) {
    //  save = true;
    //  noop = true;
    //  break;
    //}
  }
  
  fd = open("/root/temp_mem", O_WRONLY | O_CREAT);
  if (fd == -1) {
    printf("Failed to open temp mem file to save adc values.\n");
  }
  else {
    if (write(fd, samples, sizeof(samples)) < 0) {
      printf("Writing to file failed horribly and you should feel bad");
    }
  }
  
  if (map_base != (void *) -1) {
    munmap(map_base, MAP_SIZE);
  }
  
  if (youAreAFailure) {
    printf("There were errors yo\n");
  }
}
