#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdint.h>

// ABOUT VALUES (Used limits.h to check):
//   Our int can handle +-2147483647, so is 32bit
typedef unsigned int word;  // 32bit
typedef uint16_t halfword;  // 16bit
typedef uint8_t byte;       // 8bit

#define CONFIG_SIZE 10 // config strings are 9 chars long + \0
#define HW_SIZE sizeof(halfword) // halfword size
#define PRU_NUM 0 // using PRU0
#define SAMPLE_RATE 44100 // set sample rate
#define RECORD_LENGTH 180 // max record length is 180 sec - 3 min
#define BUFFER_SIZE (SAMPLE_RATE*RECORD_LENGTH) // set buffer size
#define PRU_SAMPLES_NUM 5750  // 6000 is 12kb so just a bit lower than that
#define PRU_DELAY 1980 // amount of delay in PRU cycles

#define PRU0MAP_LOC "/sys/class/uio/uio0/maps/map0/" // file containing pru memory location offset
#define SHARED_MEM_OFFSET 0x10000 // location offset of shared buffer in hex

// Define bool
typedef char bool;
#define true 1
#define false 0
#define init 2

#define MAP_SIZE 16384UL // 16k mapsize
#define MAP_MASK (MAP_SIZE - 1) // 16k mask (NOT USED)

// Configuration object
struct configs {
  bool footSwitch;
  bool memoryLow;
  bool isRecording;
  char compRotary[CONFIG_SIZE];
  char timeRotary[CONFIG_SIZE];
};

/* Bit data structure, make sure this is matched in pru:
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

  word cap_delay;  // delay in cycles to control capture frequency

  word timer; // counts number of ADC reads (NOT USED)
  word stopF; // if set, pru halts
  word buf1F; // if set, buffer is filled and must be written before pru can write
  word buf2F; // same as buf1F
};

#endif
