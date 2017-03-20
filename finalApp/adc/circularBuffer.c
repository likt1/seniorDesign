#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "circularBuffer.h"

// Volatile Exit Flag
static volatile bool exitProg = 0;

// Init globals
bool run = true;
bool noop = false;
bool save = false;
pthread_mutex_t pruWrite;
int next = 0;
int start = -1;
int fd;
void *map_base;
halfword *sampleBuffer;
halfword *pruSamples;
word PRU0RamAddrOff;
struct locals PRU_local;

// Exit interrupt function
void exitInterrupt(int signum) {
  exitProg = 1;
}

// Opens up file and parses value in hex
word readFileVal(char filenm[]) {
  FILE* fp;
  word value = 0;
  fp = fopen(filenm, "rt");
  fscanf(fp, "%x", &value);
  fclose(fp);
  return value;
}

// Get timestamp
int GetUTimeStamp() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec;
}

void *pruThread(void *var) {
  // INIT
  // ===============================
  printf("pru Thread active\n");
  volatile int r;

  // Set up ram memory mapping 
  fd = open("/dev/mem", O_RDONLY);
  if (fd == -1) {
    printf("Failed to open ram to fetch adc values.\n");
    return NULL;
  }
    
  off_t mapLoc;
  mapLoc = PRU0RamAddrOff;
  //printf("mapLoc:0x%X ", mapLoc);
      
  map_base = mmap(0, MAP_SIZE, PROT_READ, MAP_SHARED, fd, mapLoc);
  if (map_base == (void *) -1) {
    printf("Failed to map memory when accessing ram 0x%X.\n", mapLoc);
    return NULL;
  }
  //printf("map_base:0x%X\n", map_base);
      
  // Allocate and init pru mem
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
  PRU_local.samples.stopFlag = 0;
  PRU_local.samples.length = PRU_SAMPLES_NUM;
  PRU_local.cap_delay = PRU_DELAY;
  PRU_local.timer = 0;
  PRU_local.flags = 0;
  r = prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, (word *)&PRU_local, sizeof(PRU_local));
  if (r < 0) {
    printf("Failed to write memory block\n");
    prussdrv_exit();
    return NULL;
  }
 
  //printf("size:%d obj:%d\n", PRU_local.samples.addr, sizeof(PRU_local));
  // Load and execute the PRU program on PRU
  r = prussdrv_exec_program(PRU_NUM, "adcSample.bin");
  if (r < 0) {
    printf("Failed to execute PRU program on PRU%d\n", PRU_NUM);
    prussdrv_exit();
    return NULL;
  }
  // ===============================

  // Init interp vars
  int startTime = 0;
  int diff = 0;
  int interpSampNum = 0;
  
  // MAIN PRU LOOP
  // ===============================
  while (true) {
    // Wait for even compl from PRU, returns PRU_EVTOUT_0 num
    r = prussdrv_pru_wait_event(PRU_EVTOUT_0);
    // printf("PRU returned, event number %d.\n", r);
    
    if (!noop) {
      // Copy ram to local buffer
      void * buffOff = map_base + PRU_local.samples.addr;
      const void *virt_addr = buffOff;
      memcpy(pruSamples, virt_addr, HW_SIZE*PRU_SAMPLES_NUM);
      // printf("Copied:0x%X->0x%X amt:%d\n", virt_addr, pruSamples, HW_SIZE*PRU_SAMPLES_NUM);
    }
    
    // Continue PRU sampling
    prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
    PRU_local.flags = 1;
    r = prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, (word *)&PRU_local, sizeof(PRU_local));
    if (r < 0) {
      printf("Failed to continue PRU!\n");
    }
    
    if (!noop) {
      // Start timer for interp
      startTime = GetUTimeStamp();
      
      // Write to buffer
      pthread_mutex_lock(&pruWrite);
      
      // Interp previous missing time
      //   When interpolating, assume we draw a line from x = 0 to interpSampNum
      //   Calculate y then save to our circular buffer
      int i;
      for (i = 0; i < interpSampNum; i++) { // Bresenham's Line Alg
        sampleBuffer[next] = ((pruSamples[0] - sampleBuffer[next - 1])/interpSampNum)*i + sampleBuffer[next - 1];
        next++;
        if (next >= BUFFER_SIZE) { // Circle back if we reach max size
          next = 0;
        }
      }
      
      // Mass save into circularBuffer
      int bufferLim; // Specify endpoint to save to
      bool noOverflow = false;
      if (start > next) {
        bufferLim = start;
        noOverflow = true; // Active recording will not overflow
      }
      else {
        bufferLim = BUFFER_SIZE;
      }
      
      int freeSpace = bufferLim - next; // Samples that can be stored in buffer
      int overflow = freeSpace - PRU_SAMPLES_NUM; // Samples that will go over, negative if over max
      
      if (overflow >= 0) { // We don't overflow, so save all of pru samples into buffer
        memcpy(&sampleBuffer[next], pruSamples, HW_SIZE*PRU_SAMPLES_NUM);
        next += PRU_SAMPLES_NUM;
      }
      else { // Save to max limit, then reset if we overflow
        memcpy(&sampleBuffer[next], pruSamples, HW_SIZE*freeSpace);
        if (noOverflow) { // Skip overflow and just set max stored in buffer
          next = start;
        }
        else { // Move overflow into beginning of buffer
          int absOverflow = (-1)*overflow;
          memcpy(sampleBuffer, &pruSamples[freeSpace], HW_SIZE*absOverflow);
          next = absOverflow;
        }
      }
      
      if (next == start) { // We have recorded max with active recording
        //printf("start val is:%d\n", start);
        save = true;
        noop = true;
        break; // Break and noop
      }
      
      // Variables need to be updated for these lines to work now
      //if (sample != 0xfff0) { //DEBUG
      //  printf("Debug access:0x%X sample:0x%X virt_addr:0x%X ad_val:0x%X\n", buffOff, sample, virt_addr, *((word*) virt_addr));
      //}

      //if (i == PRU_local.samples.length - 1) {
      //  printf("i:%d addr:0x%X virt_addr:0x%X\n", i, buffOff, virt_addr);
      //}
      
      pthread_mutex_unlock(&pruWrite);
      
      // Stop timer for interp
      diff = GetUTimeStamp() - startTime;
      
      // Calc num for interp
      diff += diff < 0 ? 1000000 : 0;
      interpSampNum = diff / 22; // Our usec per sample
    }
  }
  // ===============================

  printf("pru Thread stopped\n");
  return NULL;
}

