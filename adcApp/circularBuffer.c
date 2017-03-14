#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#include "circularBuffer.h"

// Init globals
bool run = true;
bool noop = false;
bool save = false;
pthread_mutex_t stop;
pthread_mutex_t pruWrite;
int next = 0;
int start = -1;
halfword *sampleBuffer;
word PRU0RamAddrOff;

// Opens up file and parses value in hex
word readFileVal(char filenm[]) {
  FILE* fp;
  word value = 0;
  fp = fopen(filenm, "rt");
  fscanf(fp, "%x", &value);
  fclose(fp);
  return value;
}

void *pruThread (void *var) {
  // INIT
  // ===============================
  printf("pru Thread active\n");
  struct locals PRU_local;
  int r;

  // Allocate and init mem
  r = prussdrv_init();
  if (r != 0) {
    printf("Failed to init prussdrv driver\n");
    return NULL;
  }
  r = prussdrv_open(PRU_EVTOUT_0);
  if (r != 0) {
    printf("Failed to open PRU %d\nerror:%d\n", PRU_EVTOUT_0, r);
    prussdrv_exit();
    return NULL;
  }

  // Map PRU's interrupts
  tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
  r = prussdrv_pruintc_init(&pruss_intc_initdata);
  if (r != 0) {
    printf("Failed to init interrupts\n");
    prussdrv_exit();
    return NULL;
  }

  // Load memory
  PRU_local.samples.addr = sizeof(PRU_local);
  PRU_local.samples.offset = 0;
  PRU_local.samples.length = 13230; // 3 * 4410 or 3/10 a sec idealy
  
  PRU_local.cap_delay = 0;
  PRU_local.timer = 0;
  r = prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, (word *)&PRU_local, sizeof(PRU_local));
  if (r < 0) {
    printf("Failed to write memory block\n");
    prussdrv_exit();
    return NULL;
  }

  printf("size:%d obj:%d\n", PRU_local.samples.addr, sizeof(PRU_local));
  // Load and execute the PRU program on PRU
  prussdrv_exec_program(PRU_NUM, "adcSample.bin");
  // ===============================

  // MAIN PRU LOOP
  // ===============================
  while (true) {
    // Wait for even compl from PRU, returns PRU_EVTOUT_0 num
    //printf("Waiting for PRU\n");
    r = prussdrv_pru_wait_event(PRU_EVTOUT_0);
    printf("PRU returned, event number %d.\n", r);
    prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
    
     // Write to buffer
    pthread_mutex_lock(&pruWrite);
    if (!noop) {
      int i;
      for (i = 0; i < 0; i++) { // For each sample in pru buffer
        halfword sample = 0; // Get sample from pru buffer TODO HERE
        sampleBuffer[next] = sample * 16;
        next++;
        if (next == BUFFER_SIZE) {
          next = 0;
        }
        if (next == start) {
          save = true;
          noop = true;
          break;
        }
      }
    }
    pthread_mutex_unlock(&pruWrite);

    // Check to see if we should stop
    pthread_mutex_lock(&stop);
    if (!run) {
      break;
    }
    pthread_mutex_unlock(&stop);

    // Continue PRU sampling
    prussdrv_pru_send_event(ARM_PRU0_INTERRUPT);
    prussdrv_pru_clear_event(PRU_EVTOUT_0, ARM_PRU0_INTERRUPT);
  }
  // ===============================

  // Disable PRU and close memory mappings
  prussdrv_pru_disable(PRU_NUM);
  prussdrv_exit();

  printf("pru Thread stopped\n");
  return NULL;
}

