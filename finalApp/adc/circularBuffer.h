#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdint.h>

// ABOUT VALUES (Used limits.h to check):
// Our int can handle +-2147483647, so is 32bit
typedef unsigned int word;  // 32bit
typedef uint16_t halfword;  // 16bit
typedef uint8_t byte;       // 8bit

#define CONFIG_SIZE 10 // Config strings are 9 chars long + \0
#define HW_SIZE sizeof(halfword)
#define PRU_NUM 0 // Using PRU0
#define SAMPLE_RATE 44100 // Set sample rate
#define RECORD_LENGTH 60 // DEBUG length is 30 sec
#define BUFFER_SIZE (SAMPLE_RATE*RECORD_LENGTH) // Set buffer size
#define PRU_SAMPLES_NUM 5750  // 6000 is 12kb so just a bit lower than that
#define PRU_DELAY 1980 // Amount of delay in PRU cycles

#define PRU0MAP_LOC "/sys/class/uio/uio0/maps/map0/"
#define SHARED_MEM_OFFSET 0x10000

typedef char bool; // Define bool
#define true 1
#define false 0
#define init 2

#define MAP_SIZE 16384UL
#define MAP_MASK (MAP_SIZE - 1)

struct configs {
  bool footSwitch;
  char compRotary[CONFIG_SIZE];
  char timeRotary[CONFIG_SIZE];
  bool memoryLow;
  bool isRecording;
};

/* bit data structure:
0x00 0x00 00 00 00 | addr
  04   00 00 00 00 | length
  08   00 00 00 00 | cap_delay
  0C   00 00 00 00 | timer
  10   00 00 00 00 | stopF
  14   00 00 00 00 | buf1F
  18   00 00 00 00 | buf2F
*/
struct locals {
  struct {
    word addr;  // address of first memory bank
    word length;  // number of samples to record
  } samples;

  word cap_delay;  // extra delay to control capture frequency

  word timer; // counts number of ADC reads
  word stopF; // if set, pru halts
  word buf1F; // if set, arm is reading this buffer and pru cannot write
  word buf2F; // if set, arm is reading this buffer and pru cannot write
};

#endif
