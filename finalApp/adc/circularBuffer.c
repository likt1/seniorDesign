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
void *map_base1;
void *map_base2;
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

// Return usec of current time, will not work for
//   durations longer than 1 sec
int GetUTimeStamp() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec;
}

// Thread method that inits and runs the pru
void *pruThread(void *var) {
  // INIT
  // ===============================
  //printf("pru Thread active\n");
  volatile int r; // return var to check returns

  // Set up ram memory mapping 
  fd = open("/dev/mem", O_RDONLY);
  if (fd == -1) {
    printf("Failed to open ram to fetch adc values.\n");
    return NULL; // kill if we fail to open ram
  }
    
  off_t mapLoc; // pru location in ram
  mapLoc = PRU0RamAddrOff;
  //printf("mapLoc:0x%X ", mapLoc);
  
  // Load map of pru base location
  map_base1 = mmap(0, MAP_SIZE, PROT_READ, MAP_SHARED, fd, mapLoc);
  if (map_base1 == (void *) -1) {
    printf("Failed to map memory 1 when accessing ram 0x%X.\n", mapLoc);
    return NULL; // kill if we fail to map memory location
  }
  //printf("map_base1:0x%X\n", map_base1);
  
  // Load map of secondary buffer pru
  map_base2 = mmap(0, MAP_SIZE, PROT_READ, MAP_SHARED, fd, mapLoc + SHARED_MEM_OFFSET);
  if (map_base2 == (void *) -1) {
    printf("Failed to map memory 2 when accessing ram 0x%X.\n", mapLoc);
    return NULL; // kill if we fail to map memory location
  }
  //printf("map_base1:0x%X\n", map_base2);
  
  // Allocate and init pru mem
  r = prussdrv_init();
  if (r != 0) {
    printf("Failed to init prussdrv driver\n");
    return NULL; // kill if we fail to init prussdrv connections
  }
  r = prussdrv_open(PRU_EVTOUT_0);
  if (r != 0) {
    printf("Failed to open PRU %d\nerror:%d\n", PRU_EVTOUT_0, r);
    prussdrv_exit();
    return NULL; // kill if we fail to turn on pru
  }

  // Map PRU's interrupts
  tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
  r = prussdrv_pruintc_init(&pruss_intc_initdata);
  if (r != 0) {
    printf("Failed to init interrupts\n");
    prussdrv_exit();
    return NULL; // kill if we fail to init pru interrupts
  }

  // Load memory, definitions located in header file
  PRU_local.samples.addr = sizeof(PRU_local);
  PRU_local.samples.length = PRU_SAMPLES_NUM;
  PRU_local.cap_delay = PRU_DELAY;
  PRU_local.timer = 0;
  PRU_local.stopF = 0;
  PRU_local.buf1F = 0;
  PRU_local.buf2F = 0;
  r = prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, (word *)&PRU_local, sizeof(PRU_local));
  if (r < 0) {
    printf("Failed to write memory block\n");
    prussdrv_exit();
    return NULL; // kill if we fail to write local values to pru memory
  }
 
  //printf("size:%d obj:%d\n", PRU_local.samples.addr, sizeof(PRU_local));
  // Load and execute the PRU program on PRU
  r = prussdrv_exec_program(PRU_NUM, "adcSample.bin");
  if (r < 0) {
    printf("Failed to execute PRU program on PRU%d\n", PRU_NUM);
    prussdrv_exit();
    return NULL; // kill if we fail to execute program
  }
  // ===============================

  // MAIN PRU LOOP
  bool buf1 = true; // true when we are writing to buf1
  // ===============================
  while (true) {
    // Wait for even compl from PRU, returns PRU_EVTOUT_0 num
    r = prussdrv_pru_wait_event(PRU_EVTOUT_0);
    //printf("PRU returned, event number %d.\n", r);
    
    // Copy ram to local buffer
    void * buffOff;
    if (buf1) { // choose buffer
      buffOff = map_base1 + PRU_local.samples.addr; // samples located in offset
      PRU_local.buf1F = 0; // reset buf
      buf1 = false; // switch buf
    }
    else {
      buffOff = map_base2;
      PRU_local.buf2F = 0; // reset buf
      buf1 = true;
    }
    const void *virt_addr = buffOff;
    memcpy(pruSamples, virt_addr, HW_SIZE*PRU_SAMPLES_NUM); // copy values into pru buf
    // printf("Copied:0x%X->0x%X amt:%d\n", virt_addr, pruSamples, HW_SIZE*PRU_SAMPLES_NUM);
    
    // Continue PRU sampling
    prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT); // allegedly clears the pru to arm interrupt, during testing this doesn't seem to do anything
    r = prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, (word *)&PRU_local, sizeof(PRU_local));
    if (r < 0) {
      printf("Failed to continue PRU!\n"); // if this happens pru is still running but we can't stop it, so the program is effectively dead
    }
    
    if (!noop) { // write to circular buffer only when we need to
      // Write to buffer
      pthread_mutex_lock(&pruWrite);
      
      // Batch save into circularBuffer
      int bufferLim; // specify endpoint to save to
      bool noOverflow = false; // active recording overflow bool
      if (start > next) {
        bufferLim = start; // our endpoint is the active recording start point
        noOverflow = true; // active recording will not overflow
      }
      else {
        bufferLim = BUFFER_SIZE; // otherwise it's the end of the buffer
      }
      
      int freeSpace = bufferLim - next; // samples that can be stored in buffer
      int overflow = freeSpace - PRU_SAMPLES_NUM; // samples that will go over, negative if over max
      
      if (overflow >= 0) { // we don't overflow, so save all of pru samples into buffer
        memcpy(&sampleBuffer[next], pruSamples, HW_SIZE*PRU_SAMPLES_NUM);
        next += PRU_SAMPLES_NUM; // inc next to new location
      }
      else { // save to max limit, then reset if we overflow
        memcpy(&sampleBuffer[next], pruSamples, HW_SIZE*freeSpace);
        if (noOverflow) { // skip overflow on active and just set max stored in buffer
          next = start; // set equal to fire active recording save
        }
        else { // save overflow into beginning of buffer
          int absOverflow = (-1)*overflow; // overflow has to be neg, so this abs values it
          memcpy(sampleBuffer, &pruSamples[freeSpace], HW_SIZE*absOverflow);
          next = absOverflow; // inc next to overflow end
        }
      }
      
      if (next == start) { // we have recorded max with active recording
        //printf("start val is:%d\n", start);
        save = true;
        noop = true;
        break; // Break and noop
      }
      
      pthread_mutex_unlock(&pruWrite);
    }
  }
  // ===============================

  printf("pru Thread stopped\n");
  return NULL;
}