void cleanupThread() {
  // Tell PRU to stop
  printf("Cleaning PRU vars\n");
  PRU_local.samples.stopFlag = 1;
  PRU_local.flags = 1;
  prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, (word *)&PRU_local, sizeof(PRU_local));
  
  // Disable PRU and close memory mappings
  prussdrv_pru_disable(PRU_NUM);
  prussdrv_exit();
  
  // Cleanup
  if (map_base != (void *) -1) {
    munmap(map_base, MAP_SIZE);
  }
  if (fd) {
    close(fd);
  }
}

void buffer(void) {
  printf("Circular Buffer program start\n");

  // INIT
  // ===============================
  bool running = true;
  struct configs curConfig; // TODOM set defaults?
  curConfig.footSwitch = init;
  strncpy(curConfig.timeRotary, "\0", CONFIG_SIZE);
  strncpy(curConfig.compRotary, "\0", CONFIG_SIZE);
  
  struct timespec sleepTime = {0, 10000000}; // sleep for 10 ms
  
  // Init mutex
  if (pthread_mutex_init(&pruWrite, NULL) != 0) {
    printf("Failed to init mutex pruWrite\n");
    return;
  }
  
  // Init PRU
  pthread_t threadID;
  if (pthread_create(&threadID, NULL, pruThread, NULL) != 0) {
    printf("Failed to init pru thread obj\n");
    pthread_mutex_destroy(&pruWrite);
    return;
  }
  if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0) {
    printf("Failed to configure pru cancel type\n");
    pthread_mutex_destroy(&pruWrite);
    return;
  }
  // ===============================
  
  // MAIN CONFIG FILE LOOP 
  // ===============================
  bool testExit = true;
  while (testExit) {
    if (exitProg) {
      testExit = false;
      save = true;
    }

    // Read config file and set values
    // Init file read vars
    FILE *file = fopen("/root/conf/DIO.config", "r");
    int numLines = 0;
    char strBuf[40];
    char *lbl, *val;
    const char delim[2] = "=";
    
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
        else if (strcmp(lbl, "MemoryLow") == 0) {
          if (strcmp(val, "True") == 0) {
            newConfig.memoryLow = true;
          }
        }
        else if (strcmp(lbl, "IsRecording") == 0) {
          if (strcmp(val, "True") == 0) {
            newConfig.isRecording = true;
          }
        }
        numLines++;
      }
      fclose(file); /*
      printf("%s\n", newConfig.compRotary);
      printf("%s\n", newConfig.timeRotary);
      printf("%d\n", newConfig.footSwitch);*/
    }
    
    // Check to see we got stuff
    if (numLines < 6) {
      printf("Not enough settings were found!\n");
      continue; // re iterate the infinite loop (eventually should have good reading)
    }
    if (strlen(newConfig.timeRotary) == 0) {
      printf("Empty new time rotary string!\n");
    }
    if (strlen(newConfig.compRotary) == 0) {
      printf("Empty new comppression rotary string!\n");
    }
    
    // Block write thread to check for save switching?
    pthread_mutex_lock(&pruWrite);
    
    // Handle toggle of footswitch
    if (curConfig.footSwitch != init && newConfig.footSwitch != curConfig.footSwitch) {
      if (start == -1 && strcmp(newConfig.timeRotary, "active") == 0) { // If we are active and we are not started 
        start = next; // start active
      }
      else { // save buffer
        printf("footswitch thrown\n");
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
        if (start < 0) { // if passive get prev
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
          else { // full buffer (3 min)
            start = next;
          }
          
          printf("new time rotary:%s\n", newConfig.timeRotary);

          // If value is negative circle back
          // If the value is still negative after the first loop,
          //   output signal is undefined.
          while (start < 0) {
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

        // TODO: should check if file write was valid, then kick off dropbox sync / sd card save 
        system("sh triggerSave.sh");
        
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
  
  // Grabbing the write mutex guarantees that it's not being used by thread
  printf("stopping thread: wait for thread to end\n");
  pthread_mutex_lock(&pruWrite);
  pthread_cancel(threadID);
  pthread_join(threadID, NULL);
  pthread_mutex_unlock(&pruWrite);

  // Cleanup
  cleanupThread();
  pthread_mutex_destroy(&pruWrite);
  printf("Circular Buffer program end\n");
}

void main (void) {
  setbuf(stdout, NULL);

  // Setup graceful (ctrl-c) out
  signal(SIGINT, exitInterrupt);
  signal(SIGTERM, exitInterrupt);
  signal(SIGQUIT, exitInterrupt);


  // Global init
  sampleBuffer = malloc(HW_SIZE*BUFFER_SIZE);
  if (!sampleBuffer) {
    printf("sampleBuffer alloc failed\n");
    return;
  }
  pruSamples = malloc(HW_SIZE*PRU_SAMPLES_NUM);
  if (!pruSamples) {
    printf("pruSamples alloc failed\n");
    free(sampleBuffer);
    return;
  }
  
  PRU0RamAddrOff = readFileVal(PRU0MAP_LOC "addr");

  buffer();

  free(sampleBuffer);
  free(pruSamples);
}
