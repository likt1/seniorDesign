#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#define PRU_NUM 0 // Using PRU0
#define SAMPLE_RATE 44100 // Set sample rate
#define BUFFER_SIZE SAMPLE_RATE // DEBUG current buffer length is 1 sec
#define CONFIG_SIZE 10 // Config strings are 9 chars long + \0

#define PRU0MAP_LOC "/sys/class/uio/uio0/maps/map0/"

typedef int bool; // Define bool
#define true 1
#define false 0

bool run = true;
bool noop = false;
bool save = false;
pthread_mutex_t stop;
pthread_mutex_t pruWrite;
int next = 0;
int start = -1;
int sampleBuffer[BUFFER_SIZE];

unsigned int readFileVal(char filenm[]) {
  FILE* fp;
  unsigned int value = 0;
  fp = fopen(filenm, "rt");
  fscanf(fp, "%x", &value);
  fclose(fp);
  return value;
}

void *pruThread (void *var) {
  // INIT
  // ===============================
  printf("pru Thread active\n");
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
  unsigned int load = 0x0149;
  r = prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, &load, 2);
  if (r < 0) {
    printf("Failed to write memory block\n");
    prussdrv_exit();
    return NULL;
  }

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
        int sample = 0; // Get sample from pru buffer TODO HERE
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
    
    // Continue PRU sampling TODO
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

void main (void) {
  printf("Circular Buffer program start\n");
  
  // INIT
  // ===============================
  bool running = true;
  bool footSwitch = false; // TODOM set defaults?
  char timeRotary[CONFIG_SIZE] = "active";
  
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
  int numEpochs = 5000;
  while (running) {
    if (numEpochs < 0) {
       running = false; // DEBUG
    }
    numEpochs--;
    //save = true; // DEBUG
    //int i; // DEBUG
    //for (i = 0; i < BUFFER_SIZE; i++) {
      //sampleBuffer[i] = 4095;
    //} // DEBUG
    
    // Read config file and set values
    // Init file read vars
    FILE *file = fopen("/root/conf/DIO.config", "r");
    char strBuf[40];
    char* lbl;
    char* val;
    const char delim[2] = ":";
    
    // Init config file val vars
    bool newFootSwitch = false;
    char compRotary[CONFIG_SIZE] = "\0";
    char newTimeRotary[CONFIG_SIZE] = "\0";
    
    if (file) {
      printf("Config file detected...\n");
      while (!(fscanf(file, "%s", strBuf) == EOF)) {
        //printf("Entering loop: %s\n", strBuf);
        lbl = strtok(strBuf, delim); // Start of label
        val = strtok(NULL, delim); // Value

        //printf("lbl: %s val: %s\n", lbl, val);
        if (strcmp(lbl, "CompRotary") == 0) {
          strncpy(compRotary, val, CONFIG_SIZE);
        }
        else if (strcmp(lbl, "TimeRotary") == 0) {
          strncpy(newTimeRotary, val, CONFIG_SIZE);
        }
        else if (strcmp(lbl, "Footswitch") == 0) {
          if (strcmp(val, "True") == 0) {
            newFootSwitch = true;
          }
        }
      }
      fclose(file); /*
      printf("%s\n", compRotary);
      printf("%s\n", newTimeRotary);
      printf("%d\n", newFootSwitch);*/
    }
    
    // Block write thread to check for save switching?
    pthread_mutex_lock(&pruWrite);
    
    // Handle toggle of footswitch
    if (newFootSwitch != footSwitch) {
      if (start = -1 && strcmp(newTimeRotary, "active") == 0) { // If we are active and we are not started 
        start = next; // start active
      }
      else { // save buffer
        save = true;
      }
    }
    
    // Handle switching from active to retroactive if we are running
    if (start > 0 && strcmp(newTimeRotary, timeRotary) != 0) {
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
          if (strcmp(newTimeRotary, "30s") == 0) {
            start = next - 30*SAMPLE_RATE;
          }
          else if (strcmp(newTimeRotary, "1m") == 0) {
            start = next - 60*SAMPLE_RATE;
          }
          else if (strcmp(newTimeRotary, "1m30s") == 0) {
            start = next - 90*SAMPLE_RATE;
          }
          else if (strcmp(newTimeRotary, "2m") == 0) {
            start = next - 120*SAMPLE_RATE;
          }
          else if (strcmp(newTimeRotary, "2m30s") == 0) {
            start = next - 150*SAMPLE_RATE;
          }
          //else { // full 3 min
          if (start < 0) { // DEBUG MAX MINUTES ONLY
            start = next;
          }
          // TODO handle negative number
        }
      
        // Write until we meet next (end)
        do {
          // write to file TODOM improve one by one?
          fwrite(&sampleBuffer[start], 2, 1, file); // one by one, find out a way to save all
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
    
    footSwitch = newFootSwitch;
    strncpy(timeRotary, newTimeRotary, CONFIG_SIZE);
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
