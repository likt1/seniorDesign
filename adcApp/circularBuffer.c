#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#define PRU_NUM 0				// using PRU0 for examples
#define SAMPLE_RATE 44100

typedef int bool;
#define true 1
#define false 0

bool run = true, noop = false, save = false;
pthread_mutex_t stop;
pthread_mutex_t pruWrite;
int next = 0, start = -1, bufLength = SAMPLE_RATE, max = bufLength;
int sampleBuffer[bufLength];

void *pruThread (void *var) {
  // INIT
  // ===============================
  printf("pru Thread active");
	int r;
  
  // Allocate and init mem
	r = prussdrv_init();
	if (r != 0) {
		printf("Failed to init prussdrv driver\n");
		return NULL;
	}
	r = prussdrv_open(PRU_EVTOUT_0);
	if (r != 0) {
		printf("Failed to open PRU %d\nerror:%d\n", PRU_NUM, r);
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

	// Load and execute the PRU program on PRU
	prussdrv_exec_program(PRU_NUM, "ledButton.bin");
  // ===============================

  // MAIN PRU LOOP
  // ===============================
  while (true) {
    // Wait for even compl from PRU, returns PRU_EVTOUT_0 num
    r = prussdrv_pru_wait_event(PRU_EVTOUT_0);
    printf("PRU returned, event number %d.\n", r);
    
    // Write to buffer
    pthread_mutex_lock(&pruWrite);
    if (!noop) {
      for (int i = 0; i < 0; i++) { // For each sample in pru buffer
        int sample = 0; // Get sample from pru buffer TODO HERE
        sampleBuffer[next] = sample;
        next++;
        if (next == max) {
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
    
    // Check to see if we should stop sampling
    pthread_mutex_lock(&stop);
    if (!run) {
      break;
    }
    pthread_mutex_unlock(&stop);
    
    // Continue PRU sampling TODO
    // prussdrv_pru_clear_event?
  }
  // ===============================
  
	// Disable PRU and close memory mappings
	prussdrv_pru_disable(PRU_NUM);
	prussdrv_exit();
  
  printf("pru Thread stopped");
  return NULL;
}

void main (void) {
	printf("Circular Buffer program start\n");
  
  // INIT
  // ===============================
  bool running = true;
  bool footSwitch = false; // TODO set defaults
  char timeRotary[6] = "active";
  
  // Init mutex
  if (pthread_mutex_init(&stop, NULL) != 0) {
    printf("Failed to init mutex stop");
    return;
  }
  if (pthread_mutex_init(&pruWrite, NULL) != 0) {
    printf("Failed to init mutex stop");
    return;
  }
  
  // Init PRU
  pthread_t threadID;
  if (pthread_create(&threadID, NULL, pruThread, NULL) != 0) {
    printf("Failed to init thread");
  }
  // ===============================
  
  // MAIN CONFIG FILE LOOP 
  // ===============================
  while (running) {
    running = false; // DEBUG
    // Read config file and set values TODO
    
    FILE *file;
    
    file = fopen("TODO", "r");
    char strBuf[30];
    
    bool newFootSwitch = false;
    char compRotary[5];
    char newTimeRotary[6];
    fclose(file);
    
    // Block write thread to check for save switching?
    pthread_mutex_lock(&pruWrite);
    
    // Handle toggle of footswitch
    if (newFootSwitch != footSwitch) {
      //pthread_mutex_lock(&pruWrite);
      if (start = -1 && strcmp(timeRotary, "active")) { // If we are active and we are not started 
        start = next; // start active
      }
      else { // save buffer
        save = true;
      }
      //pthread_mutex_unlock(&pruWrite);
    }
    
    // Handle switching from active to retroactive if we are running
    // Currently we save the active recording
    if (start > 0 && strcmp(newTimeRotary, timeRotary) != 0) {
      //pthread_mutex_lock(&pruWrite);
      save = true;
      //pthread_mutex_unlock(&pruWrite);
    }
    
    // If we are saving
    //pthread_mutex_lock(&pruWrite);
    if (save) {
      // Open file
      file = fopen("~/testOut.raw", "w");
      
      // Set write head start
      if (start == -1) {
        if (strcmp(newTimeRotary, "30sec") == 0) {
          start = next - 30*SAMPLE_RATE;
        }
        else if (strcmp(newTimeRotary, "1min") == 0) {
          start = next - 60*SAMPLE_RATE;
        }
        else if (strcmp(newTimeRotary, "1min30sec") == 0) {
          start = next - 90*SAMPLE_RATE;
        }
        else if (strcmp(newTimeRotary, "2min") == 0) {
          start = next - 120*SAMPLE_RATE;
        }
        else if (strcmp(newTimeRotary, "2min30") == 0) {
          start = next - 150*SAMPLE_RATE;
        }
        else {
          start = next;
        }
      }
      
      // Write until we meet next (end)
      do {
        // write to file TODO
        // fwrite
        start++;
        if (start == max) {
          start == 0;
        }
      } while (start != next)
      
      // Reset
      fclose(file);
      noop = false;
      save = false;
      start = -1;
    }
    pthread_mutex_unlock(&pruWrite);
    
    footSwitch = newFootSwitch;
  }
  // ===============================
  
  // Tell thread to stop
  pthread_mutex_lock(&stop);
  run = false;
  pthread_mutex_unlock(&stop);
  
  // Wait for thread to end
  pthread_join(threadID, NULL);
  
  // Cleanup
  pthread_mutex_destroy(&stop);
  pthread_mutex_destroy(&pruWrite);
  printf("Circular Buffer program end");
}