void buffer (void) {
  printf("Circular Buffer program start\n");

  // INIT
  // ===============================
  bool running = true;
  struct configs curConfig; // TODOM set defaults?
  curConfig.footSwitch = false;
  strncpy(curConfig.timeRotary, "\0", CONFIG_SIZE);
  strncpy(curConfig.compRotary, "\0", CONFIG_SIZE);
  
  struct timespec sleepTime = {0, 10000000}; // sleep for 10 ms
  
  // Init mutex
  if (pthread_mutex_init(&stop, NULL) != 0) {
    printf("Failed to init mutex stop\n");
    return;
  }
  if (pthread_mutex_init(&pruWrite, NULL) != 0) {
    printf("Failed to init mutex pruWrite\n");
    return;
  }
  
  // Init PRU
  pthread_t threadID;
  if (pthread_create(&threadID, NULL, pruThread, NULL) != 0) {
    printf("Failed to init pru thread obj\n");
  }
  // ===============================
  
  // MAIN CONFIG FILE LOOP 
  // ===============================
  int numEpochs = 500;
  while (running) {
    if (numEpochs < 0) {
      running = false; // DEBUG
    }
    numEpochs--;
    // save = true; // DEBUG
    /* int i; // DEBUG
    for (i = 0; i < BUFFER_SIZE; i++) {
      sampleBuffer[i] = 4095;
    } // DEBUG */
    
    // Read config file and set values
    // Init file read vars
    FILE *file = fopen("/root/conf/DIO.config", "r");
    char strBuf[40];
    char* lbl;
    char* val;
    const char delim[2] = ":";
    
    // Init config file val vars
    struct configs newConfig;
    newConfig.footSwitch = false;
    strncpy(newConfig.timeRotary, "\0", CONFIG_SIZE);
    strncpy(newConfig.compRotary, "\0", CONFIG_SIZE);
    
    if (file) {
      //printf("Config file detected...\n");
      while (!(fscanf(file, "%s", strBuf) == EOF)) {
        //printf("Entering loop: %s\n", strBuf);
        lbl = strtok(strBuf, delim); // Start of label
        val = strtok(NULL, delim); // Value

        //printf("lbl: %s val: %s\n", lbl, val);
        if (strcmp(lbl, "CompRotary") == 0) {
          strncpy(newConfig.compRotary, val, CONFIG_SIZE);
        }
        else if (strcmp(lbl, "TimeRotary") == 0) {
          strncpy(newConfig.timeRotary, val, CONFIG_SIZE);
        }
        else if (strcmp(lbl, "Footswitch") == 0) {
          if (strcmp(val, "True") == 0) {
            newConfig.footSwitch = true;
          }
        }
      }
      fclose(file); /*
      printf("%s\n", newConfig.compRotary);
      printf("%s\n", newConfig.timeRotary);
      printf("%d\n", newConfig.footSwitch);*/
    }
    
    // Check to see we got stuff
    if (strlen(newConfig.timeRotary) == 0) {
      printf("Empty new time rotary string!\n");
    }
    if (strlen(newConfig.compRotary) == 0) {
      printf("Empty new comppression rotary string!\n");
    }
    
    // Block write thread to check for save switching?
    pthread_mutex_lock(&pruWrite);
    
    // Handle toggle of footswitch
    if (newConfig.footSwitch != curConfig.footSwitch) {
      if (start = -1 && strcmp(newConfig.timeRotary, "active") == 0) { // If we are active and we are not started 
        start = next; // start active
      }
      else { // save buffer
        save = true;
      }
    }
    
    // Handle switching from active to retroactive if we are running
    if (start > 0 && strcmp(newConfig.timeRotary, curConfig.timeRotary) != 0) {
      //save = true; // saves sample
      start = -1; // stops active recording
    }
    
    // If we are saving
    if (save) {
      // Open file
      file = fopen("/root/testOut.raw", "wb");
      printf("Saving output file\n");
      if (file) {
        // Set write head start
        if (start == -1) { // if passive get prev
          if (strcmp(newConfig.timeRotary, "30s") == 0) {
            start = next - 30*SAMPLE_RATE;
          }
          else if (strcmp(newConfig.timeRotary, "1m") == 0) {
            start = next - 60*SAMPLE_RATE;
          }
          else if (strcmp(newConfig.timeRotary, "1m30s") == 0) {
            start = next - 90*SAMPLE_RATE;
          }
          else if (strcmp(newConfig.timeRotary, "2m") == 0) {
            start = next - 120*SAMPLE_RATE;
          }
          else if (strcmp(newConfig.timeRotary, "2m30s") == 0) {
            start = next - 150*SAMPLE_RATE;
          }
          //else { // full 3 min
          if (start < 0) { // DEBUG MAX MINUTES ONLY
            start = next;
          }
          
          if (start < 0) { // Handle negative numbers
            start += BUFFER_SIZE;
          }
        }
      
        // Write until we meet next (end)
        do {
          // write to file TODOM improve one by one?
          fwrite(&sampleBuffer[start], sizeof(halfword), 1, file); // one by one, find out a way to save all
          //printf("%d\n%d\n", start, &sampleBuffer[start]);
          start++;
          if (start == BUFFER_SIZE) {
            start = 0;
          }
        } while (start != next);
      
        // Reset
        fclose(file);
        noop = false;
        save = false;
        start = -1;
      }
      else {
        printf("Could not open file for write\n");
      }
    }
    pthread_mutex_unlock(&pruWrite);
    
    curConfig.footSwitch = newConfig.footSwitch;
    strncpy(curConfig.timeRotary, newConfig.timeRotary, CONFIG_SIZE);
    strncpy(curConfig.compRotary, newConfig.compRotary, CONFIG_SIZE);
    nanosleep(&sleepTime, NULL);
  }
  // ===============================
  
  // Tell thread to stop
  printf("stopping thread\n");
  pthread_mutex_lock(&stop);
  run = false;
  pthread_mutex_unlock(&stop);
  
  // Wait for thread to end
  printf("wait for thread to end\n");
  pthread_join(threadID, NULL);
  
  // Cleanup
  pthread_mutex_destroy(&stop);
  pthread_mutex_destroy(&pruWrite);
  printf("Circular Buffer program end\n");
}

void main (void) {
  // Global init
  sampleBuffer = malloc(sizeof(int) * BUFFER_SIZE);
  if (!sampleBuffer) {
    printf("mem alloc failed\n");
  }
  
  PRU0RamAddrOff = readFileVal(PRU0MAP_LOC "addr");

  buffer();

  free(sampleBuffer);
}