// Function that cleans up the pru thread after we async kill it
void cleanupThread() {
  // Tell PRU to stop
  printf("Cleaning PRU vars\n");
  PRU_local.stopF = 1; // fire stop bit
  PRU_local.buf1F = 0; // open up both buffers
  PRU_local.buf2F = 0;
  volatile int r = prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, (word *)&PRU_local, sizeof(PRU_local));
  if (r < 0) {
    printf("Failed to stop PRU!\n"); // this means that the pru will not stop and is probably dead
  }
  
  // Disable PRU and close memory mappings
  prussdrv_pru_disable(PRU_NUM);
  prussdrv_exit();
  
  // Cleanup map bases
  if (map_base1 != (void *) -1) {
    munmap(map_base1, MAP_SIZE);
  }
  if (map_base2 != (void *) -1) {
    munmap(map_base2, MAP_SIZE);
  }
  if (fd) { // close file
    close(fd);
  }
}

// Config file reader thread
void buffer(void) {
  printf("Circular Buffer program start\n");

  // INIT
  // ===============================
  struct configs curConfig;
  curConfig.footSwitch = init; // init footswitch bool to init state
  curConfig.memoryLow = false;
  curConfig.isRecording = false;
  strncpy(curConfig.timeRotary, "\0", CONFIG_SIZE); // init to empty string
  strncpy(curConfig.compRotary, "\0", CONFIG_SIZE);
  
  struct timespec sleepTime = {0, 30000000}; // sleep for 30 ms
  
  // Init write mutex
  if (pthread_mutex_init(&pruWrite, NULL) != 0) {
    printf("Failed to init mutex pruWrite\n");
    return; // kill if mutex failed
  }
  
  // Init PRU
  pthread_t threadID;
  if (pthread_create(&threadID, NULL, pruThread, NULL) != 0) {
    printf("Failed to init pru thread obj\n");
    pthread_mutex_destroy(&pruWrite);
    return; // kill if pruthread failed
  }
  if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0) {
    printf("Failed to configure pru cancel type\n");
    pthread_mutex_destroy(&pruWrite);
    return; // kill if async failed
  }
  // ===============================
  
  // MAIN CONFIG FILE LOOP 
  // ===============================
  bool loopExit = true; // allows us to break out of loop
  while (loopExit) {
    if (exitProg) { // is set when program is terminated or killed
      loopExit = false;
      save = true; // save when we exit out of program
    }

    // Read config file and set values
    // Init file read vars
    FILE *file = fopen("/root/conf/DIO.config", "r");
    int numLines = 0;
    char strBuf[40]; // 40 chars to read each line
    char *lbl, *val;
    const char delim[2] = "=";
    
    // Init config file val vars
    struct configs newConfig; // make a newconfig struct to save the new configs
    newConfig.footSwitch = false; // init to false
    newConfig.memoryLow = false;
    newConfig.isRecording = false;
    strncpy(newConfig.timeRotary, "\0", CONFIG_SIZE);
    strncpy(newConfig.compRotary, "\0", CONFIG_SIZE);
    
    if (file) { // only move forward if file opened
      //printf("Config file detected...\n");
      while (!(fscanf(file, "%s", strBuf) == EOF)) { // keep reading lines until end of file
        lbl = strtok(strBuf, delim); // Start of label
        val = strtok(NULL, delim); // Value

        //printf("lbl: %s val: %s\n", lbl, val);
        if (strcmp(lbl, "CompRotary") == 0) { // read in conf values
          strncpy(newConfig.compRotary, val, CONFIG_SIZE);
        }
        else if (strcmp(lbl, "TimeRotary") == 0) {
          strncpy(newConfig.timeRotary, val, CONFIG_SIZE);
        }
        else if (strcmp(lbl, "Footswitch") == 0) { // update footswitch
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
      fclose(file);
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
      printf("Empty new compression rotary string!\n");
    }
    
    // Block write thread to check for save switching?
    pthread_mutex_lock(&pruWrite);
    
    // Handle toggle of footswitch
    //   If our current footswitch is valid and the new footswitch value does not equal our value
    if (curConfig.footSwitch != init && newConfig.footSwitch != curConfig.footSwitch) {
      if (start == -1 && strcmp(newConfig.timeRotary, "active") == 0) { // If we are active and we are not started 
        start = next; // start active
      }
      else { // save buffer for retro
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
        if (start < 0) { // if retroactive get prev sec amount
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
          start++;
          if (start == BUFFER_SIZE) {
            start = 0;
          }
        } while (start != next);
      
        // Reset
        fclose(file);

        // TODO: should check if file write was valid, then kick off dropbox sync / sd card save 
        system("sh triggerSave.sh");
        //system("python3 pushToDropbox.py testOut-44k1.wav");
        
        noop = false;
        save = false;
        start = -1;
      }
      else {
        printf("Could not open file for write\n"); // in this case we don't write out
      }
    }
    pthread_mutex_unlock(&pruWrite);
    
    curConfig.footSwitch = newConfig.footSwitch;
    curConfig.memoryLow = newConfig.memoryLow;
    curConfig.isRecording = newConfig.isRecording;
    strncpy(curConfig.timeRotary, newConfig.timeRotary, CONFIG_SIZE);
    strncpy(curConfig.compRotary, newConfig.compRotary, CONFIG_SIZE);
    nanosleep(&sleepTime, NULL);
  }
  // ===============================
  
  // Stopping
  //   Grabbing the write mutex guarantees that it's not being used by thread
  printf("stopping thread: wait for thread to end\n");
  pthread_mutex_lock(&pruWrite);
  pthread_cancel(threadID); // kill thread
  pthread_join(threadID, NULL); // make sure thread is dead
  pthread_mutex_unlock(&pruWrite);

  // Cleanup
  cleanupThread();
  pthread_mutex_destroy(&pruWrite);
  printf("Circular Buffer program end\n");
}

void main (void) {
  setbuf(stdout, NULL); // stdout shouldn't use buffer

  // Setup graceful (ctrl-c) out
  signal(SIGINT, exitInterrupt);
  signal(SIGTERM, exitInterrupt);
  signal(SIGQUIT, exitInterrupt);

  // Global init
  sampleBuffer = malloc(HW_SIZE*BUFFER_SIZE);
  if (!sampleBuffer) {
    printf("sampleBuffer alloc failed\n");
    return; // kill if sampleBuffer fails to load
  }
  pruSamples = malloc(HW_SIZE*PRU_SAMPLES_NUM);
  if (!pruSamples) {
    printf("pruSamples alloc failed\n");
    free(sampleBuffer);
    return; // kill if pruSamples fails to load
  }
  
  PRU0RamAddrOff = readFileVal(PRU0MAP_LOC "addr"); // grab the pru loc

  buffer(); // run buffer

  free(sampleBuffer); // free memory
  free(pruSamples);
}
