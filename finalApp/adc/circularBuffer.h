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
#define PRU_SAMPLES_NUM 7950  // 8000 is 16kb so just a bit lower than that
#define PRU_DELAY 1980 // Amount of delay in PRU cycles

#define PRU0MAP_LOC "/sys/class/uio/uio0/maps/map0/"

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
  04   00 00 00 00 | stopFlag
  08   00 00 00 00 | length
  0C   00 00 00 00 | cap_delay
  10   00 00 00 00 | timer
  14   00 00 00 00 | flags
*/
struct locals {
  struct {
    word addr;  // address of DDR memory bank
    word stopFlag;  // stop flag
    word length;  // byte size of available DDR mem bank (non-zero triggers `scope capture)
  } samples;

  word cap_delay;  // extra delay to control capture frequency

  word timer; // counts number of ADC reads
  word flags; // runtime flags
};

#endif
