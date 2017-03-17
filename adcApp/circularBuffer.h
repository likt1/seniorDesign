#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdint.h>

// ABOUT VALUES (Used limits.h to check):
// Our int can handle +-2147483647, so is 32bit
typedef unsigned int word;  // 32bit
typedef uint16_t halfword;  // 16bit
typedef uint8_t byte;       // 8bit

#define PRU_NUM 0 // Using PRU0
#define SAMPLE_RATE 44100 // Set sample rate
#define BUFFER_LENGTH 3 // DEBUG length is 30 sec
#define BUFFER_SIZE (SAMPLE_RATE*BUFFER_LENGTH) // Set buffer size
#define CONFIG_SIZE 10 // Config strings are 9 chars long + \0
#define PRU_SAMPLES_NUM 795

#define PRU0MAP_LOC "/sys/class/uio/uio0/maps/map0/"

typedef char bool; // Define bool
#define true 1
#define false 0

#define MAP_SIZE 16384UL
#define MAP_MASK (MAP_SIZE - 1)

struct configs {
  bool footSwitch;
  char compRotary[CONFIG_SIZE];
  char timeRotary[CONFIG_SIZE];
};

/* bit data structure:
0x00 0x00 00 00 00 | 03 addr
  04   00 00 00 00 | 07 offset
  08   00 00 00 00 | 0B length
  0C   00 00 00 00 | 0F cap_delay
  10   00 00 00 00 | 13 timer
  14   00 00 00 00 | 17 flags flag check would be FEA51B1E
*/
struct locals {
  struct {
    word addr;  // address of DDR memory bank
    word offset;  // byte offset into local memory to capture for `scope mode
    word length;  // byte size of available DDR mem bank (non-zero triggers `scope capture)
  } samples;

  word cap_delay;  // extra delay to control capture frequency

  word timer; // counts number of ADC reads
  word flags; // runtime flags
};

#endif
